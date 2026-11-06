#include "LockState.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/LocalLevelManager.hpp>
#include <cvolton.level-id-api/include/EditorIDs.hpp>

#include <algorithm>
#include <ctime>

using namespace geode::prelude;

namespace geolock {
    namespace {
        constexpr auto LOCK_KEY = "active-lock";
        constexpr int NO_LEVEL_ID = -1;

        LevelSource levelSource(const GJGameLevel& level) {
            switch (level.m_levelType) {
                case GJLevelType::Editor: return LevelSource::Editor;
                case GJLevelType::Main: return LevelSource::Official;
                default: return LevelSource::Online;
            }
        }

        LockProgress readProgress(const matjson::Value& saved) {
            LockProgress progress;
            progress.startedAt = saved["started-at"].as<std::int64_t>().ok();
            progress.startingBest = saved["starting-best"].as<int>().ok();
            progress.attempts = std::max(0, saved["attempts"].as<int>().unwrapOr(0));
            if (auto bests = saved["new-bests"].asArray(); bests.isOk()) {
                for (const auto& best : bests.unwrap()) {
                    auto percent = best.as<int>().unwrapOr(-1);
                    if (percent >= 0 && percent <= 100 &&
                        (progress.newBests.empty() || percent > progress.newBests.back())) {
                        progress.newBests.push_back(percent);
                    }
                }
            }
            return progress;
        }

        matjson::Value writeProgress(const LockProgress& progress) {
            auto bests = matjson::Value::array();
            for (auto percent : progress.newBests) {
                bests.push(percent);
            }
            auto saved = matjson::makeObject({
                { "attempts", progress.attempts },
                { "new-bests", std::move(bests) },
            });
            if (progress.startedAt) {
                saved["started-at"] = *progress.startedAt;
            }
            if (progress.startingBest) {
                saved["starting-best"] = *progress.startingBest;
            }
            return saved;
        }

        void saveLock(const Lock& lock) {
            auto saved = matjson::makeObject({
                { "id", lock.levelId },
                { "name", lock.levelName },
                { "source", static_cast<int>(lock.source) },
                { "progress", writeProgress(lock.progress) },
            });
            if (lock.practiceCopyId) {
                saved["practice-copy-id"] = *lock.practiceCopyId;
            }
            Mod::get()->setSavedValue(LOCK_KEY, saved);
        }

        std::optional<Lock> loadLock() {
            const auto& savedValues = Mod::get()->getSaveContainerConst();

            // A stored null means the lock was cleared. Only migrate if the new key is absent.
            if (!savedValues.contains(LOCK_KEY)) {
                auto id = savedValues["locked-level-id"].as<int>().unwrapOr(NO_LEVEL_ID);
                if (id == NO_LEVEL_ID) {
                    return std::nullopt;
                }

                Lock lock{
                    id,
                    savedValues["locked-level-name"].asString().unwrapOr(std::string{}),
                    LevelSource::Legacy,
                };
                saveLock(lock);
                return lock;
            }

            const auto& saved = savedValues[LOCK_KEY];
            auto id = saved["id"].as<int>().unwrapOr(NO_LEVEL_ID);
            if (id == NO_LEVEL_ID) {
                return std::nullopt;
            }

            auto source = saved["source"].as<int>().unwrapOr(static_cast<int>(LevelSource::Legacy));
            return Lock{
                id,
                saved["name"].asString().unwrapOr(std::string{}),
                static_cast<LevelSource>(source),
                saved["practice-copy-id"].as<int>().ok(),
                readProgress(saved["progress"]),
            };
        }

        std::optional<Lock>& lockState() {
            static auto lock = loadLock();
            return lock;
        }

        bool matches(const Lock& lock, GJGameLevel* level) {
            // Older saves stored only the ID, so match them the same way v1 did.
            return (lock.source == LevelSource::Legacy || lock.source == levelSource(*level)) &&
                lock.levelId == EditorIDs::getID(level, true);
        }
    }

    const std::optional<Lock>& activeLock() {
        return lockState();
    }

    bool startLock(GJGameLevel* level) {
        auto& active = lockState();
        if (active || !level) {
            return false;
        }

        auto id = EditorIDs::getID(level, true);
        if (id == NO_LEVEL_ID) {
            return false;
        }

        Lock lock{ id, std::string(level->m_levelName), levelSource(*level) };
        lock.progress.startedAt = std::time(nullptr);
        if (!level->isPlatformer()) {
            auto savedLevel = lock.source == LevelSource::Online ? GameLevelManager::get()->getSavedLevel(level) : nullptr;
            auto best = (savedLevel ? savedLevel : level)->m_normalPercent.value();
            lock.progress.startingBest = std::clamp(best, 0, 100);
        }
        saveLock(lock);
        active = std::move(lock);
        return true;
    }

    bool isLockTarget(GJGameLevel* level) {
        const auto& lock = activeLock();
        return lock && level && matches(*lock, level);
    }

    GJGameLevel* findCreatedLevel(int id) {
        auto level = EditorIDs::getLevelByID(id);
        auto levels = LocalLevelManager::get()->getCreatedLevels(0);
        return level && levels && levels->containsObject(level) ? level : nullptr;
    }

    GJGameLevel* practiceCopy() {
        const auto& lock = activeLock();
        return lock && lock->practiceCopyId ? findCreatedLevel(*lock->practiceCopyId) : nullptr;
    }

    bool setPracticeCopy(GJGameLevel* copy) {
        auto& lock = lockState();
        if (!lock) {
            return false;
        }

        auto updatedLock = *lock;
        updatedLock.practiceCopyId.reset();
        if (copy) {
            auto levels = LocalLevelManager::get()->getCreatedLevels(0);
            if (!levels || !levels->containsObject(copy) || copy->m_levelType != GJLevelType::Editor ||
                isLockTarget(copy)) {
                return false;
            }

            auto id = EditorIDs::getID(copy, true);
            if (id <= 0) {
                return false;
            }
            updatedLock.practiceCopyId = id;
        }

        saveLock(updatedLock);
        lock = std::move(updatedLock);
        return true;
    }

    bool isLevelBlocked(GJGameLevel* level) {
        const auto& lock = activeLock();
        if (!lock || !level || matches(*lock, level)) {
            return false;
        }

        return !lock->practiceCopyId || level->m_levelType != GJLevelType::Editor ||
            EditorIDs::getID(level, true) != *lock->practiceCopyId;
    }

    void recordLockAttempt(GJGameLevel* level) {
        auto& lock = lockState();
        if (!lock || !level || isLevelBlocked(level)) {
            return;
        }

        ++lock->progress.attempts;
        saveLock(*lock);
    }

    void recordLockBest(GJGameLevel* level, int percent) {
        if (!isLockTarget(level)) {
            return;
        }

        auto& lock = *lockState();
        auto& bests = lock.progress.newBests;
        if (bests.empty() || percent > bests.back()) {
            bests.push_back(percent);
            saveLock(lock);
        }
    }

    void clearLockOnCompletion(GJGameLevel* level, bool practiceMode) {
        auto& lock = lockState();
        if (lock && level && !practiceMode && matches(*lock, level)) {
            Mod::get()->setSavedValue(LOCK_KEY, matjson::Value());
            lock.reset();
        }
    }
}

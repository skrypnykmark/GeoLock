#include "LockState.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/LocalLevelManager.hpp>
#include <cvolton.level-id-api/include/EditorIDs.hpp>

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

        void saveLock(const Lock& lock) {
            auto saved = matjson::makeObject({
                { "id", lock.levelId },
                { "name", lock.levelName },
                { "source", static_cast<int>(lock.source) },
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
        saveLock(lock);
        active = std::move(lock);
        return true;
    }

    bool isLockTarget(GJGameLevel* level) {
        const auto& lock = activeLock();
        return lock && level && matches(*lock, level);
    }

    GJGameLevel* practiceCopy() {
        const auto& lock = activeLock();
        if (!lock || !lock->practiceCopyId) {
            return nullptr;
        }

        auto copy = EditorIDs::getLevelByID(*lock->practiceCopyId);
        auto levels = LocalLevelManager::get()->getCreatedLevels(0);
        return copy && levels && levels->containsObject(copy) ? copy : nullptr;
    }

    bool setPracticeCopy(GJGameLevel* target, GJGameLevel* copy) {
        if (!isLockTarget(target)) {
            return false;
        }

        auto updatedLock = *activeLock();
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
        lockState() = std::move(updatedLock);
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

    void clearLockOnCompletion(GJGameLevel* level, bool practiceMode) {
        auto& lock = lockState();
        if (lock && level && !practiceMode && matches(*lock, level)) {
            Mod::get()->setSavedValue(LOCK_KEY, matjson::Value());
            lock.reset();
        }
    }
}

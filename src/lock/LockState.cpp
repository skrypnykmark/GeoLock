#include "LockState.hpp"

#include <Geode/Geode.hpp>
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
            Mod::get()->setSavedValue(LOCK_KEY, matjson::makeObject({
                { "id", lock.levelId },
                { "name", lock.levelName },
                { "source", static_cast<int>(lock.source) },
            }));
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

    bool isLevelBlocked(GJGameLevel* level) {
        const auto& lock = activeLock();
        return lock && level && !matches(*lock, level);
    }

    void clearLockOnCompletion(GJGameLevel* level, bool practiceMode) {
        auto& lock = lockState();
        if (lock && level && !practiceMode && matches(*lock, level)) {
            Mod::get()->setSavedValue(LOCK_KEY, matjson::Value());
            lock.reset();
        }
    }
}

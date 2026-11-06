#pragma once

#include <optional>
#include <string>

class GJGameLevel;

namespace geolock {
    enum class LevelSource {
        Legacy = 0,
        Online = 1,
        Editor = 2,
        Official = 3,
    };

    struct Lock {
        int levelId;
        std::string levelName;
        LevelSource source;
    };

    const std::optional<Lock>& activeLock();
    bool startLock(GJGameLevel* level);
    bool isLevelBlocked(GJGameLevel* level);
    void clearLockOnCompletion(GJGameLevel* level, bool practiceMode);
}

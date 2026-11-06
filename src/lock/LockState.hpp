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
        std::optional<int> practiceCopyId = std::nullopt;
    };

    const std::optional<Lock>& activeLock();
    bool startLock(GJGameLevel* level);
    bool isLockTarget(GJGameLevel* level);
    GJGameLevel* practiceCopy();
    bool setPracticeCopy(GJGameLevel* target, GJGameLevel* copy);
    bool isLevelBlocked(GJGameLevel* level);
    void clearLockOnCompletion(GJGameLevel* level, bool practiceMode);
}

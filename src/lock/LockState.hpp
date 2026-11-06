#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

class GJGameLevel;

namespace geolock {
    enum class LevelSource {
        Legacy = 0,
        Online = 1,
        Editor = 2,
        Official = 3,
    };

    struct LockProgress {
        std::optional<std::int64_t> startedAt;
        std::optional<int> startingBest;
        int attempts = 0;
        std::vector<int> newBests;
    };

    struct Lock {
        int levelId;
        std::string levelName;
        LevelSource source;
        std::optional<int> practiceCopyId = std::nullopt;
        LockProgress progress{};
    };

    const std::optional<Lock>& activeLock();
    bool startLock(GJGameLevel* level);
    bool isLockTarget(GJGameLevel* level);
    GJGameLevel* findCreatedLevel(int id);
    GJGameLevel* practiceCopy();
    bool setPracticeCopy(GJGameLevel* copy);
    bool isLevelBlocked(GJGameLevel* level);
    void recordLockAttempt(GJGameLevel* level);
    void recordLockBest(GJGameLevel* level, int percent);
    void clearLockOnCompletion(GJGameLevel* level, bool practiceMode);
}

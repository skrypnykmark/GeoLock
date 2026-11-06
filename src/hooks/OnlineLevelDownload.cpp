#include "OnlineLevelDownload.hpp"

#include <Geode/Geode.hpp>
#include <Geode/modify/GameLevelManager.hpp>

#include <optional>

using namespace geode::prelude;

namespace {
    struct LevelRequest {
        int id;
        std::string key;
        bool startedByGeoLock;
        geode::Function<void(GJGameLevel*)> callback;
    };

    std::optional<LevelRequest> pendingRequest;
}

bool geolock::requestOnlineLevel(int id, geode::Function<void(GJGameLevel*)> callback) {
    if (pendingRequest) {
        return false;
    }

    auto manager = GameLevelManager::get();
    std::string key = manager->getLevelDownloadKey(id, false, 0);
    auto shouldStart = !manager->isDLActive(key.c_str());
    pendingRequest.emplace(LevelRequest{ id, std::move(key), shouldStart, std::move(callback) });
    if (shouldStart) {
        manager->downloadLevel(id, false, 0);
    }
    return true;
}

void geolock::cancelOnlineLevelRequest(int id) {
    if (pendingRequest && pendingRequest->id == id) {
        pendingRequest.reset();
    }
}

class $modify(GeoLockGameLevelManager, GameLevelManager) {
    void processOnDownloadLevelCompleted(gd::string response, gd::string tag, bool update) {
        if (update || !pendingRequest || pendingRequest->key != tag.c_str()) {
            GameLevelManager::processOnDownloadLevelCompleted(std::move(response), std::move(tag), update);
            return;
        }

        auto request = std::move(*pendingRequest);
        pendingRequest.reset();

        // A request started by the Lock Room has no native level page to notify.
        auto delegate = m_levelDownloadDelegate;
        if (request.startedByGeoLock) {
            m_levelDownloadDelegate = nullptr;
        }
        GameLevelManager::processOnDownloadLevelCompleted(std::move(response), std::move(tag), update);
        if (request.startedByGeoLock && !m_levelDownloadDelegate) {
            m_levelDownloadDelegate = delegate;
        }

        Ref<GJGameLevel> level(this->getSavedLevel(request.id));
        queueInMainThread([level = std::move(level), callback = std::move(request.callback)]() mutable {
            callback(level);
        });
    }
};

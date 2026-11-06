#include "../lock/LockState.hpp"
#include "../ui/LockControls.hpp"

#include <Geode/modify/EditorPauseLayer.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/GJWorldNode.hpp>
#include <Geode/modify/LevelAreaInnerLayer.hpp>
#include <Geode/modify/LevelPage.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/SecretLayer2.hpp>

#include <algorithm>
#include <cmath>

using namespace geode::prelude;

class $modify(GeoLockLevelPage, LevelPage) {
    void onPlay(CCObject* sender) {
        if (!geolock::blockLevelLaunch(m_level)) {
            LevelPage::onPlay(sender);
        }
    }
};

class $modify(GeoLockWorldNode, GJWorldNode) {
    void onLevel(CCObject* sender) {
        if (!geolock::blockLevelLaunch(m_level)) {
            GJWorldNode::onLevel(sender);
        }
    }
};

// Tower and secret entrances do not expose a level to match against the lock.
class $modify(GeoLockAreaLayer, LevelAreaInnerLayer) {
    void onDoor(CCObject* sender) {
        if (!geolock::blockUnsupportedLaunch()) {
            LevelAreaInnerLayer::onDoor(sender);
        }
    }
};

class $modify(GeoLockSecretLayer, SecretLayer2) {
    void onSecretLevel(CCObject* sender) {
        if (!geolock::blockUnsupportedLaunch()) {
            SecretLayer2::onSecretLevel(sender);
        }
    }
};

// Block editor playtest at the button callbacks, before the editor starts tearing down.
class $modify(GeoLockEditorUI, EditorUI) {
    void onPlaytest(CCObject* sender) {
        if (!geolock::blockLevelLaunch(m_editorLayer ? m_editorLayer->m_level : nullptr)) {
            EditorUI::onPlaytest(sender);
        }
    }
};

class $modify(GeoLockEditorPauseLayer, EditorPauseLayer) {
    void onSaveAndPlay(CCObject* sender) {
        if (!geolock::blockLevelLaunch(m_editorLayer ? m_editorLayer->m_level : nullptr)) {
            EditorPauseLayer::onSaveAndPlay(sender);
        }
    }
};

class $modify(GeoLockPlayLayer, PlayLayer) {
    struct Fields {
        bool trackingAttempt = false;
    };

    void finishLockAttempt(bool cleared = false) {
        if (!m_fields->trackingAttempt) {
            return;
        }

        m_fields->trackingAttempt = false;
        if (!m_isPracticeMode && !m_isPlatformer) {
            auto progress = cleared ? 100.f : this->getCurrentPercent();
            if (std::isfinite(progress)) {
                geolock::recordLockBest(m_level, static_cast<int>(std::clamp(progress, 0.f, cleared ? 100.f : 99.f)));
            }
        }
    }

    void updateAttempts() {
        PlayLayer::updateAttempts();
        if (!m_useReplay) {
            geolock::recordLockAttempt(m_level);
        }
        m_fields->trackingAttempt = !m_isPracticeMode && !m_useReplay && !m_startPosObject &&
            geolock::isLockTarget(m_level);
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        PlayLayer::destroyPlayer(player, object);
        if (m_playerDied) {
            this->finishLockAttempt();
        }
    }

    void resetLevel() {
        this->finishLockAttempt();
        PlayLayer::resetLevel();
    }

    void togglePracticeMode(bool practiceMode) {
        if (practiceMode) {
            this->finishLockAttempt();
        }
        PlayLayer::togglePracticeMode(practiceMode);
    }

    void onQuit() {
        this->finishLockAttempt();
        PlayLayer::onQuit();
    }

    void levelComplete() {
        this->finishLockAttempt(true);
        PlayLayer::levelComplete();
        geolock::clearLockOnCompletion(m_level, m_isPracticeMode);
    }
};

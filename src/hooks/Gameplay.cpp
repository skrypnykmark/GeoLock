#include "../lock/LockState.hpp"
#include "../ui/LockControls.hpp"

#include <Geode/modify/EditorPauseLayer.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/GJWorldNode.hpp>
#include <Geode/modify/LevelAreaInnerLayer.hpp>
#include <Geode/modify/LevelPage.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/SecretLayer2.hpp>

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
    void levelComplete() {
        PlayLayer::levelComplete();
        geolock::clearLockOnCompletion(m_level, m_isPracticeMode);
    }
};

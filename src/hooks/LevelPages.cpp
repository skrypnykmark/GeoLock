#include "../ui/LockControls.hpp"

#include <Geode/modify/EditLevelLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>

using namespace geode::prelude;

class $modify(GeoLockLevelInfoLayer, LevelInfoLayer) {
    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) {
            return false;
        }

        geolock::addLockButton(this, m_level, geolock::LockButtonLayout::Online,
            menu_selector(GeoLockLevelInfoLayer::onLockButton));
        geolock::refreshPlayButton(this, m_level);
        return true;
    }

    void onLockButton(CCObject*) {
        geolock::openLockControls(this, m_level);
    }

    void onPlay(CCObject* sender) {
        if (!geolock::blockLevelLaunch(m_level)) {
            LevelInfoLayer::onPlay(sender);
        }
    }

    void onPlayReplay(CCObject* sender) {
        if (!geolock::blockLevelLaunch(m_level)) {
            LevelInfoLayer::onPlayReplay(sender);
        }
    }
};

class $modify(GeoLockEditLevelLayer, EditLevelLayer) {
    bool init(GJGameLevel* level) {
        if (!EditLevelLayer::init(level)) {
            return false;
        }

        geolock::addLockButton(this, m_level, geolock::LockButtonLayout::Editor,
            menu_selector(GeoLockEditLevelLayer::onLockButton));
        geolock::refreshPlayButton(this, m_level);
        return true;
    }

    void onLockButton(CCObject*) {
        geolock::openLockControls(this, m_level);
    }

    void onPlay(CCObject* sender) {
        if (!geolock::blockLevelLaunch(m_level)) {
            EditLevelLayer::onPlay(sender);
        }
    }

    void onTest(CCObject* sender) {
        if (!geolock::blockLevelLaunch(m_level)) {
            EditLevelLayer::onTest(sender);
        }
    }
};

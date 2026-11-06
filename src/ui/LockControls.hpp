#pragma once

#include <Geode/cocos/cocoa/CCObject.h>

class GJGameLevel;

namespace cocos2d {
    class CCLayer;
}

namespace geolock {
    enum class LockButtonLayout { Online, Editor };

    void addLockButton(cocos2d::CCLayer* page, GJGameLevel* level, LockButtonLayout layout,
        cocos2d::SEL_MenuHandler onPress);
    void refreshPlayButton(cocos2d::CCLayer* page, GJGameLevel* level);
    void openLockControls(cocos2d::CCLayer* page, GJGameLevel* level);
    bool blockLevelLaunch(GJGameLevel* level);
    bool blockUnsupportedLaunch();
}

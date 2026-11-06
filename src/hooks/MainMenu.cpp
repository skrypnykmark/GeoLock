#include "../ui/LockRoomPopup.hpp"

#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

class $modify(GeoLockMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        auto icon = CCSprite::create("lock.png"_spr);
        if (icon) {
            icon->setScale(27.f / icon->getContentHeight());
            auto button = CCMenuItemExt::createSpriteExtra(icon, [](CCMenuItemSpriteExtra*) {
                geolock::showLockRoom();
            });
            button->setID("lock-room-button"_spr);

            auto size = icon->getScaledContentSize();
            button->setContentSize(size);
            button->setPosition(size / 2.f);
            auto menu = CCMenu::createWithItem(button);
            menu->setID("lock-room-menu"_spr);
            menu->ignoreAnchorPointForPosition(false);
            menu->setContentSize(size);
            this->addChildAtPosition(menu, Anchor::TopRight, { -19.f, -19.f }, false);
        }
        return true;
    }
};

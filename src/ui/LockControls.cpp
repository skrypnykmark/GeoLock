#include "LockControls.hpp"

#include "../lock/LockState.hpp"

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>

using namespace geode::prelude;

namespace geolock {
    namespace {
        CCPoint positionInPage(CCLayer* page, CCNode* node) {
            auto worldPosition = node->getParent()->convertToWorldSpace(node->getPosition());
            return page->convertToNodeSpace(worldPosition);
        }

        CCPoint onlineLockPosition(CCLayer* page, CCSize buttonSize) {
            auto gear = page->getChildByIDRecursive("settings-button");
            auto heart = page->getChildByIDRecursive("favorite-button");
            CCPoint position{ 60.f, 60.f };
            constexpr float gap = 8.f;

            if (gear) {
                position = positionInPage(page, gear);
                position.x += gear->getScaledContentSize().width / 2.f + gap + buttonSize.width / 2.f;
            }
            if (heart) {
                auto heartPosition = positionInPage(page, heart);
                position.y = heartPosition.y + heart->getScaledContentSize().height / 2.f + gap + buttonSize.height / 2.f;
                if (!gear) {
                    position.x = heartPosition.x;
                }
            }

            return { position.x + 4.f, position.y + 4.f };
        }

        CCPoint editorLockPosition(CCLayer* page) {
            auto gear = page->getChildByIDRecursive("settings-button");
            if (!gear) {
                return { 40.f, 90.f };
            }

            auto gearPosition = positionInPage(page, gear);
            auto info = page->getChildByIDRecursive("info-button");
            auto spacing = info ? gearPosition.getDistance(positionInPage(page, info)) : 45.f;
            return { gearPosition.x, gearPosition.y + spacing };
        }

        void setPlayImagesVisible(CCMenuItemSpriteExtra* button, bool visible) {
            for (auto image : { button->getNormalImage(), button->getSelectedImage(), button->getDisabledImage() }) {
                if (image) {
                    image->setVisible(visible);
                }
            }
        }

        void showBlockedMessage(const Lock& lock) {
            auto message = fmt::format("Go back to \"{}\" (ID: {}).", lock.levelName, lock.levelId);
            FLAlertLayer::create(nullptr, message, "OK")->show();
        }
    }

    void addLockButton(CCLayer* page, LockButtonLayout layout, SEL_MenuHandler onPress) {
        if (activeLock()) {
            return;
        }

        auto sprite = CCSprite::create("lock.png"_spr);
        if (!sprite) {
            return;
        }

        sprite->setScale(26.f / sprite->getContentSize().height);
        auto size = sprite->getScaledContentSize();
        auto button = CCMenuItemSpriteExtra::create(sprite, page, onPress);
        button->setContentSize(size);
        button->setID("lock-button"_spr);
        button->setPosition({ 0.f, 0.f });

        auto menu = CCMenu::create();
        menu->setID("lock-menu"_spr);
        menu->setPosition(layout == LockButtonLayout::Online ? onlineLockPosition(page, size) : editorLockPosition(page));
        menu->addChild(button);
        page->addChild(menu);
    }

    void refreshPlayButton(CCLayer* page, GJGameLevel* level) {
        auto button = typeinfo_cast<CCMenuItemSpriteExtra*>(page->getChildByIDRecursive("play-button"));
        if (!button) {
            return;
        }

        auto lockIcon = button->getChildByID("lock-icon"_spr);
        auto blocked = isLevelBlocked(level);
        if (blocked == (lockIcon != nullptr)) {
            return;
        }

        // Keep the original button so its layout and input handling still work.
        if (blocked) {
            auto icon = CCSprite::create("lock.png"_spr);
            if (!icon) {
                return;
            }

            auto size = button->getContentSize();
            icon->setScale(size.height * 0.8f / icon->getContentSize().height);
            icon->setPosition({ size.width / 2.f, size.height / 2.f });
            icon->setID("lock-icon"_spr);
            setPlayImagesVisible(button, false);
            button->addChild(icon);
        } else {
            lockIcon->removeFromParent();
            setPlayImagesVisible(button, true);
        }
    }

    void confirmLock(CCLayer* page, GJGameLevel* level) {
        if (!level) {
            return;
        }

        createQuickPopup(nullptr, "Are you sure you want to lock in?", "No", "Yes",
            [page = Ref(page), level = Ref(level)](FLAlertLayer*, bool confirmed) {
                if (!confirmed || !startLock(level)) {
                    return;
                }

                if (auto menu = page->getChildByID("lock-menu"_spr)) {
                    menu->removeFromParent();
                }
                refreshPlayButton(page, level);
            }
        );
    }

    bool blockLevelLaunch(GJGameLevel* level) {
        if (!isLevelBlocked(level)) {
            return false;
        }

        showBlockedMessage(*activeLock());
        return true;
    }

    bool blockUnsupportedLaunch() {
        const auto& lock = activeLock();
        if (!lock) {
            return false;
        }

        showBlockedMessage(*lock);
        return true;
    }
}

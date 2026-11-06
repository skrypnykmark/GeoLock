#include "LockControls.hpp"

#include "LockRoomPopup.hpp"
#include "../lock/LockState.hpp"

#include <Geode/Geode.hpp>
#include <Geode/ui/NineSlice.hpp>
#include <Geode/ui/Popup.hpp>

#include <algorithm>

using namespace geode::prelude;

namespace geolock {
    namespace {
        void showLockInFlash() {
            auto director = CCDirector::get();
            auto scene = director->getRunningScene();
            auto path = (Mod::get()->getResourcesDir() / "lock-in.jpeg").string();
            auto image = CCSprite::create(path.c_str());
            if (!scene || !image) {
                return;
            }

            auto screen = director->getWinSize();
            image->setScale(std::min(screen.width * 0.55f / image->getContentWidth(),
                screen.height * 0.51f / image->getContentHeight()));
            auto imageSize = image->getScaledContentSize();

            auto overlay = CCNode::create();
            auto backdrop = CCLayerColor::create({ 0, 0, 0, 0 }, screen.width, screen.height);
            overlay->addChild(backdrop);

            auto card = CCNodeRGBA::create();
            card->setCascadeOpacityEnabled(true);
            card->setPosition(screen / 2.f);
            card->setScale(0.94f);

            auto frame = NineSlice::create("GJ_square01.png");
            frame->setScaleMultiplier(0.55f);
            frame->setContentSize({ imageSize.width + 24.f, imageSize.height + 24.f });
            card->addChild(frame);
            card->addChild(image);
            overlay->addChild(card);
            card->setOpacity(0);
            scene->addChild(overlay, 1000);

            constexpr float duration = 3.f;
            constexpr float fadeIn = 0.12f;
            constexpr float fadeOut = 0.25f;
            constexpr float hold = duration - fadeIn - fadeOut;
            auto fade = [](GLubyte opacity) {
                return CCSequence::create(CCFadeTo::create(fadeIn, opacity), CCDelayTime::create(hold),
                    CCFadeTo::create(fadeOut, 0), nullptr);
            };
            backdrop->runAction(fade(140));
            card->runAction(fade(255));
            card->runAction(CCEaseSineOut::create(CCScaleTo::create(0.16f, 1.f)));
            overlay->runAction(CCSequence::create(CCDelayTime::create(duration),
                CCRemoveSelf::create(), nullptr));
        }

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

        void hidePlayImages(CCMenuItemSpriteExtra* button) {
            for (auto image : { button->getNormalImage(), button->getSelectedImage(), button->getDisabledImage() }) {
                if (image) {
                    image->setVisible(false);
                }
            }
        }

        void showBlockedMessage(const Lock& lock) {
            auto message = fmt::format("Go back to \"{}\" (ID: {}).", lock.levelName, lock.levelId);
            createQuickPopup(nullptr, std::move(message), "OK", nullptr,
                [](FLAlertLayer*, bool) {
                    queueInMainThread([] { showLockRoom(); });
                }, true, true);
        }
    }

    void refreshLockButton(CCLayer* page, GJGameLevel* level, LockButtonLayout layout, SEL_MenuHandler onPress) {
        auto existing = page->getChildByID("lock-menu"_spr);
        if (!level || isLevelBlocked(level)) {
            if (existing) {
                existing->removeFromParent();
            }
            return;
        }
        if (existing) {
            return;
        }

        auto sprite = CCSprite::create("lock.png"_spr);
        if (!sprite) {
            return;
        }

        sprite->setScale(26.f / sprite->getContentHeight());
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
            hidePlayImages(button);
            button->addChild(icon);
        } else {
            lockIcon->removeFromParent();
            auto image = button->isEnabled()
                ? (button->isSelected() ? button->getSelectedImage() : button->getNormalImage())
                : button->getDisabledImage();
            if (!image) {
                image = button->getNormalImage();
            }
            if (image) {
                image->setVisible(true);
            }
        }
    }

    void openLockControls(CCLayer* page, GJGameLevel* level) {
        if (!level) {
            return;
        }
        if (activeLock()) {
            if (!isLevelBlocked(level)) {
                showLockRoom(isLockTarget(level) ? LockRoomContext::TargetPage : LockRoomContext::PracticePage);
            }
            return;
        }

        createQuickPopup(nullptr, "Are you sure you want to lock in?", "No", "Yes",
            [page = Ref(page), level = Ref(level)](FLAlertLayer*, bool confirmed) {
                if (!confirmed || !startLock(level)) {
                    return;
                }

                refreshPlayButton(page, level);
                queueInMainThread([] { showLockInFlash(); });
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

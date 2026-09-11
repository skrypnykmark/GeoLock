#include <Geode/Geode.hpp>
#include <Geode/modify/EditLevelLayer.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/GJWorldNode.hpp>
#include <Geode/modify/LevelAreaInnerLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/LevelPage.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/SecretLayer2.hpp>

using namespace geode::prelude;

namespace geolock {
    // A lock is either active (a real level id is stored) or inactive.
    constexpr int NO_LOCK = -1;

    int lockedID() {
        return Mod::get()->getSavedValue<int>("locked-level-id", NO_LOCK);
    }

    std::string lockedName() {
        return Mod::get()->getSavedValue<std::string>("locked-level-name", "");
    }

    bool isLocked() {
        return lockedID() != NO_LOCK;
    }

    void lockLevel(GJGameLevel* level) {
        Mod::get()->setSavedValue<int>("locked-level-id", level->m_levelID.value());
        Mod::get()->setSavedValue<std::string>("locked-level-name", std::string(level->m_levelName));
    }

    void clearLock() {
        Mod::get()->setSavedValue<int>("locked-level-id", NO_LOCK);
    }

    // Shows the only allowed message while a lock is active.
    void showLockMessage() {
        auto message = fmt::format(
            "Go back to \"{}\" (ID: {}).", lockedName(), lockedID()
        );
        FLAlertLayer::create(nullptr, message, "OK")->show();
    }

    // Returns true when playing this level has to be stopped. When it does, the
    // lock message is shown.
    bool blocks(GJGameLevel* level) {
        if (!isLocked() || !level) {
            return false;
        }
        if (level->m_levelID.value() == lockedID()) {
            return false;
        }
        showLockMessage();
        return true;
    }

    // For level entries that have no level object handy (tower floors, secret
    // levels). None of those can ever be the locked level.
    bool blocksAny() {
        if (!isLocked()) {
            return false;
        }
        showLockMessage();
        return true;
    }
}

// The level info page (online, saved, downloaded, daily, gauntlet, ...).
class $modify(GeoLockLevelInfoLayer, LevelInfoLayer) {
    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) {
            return false;
        }

        // Only offer to lock when nothing is locked yet, so an active lock can
        // never be moved or cleared from the UI.
        if (!geolock::isLocked()) {
            this->addLockButton();
        }
        return true;
    }

    void addLockButton() {
        auto sprite = CCSprite::create("lock.png"_spr);
        if (!sprite) {
            return;
        }
        sprite->setScale(26.f / sprite->getContentSize().height);
        auto lockSize = sprite->getScaledContentSize();

        auto button = CCMenuItemSpriteExtra::create(
            sprite, this, menu_selector(GeoLockLevelInfoLayer::onLockButton)
        );
        button->setContentSize(lockSize);
        button->setID("lock-button"_spr);
        button->setPosition({ 0.f, 0.f });

        auto menu = CCMenu::create();
        menu->setID("lock-menu"_spr);
        menu->addChild(button);
        this->addChild(menu);

        // Sit in the bottom-left corner, to the right of the gear and above
        // the favourite (heart) button.
        auto gear = this->getChildByIDRecursive("settings-button");
        auto heart = this->getChildByIDRecursive("favorite-button");

        auto center = [this](CCNode* node) {
            return this->convertToNodeSpace(
                node->getParent()->convertToWorldSpace(node->getPosition())
            );
        };

        CCPoint pos = { 60.f, 60.f };
        if (gear) {
            auto p = center(gear);
            pos.x = p.x + gear->getScaledContentSize().width / 2.f + 8.f + lockSize.width / 2.f;
            pos.y = p.y;
        }
        if (heart) {
            auto p = center(heart);
            pos.y = p.y + heart->getScaledContentSize().height / 2.f + 8.f + lockSize.height / 2.f;
            if (!gear) {
                pos.x = p.x;
            }
        }
        pos.x += 4.f;
        pos.y += 4.f;
        menu->setPosition(pos);
    }

    void onLockButton(CCObject*) {
        auto level = m_level;
        if (!level) {
            return;
        }
        createQuickPopup(
            nullptr,
            "Are you sure you want to lock in?",
            "No", "Yes",
            [this, level](FLAlertLayer*, bool yes) {
                if (!yes) {
                    return;
                }
                geolock::lockLevel(level);
                if (auto menu = this->getChildByID("lock-menu"_spr)) {
                    menu->removeFromParent();
                }
            }
        );
    }

    void onPlay(CCObject* sender) {
        if (geolock::blocks(m_level)) {
            return;
        }
        LevelInfoLayer::onPlay(sender);
    }

    void onPlayReplay(CCObject* sender) {
        if (geolock::blocks(m_level)) {
            return;
        }
        LevelInfoLayer::onPlayReplay(sender);
    }
};

// Official RobTop main levels.
class $modify(GeoLockLevelPage, LevelPage) {
    void onPlay(CCObject* sender) {
        if (geolock::blocks(m_level)) {
            return;
        }
        LevelPage::onPlay(sender);
    }
};

// Saved / local levels opened from the edit list.
class $modify(GeoLockEditLevelLayer, EditLevelLayer) {
    void onPlay(CCObject* sender) {
        if (geolock::blocks(m_level)) {
            return;
        }
        EditLevelLayer::onPlay(sender);
    }

    void onTest(CCObject* sender) {
        if (geolock::blocks(m_level)) {
            return;
        }
        EditLevelLayer::onTest(sender);
    }
};

// World levels.
class $modify(GeoLockWorldNode, GJWorldNode) {
    void onLevel(CCObject* sender) {
        if (geolock::blocks(m_level)) {
            return;
        }
        GJWorldNode::onLevel(sender);
    }
};

// The Tower / area floors.
class $modify(GeoLockAreaLayer, LevelAreaInnerLayer) {
    void onDoor(CCObject* sender) {
        if (geolock::blocksAny()) {
            return;
        }
        LevelAreaInnerLayer::onDoor(sender);
    }
};

// Secret levels.
class $modify(GeoLockSecretLayer, SecretLayer2) {
    void onSecretLevel(CCObject* sender) {
        if (geolock::blocksAny()) {
            return;
        }
        SecretLayer2::onSecretLevel(sender);
    }
};

// The editor playtest starts from the toolbar button and from the pause menu's
// "Save and Play". Block both before the editor starts tearing itself down.
class $modify(GeoLockEditorUI, EditorUI) {
    void onPlaytest(CCObject* sender) {
        if (geolock::blocks(m_editorLayer ? m_editorLayer->m_level : nullptr)) {
            return;
        }
        EditorUI::onPlaytest(sender);
    }
};

class $modify(GeoLockEditorPauseLayer, EditorPauseLayer) {
    void onSaveAndPlay(CCObject* sender) {
        if (geolock::blocks(m_editorLayer ? m_editorLayer->m_level : nullptr)) {
            return;
        }
        EditorPauseLayer::onSaveAndPlay(sender);
    }
};

class $modify(GeoLockPlayLayer, PlayLayer) {
    void levelComplete() {
        PlayLayer::levelComplete();

        if (!m_isPracticeMode && geolock::isLocked() && m_level &&
            m_level->m_levelID.value() == geolock::lockedID()) {
            geolock::clearLock();
        }
    }
};

#include "PracticeCopyPopup.hpp"

#include "LockControls.hpp"
#include "../lock/LockState.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/LocalLevelManager.hpp>
#include <Geode/ui/NineSlice.hpp>
#include <Geode/ui/Notification.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/ui/Scrollbar.hpp>
#include <cvolton.level-id-api/include/EditorIDs.hpp>

#include <algorithm>

using namespace geode::prelude;

namespace geolock {
    namespace {
        constexpr float LIST_WIDTH = 234.f;
        constexpr float LIST_HEIGHT = 156.f;
        constexpr float LIST_OFFSET_Y = -15.f;
        constexpr float PANEL_WIDTH = 242.f;
        constexpr float ROW_WIDTH = 224.f;
        constexpr float ROW_HEIGHT = 23.f;
        constexpr ccColor3B ROW_COLOR{ 112, 63, 38 };
        constexpr ccColor3B SELECTED_ROW_COLOR{ 70, 113, 56 };
        constexpr float SELECTION_DURATION = 0.2f;

        NineSlice* panel(CCSize size, ccColor3B color, GLubyte opacity) {
            auto background = NineSlice::create("square02b_001.png");
            background->setScaleMultiplier(0.3f);
            background->setContentSize(size);
            background->setColor(color);
            background->setOpacity(opacity);
            return background;
        }

        class PracticeCopyPopup : public Popup {
            LockRoomContext m_context;
            int m_targetId = 0;
            LevelSource m_targetSource = LevelSource::Legacy;
            CCMenuItemSpriteExtra* m_selectedButton = nullptr;
            CCLabelBMFont* m_missingNotice = nullptr;

            explicit PracticeCopyPopup(LockRoomContext context) : m_context(context) {}

            bool init() override {
                const auto& lock = activeLock();
                if (!lock || !Popup::init(280.f, 230.f)) {
                    return false;
                }

                m_targetId = lock->levelId;
                m_targetSource = lock->source;
                this->setTitle("Practice Copy", "goldFont.fnt", 0.62f);
                this->addLevelList(practiceCopy());
                return true;
            }

            void addLevelList(GJGameLevel* attachedCopy) {
                auto heading = CCLabelBMFont::create("My Levels", "bigFont.fnt");
                heading->setAnchorPoint({ 0.f, 0.5f });
                heading->setColor({ 255, 221, 173 });
                heading->setScale(0.35f);
                m_mainLayer->addChildAtPosition(heading, Anchor::TopLeft, { 28.f, -38.f });

                if (!attachedCopy && activeLock()->practiceCopyId) {
                    m_missingNotice = CCLabelBMFont::create("Previous copy missing", "chatFont.fnt");
                    m_missingNotice->setAnchorPoint({ 1.f, 0.5f });
                    m_missingNotice->setColor({ 237, 208, 174 });
                    m_missingNotice->setScale(0.72f);
                    m_mainLayer->addChildAtPosition(m_missingNotice, Anchor::TopRight, { -28.f, -38.f });
                }

                auto listBackground = panel({ PANEL_WIDTH, LIST_HEIGHT + 8.f }, { 47, 23, 12 }, 130);
                m_mainLayer->addChildAtPosition(listBackground, Anchor::Center, { 0.f, LIST_OFFSET_Y });

                auto list = ScrollLayer::create(CCSize{ LIST_WIDTH, LIST_HEIGHT });
                list->setStealingTouches(true);
                auto content = list->m_contentLayer;
                content->setLayout(ScrollLayer::createDefaultListLayout(3.f));

                CCMenu* selectedRow = nullptr;
                auto levels = LocalLevelManager::get()->getCreatedLevels(0);
                for (auto level : CCArrayExt<GJGameLevel*>(levels)) {
                    if (level->m_levelType != GJLevelType::Editor || isLockTarget(level)) {
                        continue;
                    }
                    auto button = this->createLevelButton(level);
                    auto row = CCMenu::createWithItem(button);
                    row->setContentSize({ ROW_WIDTH, ROW_HEIGHT });
                    row->ignoreAnchorPointForPosition(false);
                    button->setPosition(row->getContentSize() / 2.f);
                    content->addChild(row);

                    if (level == attachedCopy) {
                        selectedRow = row;
                        m_selectedButton = button;
                        this->setSelectedColor(button, true, false);
                    }
                }

                if (content->getChildrenCount() == 0) {
                    auto empty = CCLabelBMFont::create("No other created levels", "chatFont.fnt");
                    empty->setColor({ 237, 208, 174 });
                    m_mainLayer->addChildAtPosition(empty, Anchor::Center, { 0.f, LIST_OFFSET_Y });
                    return;
                }

                m_mainLayer->addChildAtPosition(list, Anchor::Center,
                    { -LIST_WIDTH / 2.f, LIST_OFFSET_Y - LIST_HEIGHT / 2.f });
                content->updateLayout();
                list->scrollToTop();
                auto scrollRange = content->getContentHeight() - LIST_HEIGHT;
                if (scrollRange > 0.f) {
                    if (selectedRow) {
                        auto centeredOffset = LIST_HEIGHT / 2.f - selectedRow->getPositionY();
                        content->setPositionY(std::clamp(centeredOffset, -scrollRange, 0.f));
                    }
                    m_mainLayer->addChildAtPosition(Scrollbar::create(list), Anchor::Right, { -14.f, LIST_OFFSET_Y });
                }
            }

            CCMenuItemSpriteExtra* createLevelButton(GJGameLevel* level) {
                auto background = panel({ ROW_WIDTH, ROW_HEIGHT }, ROW_COLOR, 250);

                auto name = CCLabelBMFont::create(level->m_levelName.c_str(), "bigFont.fnt");
                name->setAnchorPoint({ 0.f, 0.5f });
                name->limitLabelWidth(150.f, 0.37f, 0.18f);
                background->addChildAtPosition(name, Anchor::Left, { 9.f, 0.f });

                if (auto id = EditorIDs::getID(level, false); id > 0) {
                    auto text = fmt::format("#{}", id);
                    auto idLabel = CCLabelBMFont::create(text.c_str(), "chatFont.fnt");
                    idLabel->setAnchorPoint({ 1.f, 0.5f });
                    idLabel->setColor({ 237, 208, 174 });
                    idLabel->limitLabelWidth(49.f, 0.75f, 0.5f);
                    background->addChildAtPosition(idLabel, Anchor::Right, { -9.f, 0.f });
                }

                auto pick = CCMenuItemSpriteExtra::create(background, this, menu_selector(PracticeCopyPopup::onPick));
                pick->m_animationEnabled = false;
                pick->setUserObject(level);
                return pick;
            }

            void setSelectedColor(CCMenuItemSpriteExtra* button, bool selected, bool animated = true) {
                auto background = static_cast<NineSlice*>(button->getNormalImage());
                auto color = selected ? SELECTED_ROW_COLOR : ROW_COLOR;
                background->stopAllActions();
                if (animated) {
                    background->runAction(CCTintTo::create(SELECTION_DURATION, color.r, color.g, color.b));
                } else {
                    background->setColor(color);
                }
            }

            void onPick(CCObject* sender) {
                auto button = static_cast<CCMenuItemSpriteExtra*>(sender);
                auto selection = button == m_selectedButton ? nullptr : button;
                auto copy = selection ? static_cast<GJGameLevel*>(selection->getUserObject()) : nullptr;
                const auto& lock = activeLock();
                if (!lock || lock->levelId != m_targetId || lock->source != m_targetSource || !setPracticeCopy(copy)) {
                    Notification::create("Could not update the practice copy", NotificationIcon::Error)->show();
                    return;
                }

                PracticeCopyChanged().send();
                if (m_selectedButton) {
                    this->setSelectedColor(m_selectedButton, false);
                }
                m_selectedButton = selection;
                if (m_selectedButton) {
                    this->setSelectedColor(m_selectedButton, true);
                }
                if (m_missingNotice) {
                    m_missingNotice->setVisible(false);
                }
            }

            void onClose(CCObject* sender) override {
                auto context = m_context;
                Popup::onClose(sender);
                queueInMainThread([context] { showLockRoom(context); });
            }

        public:
            static PracticeCopyPopup* create(LockRoomContext context) {
                auto popup = new PracticeCopyPopup(context);
                if (popup->init()) {
                    popup->autorelease();
                    return popup;
                }

                delete popup;
                return nullptr;
            }
        };
    }

    void showPracticeCopyPopup(LockRoomContext context) {
        if (auto popup = PracticeCopyPopup::create(context)) {
            popup->show();
        }
    }
}

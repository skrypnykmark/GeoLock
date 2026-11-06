#include "LockRoomPopup.hpp"

#include "PracticeCopyPopup.hpp"
#include "../hooks/OnlineLevelDownload.hpp"
#include "../lock/LockState.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/GameLevelManager.hpp>
#include <Geode/binding/LevelSelectLayer.hpp>
#include <Geode/ui/NineSlice.hpp>
#include <Geode/ui/Popup.hpp>
#include <fmt/chrono.h>

#include <algorithm>
#include <ctime>

using namespace geode::prelude;

namespace geolock {
    namespace {
        constexpr ccColor3B MUTED_TEXT{ 246, 216, 176 };
        constexpr ccColor3B ERROR_TEXT{ 255, 191, 151 };

        NineSlice* panel(CCSize size, ccColor3B color, GLubyte opacity) {
            auto background = NineSlice::create("square02b_001.png");
            background->setScaleMultiplier(0.3f);
            background->setContentSize(size);
            background->setColor(color);
            background->setOpacity(opacity);
            return background;
        }

        GJGameLevel* cachedOnlineTarget(int id) {
            auto manager = GameLevelManager::get();
            for (auto level : { manager->getSavedLevel(id), manager->getSavedDailyLevelFromLevelID(id),
                     manager->getSavedGauntletLevel(id) }) {
                if (level && !level->m_levelString.empty()) {
                    return level;
                }
            }
            return nullptr;
        }

        std::string percentText(const std::optional<int>& percent) {
            return percent ? fmt::format("{}%", *percent) : "N/A";
        }

        class LockRoomPopup : public Popup {
            LockRoomContext m_context;
            CCLabelBMFont* m_status = nullptr;
            std::optional<int> m_loadingId;

            explicit LockRoomPopup(LockRoomContext context) : m_context(context) {}

            bool init() override {
                const auto& lock = activeLock();
                const bool canOpenTarget = m_context != LockRoomContext::TargetPage;
                const bool canOpenPractice = m_context != LockRoomContext::PracticePage && lock && practiceCopy();
                const bool hasBothDestinations = canOpenTarget && canOpenPractice;
                if (!Popup::init(304.f, hasBothDestinations ? 246.f : 216.f)) {
                    return false;
                }

                this->setTitle("Lock Room", "goldFont.fnt", 0.7f);
                if (!lock) {
                    this->addLabel("No active lock", "bigFont.fnt", { 152.f, 119.f }, 0.6f);
                    this->addLabel("Open a level and press its lock button.", "chatFont.fnt",
                        { 152.f, 93.f }, 0.95f)->setColor(MUTED_TEXT);
                    return true;
                }

                this->addLockDetails(*lock);
                auto actionY = hasBothDestinations ? 65.f : 36.f;
                if (canOpenTarget) {
                    this->addButton("Open Target", "GJ_button_01.png", { 87.f, actionY },
                        menu_selector(LockRoomPopup::onOpenTarget));
                }
                if (canOpenPractice) {
                    this->addButton("Open Practice", "GJ_button_02.png", { hasBothDestinations ? 217.f : 87.f, actionY },
                        menu_selector(LockRoomPopup::onOpenCopy));
                }
                auto choosePosition = hasBothDestinations ? CCPoint{ 152.f, 32.f }
                    : CCPoint{ canOpenTarget || canOpenPractice ? 217.f : 152.f, actionY };
                this->addButton("Choose Practice", "GJ_button_04.png", choosePosition,
                    menu_selector(LockRoomPopup::onChoosePractice));

                m_status = this->addLabel("", "chatFont.fnt", { 152.f, m_size.height - 155.f }, 0.8f);
                return true;
            }

            CCLabelBMFont* addLabel(const char* text, const char* font, CCPoint position, float scale) {
                auto label = CCLabelBMFont::create(text, font);
                label->setScale(scale);
                label->setPosition(position);
                m_mainLayer->addChild(label);
                return label;
            }

            void addLockDetails(const Lock& lock) {
                this->addLabel(lock.levelName.c_str(), "bigFont.fnt", { 152.f, m_size.height - 48.f }, 0.57f)
                    ->limitLabelWidth(260.f, 0.57f, 0.2f);

                auto date = lock.progress.startedAt
                    ? fmt::format("Locked {:%b %d, %Y}", geode::localtime(static_cast<std::time_t>(*lock.progress.startedAt)))
                    : std::string("Lock date unavailable");
                auto details = fmt::format("{}  |  ID {}", date, lock.levelId);
                this->addLabel(details.c_str(), "chatFont.fnt", { 152.f, m_size.height - 71.f }, 0.9f)->setColor(MUTED_TEXT);

                auto statsPanel = panel({ 270.f, 49.f }, { 47, 23, 12 }, 130);
                statsPanel->setPosition({ 152.f, m_size.height - 119.f });
                m_mainLayer->addChild(statsPanel);

                const auto& progress = lock.progress;
                auto best = progress.newBests.empty()
                    ? std::string(progress.startingBest ? "No run yet" : "Unavailable")
                    : fmt::format("{}%", progress.newBests.back());
                this->addMetric("Starting best", percentText(progress.startingBest), 62.f);
                this->addMetric(progress.startedAt ? "Best since lock" : "Best recorded", best, 152.f);
                this->addMetric("Locked attempts", fmt::format("{}", progress.attempts), 242.f);
            }

            void addMetric(const char* caption, const std::string& value, float x) {
                this->addLabel(caption, "chatFont.fnt", { x, m_size.height - 108.f }, 0.72f)->setColor(MUTED_TEXT);
                this->addLabel(value.c_str(), "bigFont.fnt", { x, m_size.height - 128.f }, 0.42f)
                    ->limitLabelWidth(83.f, 0.42f, 0.23f);
            }

            void addButton(const char* text, const char* texture, CCPoint position, SEL_MenuHandler selector) {
                auto sprite = ButtonSprite::create(text, 112, true, "bigFont.fnt", texture, 27.f, 0.38f);
                auto size = sprite->getContentSize();
                sprite->setScale(std::min({ 1.f, 112.f / size.width, 27.f / size.height }));

                auto button = CCMenuItemSpriteExtra::create(sprite, this, selector);
                button->updateSprite();
                button->m_scaleMultiplier = 0.96f;
                m_buttonMenu->addChild(button);
                button->setPosition(position);
            }

            void setStatus(const char* text, bool error = false) {
                m_status->setString(text);
                m_status->setColor(error ? ERROR_TEXT : MUTED_TEXT);
                m_status->limitLabelWidth(275.f, 0.8f, 0.45f);
            }

            void openLevelPage(GJGameLevel* level) {
                this->onClose(nullptr);
                GameLevelManager::get()->gotoLevelPage(level);
            }

            void onOpenTarget(CCObject*) {
                const auto& lock = activeLock();
                if (!lock || m_loadingId) {
                    return;
                }

                if (lock->source == LevelSource::Official) {
                    auto scene = LevelSelectLayer::scene(std::max(0, lock->levelId - 1));
                    if (scene) {
                        this->onClose(nullptr);
                        CCDirector::get()->replaceScene(CCTransitionFade::create(0.35f, scene));
                    } else {
                        this->setStatus("Could not open the level. Try again.", true);
                    }
                    return;
                }

                if (lock->source == LevelSource::Editor || lock->source == LevelSource::Legacy) {
                    auto local = findCreatedLevel(lock->levelId);
                    if (local && (lock->source == LevelSource::Editor || lock->levelName == local->m_levelName.c_str())) {
                        this->openLevelPage(local);
                        return;
                    }
                    if (lock->source == LevelSource::Editor) {
                        this->setStatus("Target is missing from My Levels. Try again to check.", true);
                        return;
                    }
                }

                if (auto level = cachedOnlineTarget(lock->levelId); level && this->isOnlineTarget(level)) {
                    this->openLevelPage(level);
                    return;
                }
                this->fetchTarget(lock->levelId);
            }

            bool isOnlineTarget(GJGameLevel* level) {
                if (!level || level->m_levelString.empty() || !isLockTarget(level)) {
                    return false;
                }

                const auto& lock = *activeLock();
                // Legacy locks may share their numeric ID with a local level.
                return lock.source != LevelSource::Legacy || lock.levelName == level->m_levelName.c_str() ||
                    !findCreatedLevel(lock.levelId);
            }

            void onOpenCopy(CCObject*) {
                if (auto copy = practiceCopy()) {
                    this->openLevelPage(copy);
                } else {
                    this->setStatus("Practice copy is missing. Use Choose Practice to replace it.", true);
                }
            }

            void onChoosePractice(CCObject*) {
                if (activeLock()) {
                    auto context = m_context;
                    this->onClose(nullptr);
                    queueInMainThread([context] { showPracticeCopyPopup(context); });
                }
            }

            void fetchTarget(int id) {
                m_loadingId = id;
                this->setStatus("Fetching level...");
                auto requested = requestOnlineLevel(id, [popup = WeakRef(this), id](GJGameLevel* level) {
                    auto room = popup.lock();
                    if (room && room->getParent() && room->m_loadingId == id) {
                        room->finishDownload(level);
                    }
                });
                if (requested) {
                    this->scheduleOnce(schedule_selector(LockRoomPopup::onDownloadTimeout), 15.f);
                } else {
                    this->finishDownload(nullptr);
                }
            }

            void finishDownload(GJGameLevel* level) {
                m_loadingId.reset();
                this->unschedule(schedule_selector(LockRoomPopup::onDownloadTimeout));
                if (this->isOnlineTarget(level)) {
                    this->openLevelPage(level);
                } else {
                    this->setStatus("Could not load the target. Press Open Target to retry.", true);
                }
            }

            void onDownloadTimeout(float) {
                if (m_loadingId) {
                    cancelOnlineLevelRequest(*m_loadingId);
                    this->finishDownload(nullptr);
                }
            }

            void onExit() override {
                if (m_loadingId) {
                    cancelOnlineLevelRequest(*m_loadingId);
                    m_loadingId.reset();
                    this->unschedule(schedule_selector(LockRoomPopup::onDownloadTimeout));
                }
                Popup::onExit();
            }

        public:
            static LockRoomPopup* create(LockRoomContext context) {
                auto popup = new LockRoomPopup(context);
                if (popup->init()) {
                    popup->autorelease();
                    return popup;
                }

                delete popup;
                return nullptr;
            }
        };
    }

    void showLockRoom(LockRoomContext context) {
        if (auto popup = LockRoomPopup::create(context)) {
            popup->show();
        }
    }
}

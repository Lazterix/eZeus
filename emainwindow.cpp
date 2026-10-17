#include "emainwindow.h"

#include "widgets/emainmenu.h"
#include "widgets/esettingsmenu.h"
#include "widgets/egamewidget.h"
#include "widgets/egameloadingwidget.h"
#include "widgets/egamemenu.h"
#include "widgets/emenuloadingwidget.h"
#include "widgets/eworldwidget.h"
#include "widgets/echoosegameeditmenu.h"
#include "widgets/eselectcolonywidget.h"

#include "textures/egametextures.h"

#include "audio/emusic.h"

#include "engine/ethreadpool.h"

#include "egamedir.h"

#include "fileIO/ereadstream.h"

#include <chrono>
#include <filesystem>
#include <fstream>

#include "widgets/efilewidget.h"
#include "elanguage.h"

#include "evectorhelpers.h"

#include "widgets/eeventbackground.h"
#include "widgets/eepisodeintroductionwidget.h"
#include "widgets/eepisodelostwidget.h"
#include "widgets/erosterofleaders.h"
#include "widgets/edisplaymode.h"
#include "widgets/edisplaytransition.h"
#include "widgets/esoundsettingswidget.h"

eMainWindow::eMainWindow() {}

eMainWindow::~eMainWindow() {
    // Destroy widgets while their renderer-backed textures are still valid.
    if(mGW) {
        mGW->setBoard(nullptr);
        if(mWidget == mGW) mWidget = nullptr;
        mGW->deleteLater();
        mGW = nullptr;
    }
    if(mWW) {
        if(mWidget == mWW) mWidget = nullptr;
        mWW->deleteLater();
        mWW = nullptr;
    }
    if(mWidget) {
        mWidget->deleteLater();
        mWidget = nullptr;
    }

    for(const auto& slot : mSlots) {
        slot();
    }
    mSlots.clear();

    // Global texture collections must release SDL_Texture instances before
    // the renderer they belong to is destroyed.
    eGameTextures::shutdown();

    if(mSdlRenderer) SDL_DestroyRenderer(mSdlRenderer);
    mSdlRenderer = nullptr;
    if(mSdlWindow) SDL_DestroyWindow(mSdlWindow);
    mSdlWindow = nullptr;
}

bool eMainWindow::initialize(const eSettings& settings) {
    eSettings applied = settings;
    auto resolutions = eDisplayTransition::availableResolutions(
        nullptr, applied.fRes);
    applied.fRes = eDisplayModeModel::validatedResolution(
        applied.fRes, resolutions);
    eResolution::sResolutions = resolutions;

    const auto& res = applied.fRes;
    const int w = res.width();
    const int h = res.height();
    const auto window = SDL_CreateWindow("eZeus",
                                         SDL_WINDOWPOS_UNDEFINED,
                                         SDL_WINDOWPOS_UNDEFINED,
                                         w, h, SDL_WINDOW_SHOWN);

    if(!window) {
        printf("Window could not be created! SDL Error: %s\n",
               SDL_GetError());
        return false;
    }
    const Uint32 flags = SDL_RENDERER_ACCELERATED/* |
                         SDL_RENDERER_PRESENTVSYNC*/;
    const auto renderer = SDL_CreateRenderer(window, -1, flags);
    if(!renderer) {
        printf("Renderer could not be created! SDL Error: %s\n",
               SDL_GetError());
        SDL_DestroyWindow(window);
        return false;
    }

    if(mSdlRenderer) SDL_DestroyRenderer(mSdlRenderer);
    if(mSdlWindow) SDL_DestroyWindow(mSdlWindow);
    mSdlWindow = window;
    mSdlRenderer = renderer;
    mSettings = applied;
    std::string displayError;
    if(!eDisplayTransition::apply(window, renderer, res,
                                  applied.fFullscreen,
                                  mWindowedPlacement,
                                  displayError)) {
        printf("Display mode could not be applied: %s\n",
               displayError.c_str());
        applied.fFullscreen = false;
        if(!eDisplayTransition::apply(window, renderer, res, false,
                                      mWindowedPlacement,
                                      displayError)) {
            printf("Windowed display fallback failed: %s\n",
                   displayError.c_str());
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            mSdlRenderer = nullptr;
            mSdlWindow = nullptr;
            return false;
        }
        mSettings = applied;
    }
    eResolution::sResolutions =
        eDisplayTransition::availableResolutions(window, res);

    if(settings.fRes != applied.fRes ||
       settings.fFullscreen != applied.fFullscreen) {
        mSettings.write();
    }

    const std::string icoPath = eGameDir::path("zeus.ico");
    const auto icon = IMG_Load(icoPath.c_str());
    if(icon) {
        SDL_SetWindowIcon(window, icon);
        SDL_FreeSurface(icon);
    }
    eGameTextures::setSettings(mSettings);
    return true;
}

void eMainWindow::setWidget(eWidget* const w) {
    if(mWidget == mGW && w != mGW && mGW) {
        mGW->closeTopMenu();
    }
    if(mWidget) {
        if(mWidget != mGW && mWidget != mWW) {
            mWidget->deleteLater();
        }
    }
    mWidget = w;
}

eWidget* eMainWindow::takeWidget() {
    const auto w = mWidget;
    mWidget = nullptr;
    return w;
}

void eMainWindow::addSlot(const eSlot& slot) {
    mSlots.push_back(slot);
}

bool eMainWindow::applyDisplaySettings(const eResolution& res,
                                       const bool fullscreen) {
    const auto found = std::find(eResolution::sResolutions.begin(),
                                 eResolution::sResolutions.end(), res);
    if(found == eResolution::sResolutions.end()) return false;

    std::string error;
    if(!eDisplayTransition::apply(mSdlWindow, mSdlRenderer,
                                  res, fullscreen,
                                  mWindowedPlacement, error)) {
        printf("Display mode could not be applied: %s\n", error.c_str());
        return false;
    }
    mSettings.fRes = res;
    mSettings.fFullscreen = fullscreen;
    return true;
}

void eMainWindow::startGameAction(eGameBoard* const board,
                                  const eGameWidgetSettings& settings) {
    const auto show = [this, board, settings]() {
        showGame(board, settings);
    };
    startGameAction(show);
}

void eMainWindow::startGameAction(const stdsptr<eCampaign>& c,
                                  const eGameWidgetSettings& settings) {
    const auto show = [this, c, settings]() {
        showGame(c, settings);
    };
    startGameAction(show);
}

void eMainWindow::startGameAction(const eAction& a) {
    clearWidgets();
    const auto l = new eGameLoadingWidget(this);
    l->resize(width(), height());
    l->setDoneAction(a);
    setWidget(l);
    l->initialize();
}

void eMainWindow::showEpisodeIntroduction(
        const stdsptr<eCampaign>& c) {
    clearWidgets();
    if(c) mCampaign = c;
    const auto e = new eEpisodeIntroductionWidget(this);
    const auto proceedA = [this]() {
        mCampaign->startEpisode();
        const auto dir = leaderSaveDir();
        saveGame(dir + "autosave replay.ez");
        startGameAction([this]() {
            eGameWidgetSettings settings;
            settings.fPaused = true;
            showGame(mCampaign, settings);
        });
    };
    e->resize(width(), height());
    const auto ee = mCampaign->currentEpisode();

    const auto path = mCampaign->currentEpisodeAudioFilePath(true);
    const bool played = eMusic::playCampaignVoice(path);
    if(!played) eMusic::playMissionIntroMusic();

    e->initialize(mCampaign,
                  mCampaign->titleText(),
                  ee->fIntroduction,
                  ee->fGoals,
                  proceedA,
                  eEpisodeIntroType::intro);
    setWidget(e);
}

std::string eMainWindow::leaderSaveDir() const {
    return eGameDir::saveDir() + mLeader + "/";
}

void eMainWindow::clearWidgets() {
    if(mGW && mWidget != mGW) {
        mGW->setBoard(nullptr);
        mGW->deleteLater();
        mGW = nullptr;
    }
    if(mWW && mWidget != mWW) {
        mWW->deleteLater();
        mWW = nullptr;
    }
}

void eMainWindow::episodeFinished() {
    clearWidgets();
    if(!mCampaign) return;
    mCampaign->episodeFinished();
    const bool f = mCampaign->finished();
    if(f) return adventureComplete();
    const auto n = mCampaign->currentEpisodeType();
    if(n == eEpisodeType::parentCity) {
        showEpisodeIntroduction();
    } else {
        const auto w = new eSelectColonyWidget(this);
        const auto sel = mCampaign->remainingColonies();
        const auto selA = [this](const stdsptr<eWorldCity>& c) {
            int cid = 0;
            const auto& eps = mCampaign->colonyEpisodes();
            for(const auto& e : eps) {
                if(e->fCity == c) break;
                cid++;
            }
            mCampaign->setCurrentColonyEpisode(cid);
            showEpisodeIntroduction();
        };
        w->resize(width(), height());
        w->initialize(sel, selA, &mCampaign->worldBoard());
        setWidget(w);
    }
}

void eMainWindow::adventureComplete() {
    clearWidgets();
    if(!mCampaign) return;
    const auto e = new eEpisodeIntroductionWidget(this);
    const auto proceedA = [this]() {
        showMainMenu();
    };
    e->resize(width(), height());

    const auto path = mCampaign->adventureVictoryAudioFilePath();
    const bool played = eMusic::playCampaignVoice(path);
    if(!played) eMusic::playCampaignVictoryMusic();

    e->initialize(mCampaign,
                  eLanguage::zeusText(62, 0),
                  mCampaign->completeText(),
                  {},
                  proceedA,
                  eEpisodeIntroType::campaingVictory);
    setWidget(e);
}

void eMainWindow::episodeLost() {
    clearWidgets();
    const auto e = new eEpisodeLostWidget(this);
    const auto proceedA = [this]() {
        showMainMenu();
    };
    e->resize(width(), height());
    e->initialize(proceedA);
    setWidget(e);
}

bool eMainWindow::saveGame(const std::string& path) {
    if(!mCampaign) return false;

    const auto fsp = std::filesystem::path(path);
    const auto fspd = fsp.parent_path();
    std::filesystem::create_directories(fspd);
    std::ofstream file(path, std::ios::out | std::ios::binary |
                       std::ios::trunc);
    if(!file) return false;
    eWriteTarget target(&file);
    eWriteStream dst(target);
    dst.writeFormat("eZeus.ez");
    if(mGW) {
        const auto s = mGW->settings();
        s.write(dst);
    } else {
        eGameWidgetSettings s;
        s.fPaused = true;
        s.write(dst);
    }
    mCampaign->write(dst);
    file.close();
    return true;
}

void eMainWindow::setAutosave(const bool enabled) {
    if(mSettings.fAutosave == enabled) return;
    mSettings.fAutosave = enabled;
    mSettings.write();
}

bool eMainWindow::loadGame(const std::string& path) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if(!file) return false;
    eReadSource source(&file);
    eReadStream src(source);
    src.readFormat();
    if(src.failed()) {
        printf("Cannot read '%s': truncated file header.\n", path.c_str());
        return false;
    }
    const auto& format = src.format();
    const int version = src.formatVersion();
    if(format != "eZeus.ez") {
        printf("Invalid file '%s' format '%s', expected 'eZeus.ez'.\n",
               path.c_str(), format.c_str());
        return false;
    }
    if(version > eFileFormat::version) {
        printf("Cannot read '%s' format '%s' version '%i': newer than executable version '%i'.\n",
               path.c_str(), format.c_str(), version,
               eFileFormat::version);
        return false;
    }
    eGameWidgetSettings s;
    s.read(src);
    const auto c = std::make_shared<eCampaign>();
    c->read(src);
    if(src.failed()) {
        printf("Cannot read '%s': truncated save data.\n", path.c_str());
        return false;
    }
    c->loadStrings();
    c->loadNumbers();
    src.handlePostFuncs();
    file.close();

    startGameAction(c, s);
    return true;
}

void eMainWindow::closeGame() {
    if(!mGW) return;
    if(mGW) {
        mGW->setBoard(nullptr);
        mGW->deleteLater();
        mGW = nullptr;
    }
    if(mWW) {
        mWW->deleteLater();
        mWW = nullptr;
    }
    showMainMenu();
}

void eMainWindow::showRosterOfLeaders() {
    clearWidgets();
    eMusic::playMenuMusic();
    const auto rol = new eRosterOfLeaders(this);
    rol->resize(width(), height());
    rol->initialize();
    setWidget(rol);
}

void eMainWindow::showMenuLoading() {
    const auto mlw = new eMenuLoadingWidget(this);
    mlw->setDoneAction([this]() {
        const auto ls = eRosterOfLeaders::sLeaders();
        if(ls.size() == 1) setLeader(ls[0]);
        if(mLeader.empty()) {
            showRosterOfLeaders();
        } else {
            showMainMenu();
        }
    });
    mlw->initialize();
    mlw->resize(width(), height());
    setWidget(mlw);
}

void eMainWindow::showMainMenu() {
    mCampaign = nullptr;
    clearWidgets();
    eMusic::playMenuMusic();

    const auto mm = new eMainMenu(this);
    mm->resize(width(), height());
    setWidget(mm);

    const auto newGameAction = [this]() {
        showChooseGameMenu();
    };

    const auto loadGameAction = [this, mm]() {
        const auto fw = new eFileWidget(this);
        const auto func = [this](const std::string& path) {
            return loadGame(path);
        };
        const auto closeAct = [mm, fw]() {
            mm->removeWidget(fw);
            fw->deleteLater();
        };
        const auto dir = leaderSaveDir();
        fw->intialize(eLanguage::zeusText(1, 3),
                      dir, func, closeAct);
        mm->addWidget(fw);
        fw->align(eAlignment::center);
    };

    const auto editGameAction = [this]() {
        showChooseGameEditMenu();
    };

    const auto settingsAction = [this]() {
        showSettingsMenu([this]() { showMainMenu(); });
    };

    const auto quitAction = [this]() {
        mQuit = true;
    };

    const auto leaderAction = [this]() {
        showRosterOfLeaders();
    };

    mm->initialize(newGameAction,
                   loadGameAction,
                   editGameAction,
                   settingsAction,
                   quitAction,
                   leaderAction);
}

void eMainWindow::showSettingsMenu(const eAction& returnAction,
                                   const bool displayOnly) {
    const bool activeGame = mGW && mWidget == mGW;
    const eGameWidgetSettings gameSettings =
        activeGame ? mGW->settings() : eGameWidgetSettings{};
    const eMessageHistory messageHistory =
        activeGame ? mGW->messageHistory() : eMessageHistory{};

    eResolution::sResolutions = eDisplayTransition::availableResolutions(
        mSdlWindow, mSettings.fRes);
    const auto esm = new eSettingsMenu(mSettings, this);
    esm->resize(width(), height());

    const auto applyA = [this, activeGame, gameSettings, messageHistory,
                         returnAction, displayOnly](const eSettings& settings) {
        eSettings applied = settings;

        if(displayOnly) {
            applied.fTinyTextures = mSettings.fTinyTextures;
            applied.fSmallTextures = mSettings.fSmallTextures;
            applied.fMediumTextures = mSettings.fMediumTextures;
            applied.fLargeTextures = mSettings.fLargeTextures;
        }

        if(!applied.fTinyTextures &&
           !applied.fSmallTextures &&
           !applied.fMediumTextures &&
           !applied.fLargeTextures) {
            const auto enableIfPresent = [](const std::string& path,
                                            bool& enabled) {
                if(!std::filesystem::exists(path)) return false;
                enabled = true;
                return true;
            };

            bool recovered =
                enableIfPresent(eGameDir::i30BinaryPath(),
                                applied.fSmallTextures);
            if(!recovered) {
                recovered = enableIfPresent(eGameDir::i15BinaryPath(),
                                            applied.fTinyTextures);
            }
            if(!recovered) {
                recovered = enableIfPresent(eGameDir::i45BinaryPath(),
                                            applied.fMediumTextures);
            }
            if(!recovered) {
                enableIfPresent(eGameDir::i60BinaryPath(),
                                applied.fLargeTextures);
            }
        }

        const bool textureSetChanged =
            applied.fTinyTextures != mSettings.fTinyTextures ||
            applied.fSmallTextures != mSettings.fSmallTextures ||
            applied.fMediumTextures != mSettings.fMediumTextures ||
            applied.fLargeTextures != mSettings.fLargeTextures;
        const bool resolutionChanged = applied.fRes != mSettings.fRes;
        if(!applyDisplaySettings(applied.fRes, applied.fFullscreen)) return;

        mSettings.fTinyTextures = applied.fTinyTextures;
        mSettings.fSmallTextures = applied.fSmallTextures;
        mSettings.fMediumTextures = applied.fMediumTextures;
        mSettings.fLargeTextures = applied.fLargeTextures;
        mSettings.fAutosave = applied.fAutosave;
        mSettings.write();
        eGameTextures::setSettings(mSettings);

        if(activeGame && resolutionChanged) {
            if(mGW) {
                mGW->setBoard(nullptr);
                mGW->deleteLater();
                mGW = nullptr;
            }
            if(mWW) {
                mWW->deleteLater();
                mWW = nullptr;
            }
            mGW = new eGameWidget(this);
            mGW->setBoard(mBoard);
            mGW->resize(width(), height());
            mGW->initialize();
            mGW->setMessageHistory(messageHistory);
            mGW->setSettings(gameSettings);
            setWidget(mGW);
        } else if(activeGame) {
            returnToGame();
        } else if(resolutionChanged || textureSetChanged) {
            showMenuLoading();
        } else if(returnAction) {
            returnAction();
        } else {
            showMainMenu();
        }
    };
    const auto cancelA = [this, returnAction, activeGame]() {
        if(activeGame) returnToGame();
        else if(returnAction) returnAction();
        else showMainMenu();
    };
    esm->initialize(applyA, cancelA, displayOnly);
    setWidget(esm);
}

void eMainWindow::returnToGame() {
    if(mGW) setWidget(mGW);
}

void eMainWindow::showSoundSettings() {
    const auto soundSettings = new eSoundSettingsWidget(this);
    soundSettings->initialize(mSettings, [this](const eSettings& settings) {
        mSettings.fMusicEnabled = settings.fMusicEnabled;
        mSettings.fSoundsEnabled = settings.fSoundsEnabled;
        mSettings.fMusicVolume = settings.fMusicVolume;
        mSettings.fSoundsVolume = settings.fSoundsVolume;
        mSettings.applyAudio();
        mSettings.write();
    });
    execDialog(soundSettings, true, nullptr, mGW);
    soundSettings->align(eAlignment::center);
}

void eMainWindow::showChooseGameMenu() {
    const auto gem = new eChooseGameEditMenu(this);
    gem->resize(width(), height());
    gem->initialize(false);
    setWidget(gem);
}

void eMainWindow::showChooseGameEditMenu() {
    const auto gem = new eChooseGameEditMenu(this);
    gem->resize(width(), height());
    gem->initialize(true);
    setWidget(gem);
}

void eMainWindow::showGame(const stdsptr<eCampaign>& c,
                           const eGameWidgetSettings& settings) {
    if(!c) return;
    const auto e = c->currentEpisode();
    if(!e || !e->fBoard) return;
    mCampaign = c;
    showGame(e->fBoard, settings);
}

void eMainWindow::showGame(eGameBoard* b,
                           const eGameWidgetSettings& settings) {
    if(!b) b = mBoard;

    if(b == mBoard && mGW) {
        return setWidget(mGW);
    }

    if(mGW) {
        mGW->setBoard(nullptr);
        mGW->deleteLater();
        mGW = nullptr;
    }

    mBoard = b;
    if(mBoard) {
        mBoard->updateAppealMapIfNeeded();
        mBoard->waitUntilFinished();
    }

    eMusic::playRandomMusic();
    mGW = new eGameWidget(this);
    mGW->setBoard(b);
    mGW->resize(width(), height());
    mGW->initialize();
    mGW->setSettings(settings);
    setWidget(mGW);
}

void eMainWindow::showWorld() {
    if(mWidget == mWW) return;
    if(!mCampaign) return;
    if(!mWW) {
        mWW = new eWorldWidget(this);
        mWW->resize(width(), height());
        mWW->initialize();
        mWW->setBoard(mBoard);
    } else {
        mWW->update();
    }
    setWidget(mWW);
}

void eMainWindow::execDialog(
        eWidget* const d, const bool closable,
        const eAction &closeFunc,
        eWidget* const parent) {
    if(!mWidget) return;
    const auto bg = new eEventBackground(this);
    if(closeFunc) {
        bg->initialize(parent ? parent : mWidget, d, closable, closeFunc);
    } else {
        const auto closeFunc = [d]() {
            d->deleteLater();
        };
        bg->initialize(parent ? parent : mWidget, d, closable, closeFunc);
    }
}

class eTooltip {
public:
    eTooltip(eMainWindow& w) : mWindow(w) {}

    void update() {
        const auto txt = eWidget::sTooltip();
        const bool updateTxt = mText != txt;
        if(updateTxt) {
            mText = txt;
        }

        const auto& res = mWindow.resolution();
        const int fontSize = res.verySmallFontSize();
        const bool updateFont = mFontSize != fontSize;
        if(updateFont) {
            mFontSize = fontSize;
            mFont = eFonts::defaultFont(mFontSize);
        }

        const bool updateTexture = updateTxt || updateFont;
        if(updateTexture) {
            const auto r = mWindow.renderer();
            if(mText.empty()) {
                mTexture->reset();
            } else {
                mTexture->loadText(r, mText, eFontColor::light, *mFont, 50*fontSize);
            }
        }
    }

    void paint(const int x, const int y, ePainter& p) {
        const int pp = padding();
        SDL_Rect rect{x, y, width(), height()};
        p.fillRect(rect, SDL_Color{16, 108, 144, 255});
        p.drawRect(rect, SDL_Color{0, 32, 32, 255}, 1);
        p.drawTexture(x + pp, y + pp, mTexture);
    }

    int width() const { return mTexture->width() + 2*padding(); }
    int height() const { return mTexture->height() + 2*padding(); }

    bool empty() const { return mText.empty(); }
private:
    int padding() const { return mFontSize/2; }

    eMainWindow& mWindow;
    int mFontSize = -1;
    TTF_Font* mFont = nullptr;
    std::string mText;
    stdsptr<eTexture> mTexture = std::make_shared<eTexture>();
};

int eMainWindow::exec() {
    using namespace std::chrono;
    using namespace std::chrono_literals;

    showMenuLoading();

    eMouseButton button{eMouseButton::none};
    eMouseButton buttons{eMouseButton::none};

    SDL_Event e;
    eTooltip tooltip(*this);

    const bool showFPS = false;
    const double fpsClamp = 20;

    int c = 0;
    int fpsVal = 0;
    bool resetRenderTargets = false;
    while(!mQuit) {
        const auto fpsStart = high_resolution_clock::now();

        while(SDL_PollEvent(&e)) {
            int x, y;
            SDL_GetMouseState(&x, &y);
            const bool shift = mShiftPressed > 0;
            const bool ctrl = mCtrlPressed > 0;
            if(e.type == SDL_QUIT) {
                mQuit = true;
            } else if(e.type == SDL_WINDOWEVENT) {
                const auto we = e.window.event;
                if(we == SDL_WINDOWEVENT_MINIMIZED) {
                    resetRenderTargets = true;
                    while(SDL_WaitEvent(&e)) {
                        if(e.window.event == SDL_WINDOWEVENT_RESTORED) {
                            break;
                        }
                    }
                } else if(we == SDL_WINDOWEVENT_EXPOSED) {
                    resetRenderTargets = true;
                }
            } else if(e.type == SDL_RENDER_TARGETS_RESET ||
                      e.type == SDL_RENDER_DEVICE_RESET) {
                resetRenderTargets = true;
            } else if(e.type == SDL_MOUSEMOTION) {
                const eMouseEvent me(x, y, shift, ctrl, buttons, button);
                if(mWidget) mWidget->mouseMove(me);
            } else if(e.type == SDL_MOUSEBUTTONDOWN) {
                switch(e.button.button) {
                case SDL_BUTTON_LEFT:
                    button = eMouseButton::left;
                    break;
                case SDL_BUTTON_RIGHT:
                    button = eMouseButton::right;
                    break;
                case SDL_BUTTON_MIDDLE:
                    button = eMouseButton::middle;
                    break;
                default: continue;
                }
                buttons = button | buttons;

                const eMouseEvent me(x, y, shift, ctrl, buttons, button);
                if(mWidget) mWidget->mousePress(me);
            } else if(e.type == SDL_MOUSEBUTTONUP) {
                switch(e.button.button) {
                case SDL_BUTTON_LEFT:
                    button = eMouseButton::left;
                    break;
                case SDL_BUTTON_RIGHT:
                    button = eMouseButton::right;
                    break;
                case SDL_BUTTON_MIDDLE:
                    button = eMouseButton::middle;
                    break;
                default: continue;
                }
                buttons = buttons & ~button;
                const eMouseEvent me(x, y, shift, ctrl, buttons, button);
                if(mWidget) mWidget->mouseRelease(me);
            } else if(e.type == SDL_MOUSEWHEEL) {
                const eMouseWheelEvent me(x, y, shift, ctrl, buttons, e.wheel.y);
                if(mWidget) mWidget->mouseWheel(me);
            } else if(e.type == SDL_KEYDOWN) {
                const auto k = e.key.keysym.scancode;
                if(k == SDL_Scancode::SDL_SCANCODE_LSHIFT ||
                   k == SDL_Scancode::SDL_SCANCODE_RSHIFT) {
                    mShiftPressed++;
                } else if(k == SDL_Scancode::SDL_SCANCODE_LCTRL ||
                          k == SDL_Scancode::SDL_SCANCODE_RCTRL) {
                    mCtrlPressed++;
                }
                const eKeyPressEvent ke(x, y, shift, ctrl, buttons, k);
                if(mWidget) mWidget->keyPress(ke);
            } else if(e.type == SDL_KEYUP) {
                const auto k = e.key.keysym.scancode;
                if(k == SDL_Scancode::SDL_SCANCODE_LSHIFT ||
                   k == SDL_Scancode::SDL_SCANCODE_RSHIFT) {
                    mShiftPressed--;
                } else if(k == SDL_Scancode::SDL_SCANCODE_LCTRL ||
                          k == SDL_Scancode::SDL_SCANCODE_RCTRL) {
                    mCtrlPressed--;
                }
            }
        }

        if(resetRenderTargets) {
            resetRenderTargets = false;
            if(mWidget) mWidget->renderTargetsReset();
        }

        SDL_SetRenderDrawColor(mSdlRenderer, 0x0, 0x0, 0x0, 0xFF);
        SDL_RenderClear(mSdlRenderer);

        ePainter p(mSdlRenderer);

        eMusic::incTime();
        if(mWidget) {
            mWidget->paint(p);
            tooltip.update();
            if(!tooltip.empty()) {
                const auto& res = resolution();
                const int pp = 25*res.multiplier();
                const int wtt = tooltip.width();
                const int htt = tooltip.height();
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                int xtt;
                int ytt;
                if(mx > width()/2) {
                    xtt = mx - wtt;
                } else {
                    xtt = mx;
                }
                if(my > height()/2) {
                    ytt = my - htt - pp;
                } else {
                    ytt = my + pp;
                }
                tooltip.paint(xtt, ytt, p);
            }
        }

        if(showFPS) {
            p.setFont(eFonts::defaultFont(resolution()));
            p.drawText(0, 0, std::to_string(fpsVal), eFontColor::dark);
        }

        SDL_RenderPresent(mSdlRenderer);

        for(const auto& s : mSlots) {
            s();
        }
        mSlots.clear();

        const auto fpsEnd = high_resolution_clock::now();
        const duration<double, std::milli> fpsElapsed = fpsEnd - fpsStart;
        const duration<double, std::milli> fpsDuration(1000./fpsClamp);
        const duration<double, std::milli> fpsSleep(fpsDuration - fpsElapsed);
        std::this_thread::sleep_for(fpsSleep);

        if(showFPS) {
            c++;
            if(c % 25 == 0) {
                const auto fpsEnd = high_resolution_clock::now();
                const duration<double, std::milli> fpsElapsed = fpsEnd - fpsStart;
                fpsVal = (int)std::round(1000./fpsElapsed.count());
            }
        }
    }

    return 0;
}

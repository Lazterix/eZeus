#include "estorageinfowidget.h"

#include "engine/egameboard.h"

#include "elanguage.h"

#include "emainwindow.h"
#include "egamedir.h"
#include "widgets/echoosebutton.h"
#include "widgets/eframedbutton.h"
#include "widgets/escrollwidget.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <optional>
#include <string>
#include <utility>

// eZeus warehouse presets v1 (UI-only, no save schema changes)
namespace {
constexpr int kStoragePresetSlots = 6;
constexpr int kStoragePresetVersion = 1;

struct eStoragePreset {
    eBuildingType fKind = eBuildingType::warehouse;
    eResourceType fSupported = eResourceType::none;
    eResourceType fGet = eResourceType::none;
    eResourceType fEmpty = eResourceType::none;
    eResourceType fAccept = eResourceType::none;
    int fSpaceCount = 0;
    std::map<eResourceType, int> fLimits;
};

std::optional<eStoragePreset>& storageClipboard() {
    static std::optional<eStoragePreset> clipboard;
    return clipboard;
}

std::string storagePresetFile() {
    // Kept alongside settings.txt, outside proprietary original-game assets.
    return eGameDir::exeDir() + "../storage-presets.txt";
}

eStoragePreset captureStoragePreset(const eStorageBuilding& storage) {
    eStoragePreset preset;
    preset.fKind = storage.type();
    preset.fSupported = storage.canAccept();
    preset.fSpaceCount = storage.spaceCount();
    storage.getOrders(preset.fGet, preset.fEmpty, preset.fAccept);
    preset.fLimits = storage.maxCount();
    return preset;
}

bool isStorageType(const eBuildingType type) {
    return type == eBuildingType::warehouse || type == eBuildingType::granary;
}

bool validateStoragePreset(const eStoragePreset& preset) {
    if(!isStorageType(preset.fKind)) return false;
    if(preset.fSpaceCount < 1 || preset.fSpaceCount > 15) return false;
    const auto supported = static_cast<int>(preset.fSupported);
    const auto get = static_cast<int>(preset.fGet);
    const auto empty = static_cast<int>(preset.fEmpty);
    const auto accept = static_cast<int>(preset.fAccept);
    if(supported < 0 || get < 0 || empty < 0 || accept < 0) return false;
    if((get & ~supported) || (empty & ~supported) || (accept & ~supported)) return false;
    if((get & empty) || (empty & accept)) return false;
    if(preset.fLimits.size() > 32) return false;
    for(const auto& [type, count] : preset.fLimits) {
        const int bit = static_cast<int>(type);
        if(bit <= 0 || (bit & (bit - 1)) != 0 || (bit & ~supported)) return false;
        const int max = (type == eResourceType::sculpture ? 1 : 4)*preset.fSpaceCount;
        if(count < 0 || count > max) return false;
    }
    return true;
}

// Separate versioned file: old .ez save files remain byte-for-byte compatible.
bool loadStoragePresets(std::map<int, eStoragePreset>& presets) {
    std::ifstream file(storagePresetFile());
    if(!file) {
        presets.clear(); // First run: no slots saved yet.
        return true;
    }
    std::string magic;
    int version = 0;
    int count = 0;
    if(!(file >> magic >> version >> count) ||
       magic != "EZEUS_STORAGE_PRESETS" ||
       version != kStoragePresetVersion || count < 0 ||
       count > kStoragePresetSlots) return false;

    std::map<int, eStoragePreset> parsed;
    for(int i = 0; i < count; ++i) {
        int slot, kind, supported, get, empty, accept, capacity, countLimits;
        if(!(file >> slot >> kind >> supported >> get >> empty >> accept >> capacity >> countLimits) ||
           slot < 0 || slot >= kStoragePresetSlots || countLimits < 0 ||
           countLimits > 32 || parsed.count(slot)) return false;
        eStoragePreset p;
        p.fKind = static_cast<eBuildingType>(kind);
        p.fSupported = static_cast<eResourceType>(supported);
        p.fGet = static_cast<eResourceType>(get);
        p.fEmpty = static_cast<eResourceType>(empty);
        p.fAccept = static_cast<eResourceType>(accept);
        p.fSpaceCount = capacity;
        for(int j = 0; j < countLimits; ++j) {
            int type, limit;
            if(!(file >> type >> limit)) return false;
            const auto resource = static_cast<eResourceType>(type);
            if(p.fLimits.count(resource)) return false;
            p.fLimits[resource] = limit;
        }
        if(!validateStoragePreset(p)) return false;
        parsed.emplace(slot, std::move(p));
    }
    std::string trailing;
    if(file >> trailing) return false; // Reject truncated/extra unexpected content.
    presets = std::move(parsed);
    return true;
}

bool saveStoragePresets(const std::map<int, eStoragePreset>& presets) {
    const std::string path = storagePresetFile();
    const std::string tmp = path + ".tmp";
    {
        std::ofstream file(tmp, std::ios::trunc);
        if(!file) return false;
        file << "EZEUS_STORAGE_PRESETS " << kStoragePresetVersion << ' '
             << presets.size() << '\n';
        for(const auto& [slot, p] : presets) {
            if(slot < 0 || slot >= kStoragePresetSlots || !validateStoragePreset(p)) return false;
            file << slot << ' ' << static_cast<int>(p.fKind) << ' '
                 << static_cast<int>(p.fSupported) << ' '
                 << static_cast<int>(p.fGet) << ' '
                 << static_cast<int>(p.fEmpty) << ' '
                 << static_cast<int>(p.fAccept) << ' '
                 << p.fSpaceCount << ' ' << p.fLimits.size() << '\n';
            for(const auto& [resource, limit] : p.fLimits) {
                file << static_cast<int>(resource) << ' ' << limit << '\n';
            }
        }
        file.flush();
        if(!file.good()) return false;
    }
    // Preserve the existing valid file on failure when possible.
    // Windows' rename cannot replace an existing file, so stage the prior
    // config as a backup before installing the new one.
    const std::string backup = path + ".bak";
    std::remove(backup.c_str());
    bool hadPrevious = false;
    {
        std::ifstream current(path);
        hadPrevious = current.good();
    }
    if(hadPrevious && std::rename(path.c_str(), backup.c_str()) != 0) {
        std::remove(tmp.c_str());
        return false;
    }
    if(std::rename(tmp.c_str(), path.c_str()) != 0) {
        std::rename(backup.c_str(), path.c_str());
        std::remove(tmp.c_str());
        return false;
    }
    std::remove(backup.c_str());
    return true;
}

bool applyStoragePreset(const eStoragePreset& preset,
                        eStorageBuilding& storage,
                        const std::map<eResourceType, eSwitchButton*>& buttons,
                        const std::map<eResourceType, eSpinBox*>& spinBoxes) {
    if(!validateStoragePreset(preset) || preset.fKind != storage.type()) return false;
    if(preset.fSpaceCount != storage.spaceCount()) return false;
    const auto types = eResourceTypeHelpers::extractResourceTypes(storage.canAccept());
    for(const auto type : types) {
        const auto bi = buttons.find(type);
        const auto si = spinBoxes.find(type);
        if(bi == buttons.end() || si == spinBoxes.end()) return false;
    }
    eResourceType get = eResourceType::none;
    eResourceType empty = eResourceType::none;
    eResourceType accept = eResourceType::none;
    std::map<eResourceType, int> limits;
    for(const auto type : types) {
        // Cross-scenario compatibility: unsupported source goods are disabled.
        const bool supported = static_cast<bool>(preset.fSupported & type);
        int mode = 0;
        if(supported && static_cast<bool>(preset.fGet & type)) {
            mode = 2;
            get = get | type;
        } else if(supported && static_cast<bool>(preset.fEmpty & type)) {
            mode = 3;
            empty = empty | type;
        } else if(supported && static_cast<bool>(preset.fAccept & type)) {
            mode = 1;
            accept = accept | type;
        }
        auto* button = buttons.at(type);
        button->setValue(mode);
        if(mode == 0 || mode == 3) button->setDarkFontColor();
        else button->setLightFontColor();

        const auto found = preset.fLimits.find(type);
        const int maximum = (type == eResourceType::sculpture ? 1 : 4)*storage.spaceCount();
        const int wanted = found == preset.fLimits.end() ? maximum : found->second;
        const int limit = std::clamp(wanted, 0, maximum);
        spinBoxes.at(type)->setValue(limit);
        limits[type] = limit;
    }
    // Inventory and moving carts are left untouched.
    storage.setOrders(get, empty, accept);
    storage.setMaxCount(limits);
    return true;
}
} // namespace

class eResourceStorageStack : public eWidget {
public:
    using eWidget::eWidget;

    void initialize(eStorageBuilding* const stor,
                    const std::vector<eResourceType>& types,
                    const eResourceType get,
                    const eResourceType empty,
                    const eResourceType accept,
                    std::map<eResourceType, eSwitchButton*>& buttons,
                    std::map<eResourceType, eSpinBox*>& spinBoxes,
                    const std::map<eResourceType, int>& maxCount,
                    const eAction& changed) {
        auto& board = stor->getBoard();

        const auto countW = new eWidget(window());
        const auto iconsW = new eWidget(window());
        const auto namesW = new eWidget(window());
        const auto buttonsW = new eWidget(window());
        const auto spinsW = new eWidget(window());

        const auto res = resolution();
        const double mult = res.multiplier();
        const int rowHeight = mult*23;
        const int countWidth = mult*25;
        const int iconsWidth = mult*40;
        const int namesWidth = mult*120;
        const int buttonsWidth = mult*160;
        const int spinsWidth = mult*90;

        buttonsW->setWidth(namesWidth);

        for(const auto type : types) {
            const auto count = new eLabel(window());
            count->setSmallFontSize();
            const int c = stor->count(type);
            count->setText(std::to_string(c));
            count->fitContent();
            count->setHeight(rowHeight);

            const auto icon = new eLabel(window());
            const auto tex = eResourceTypeHelpers::icon(
                                 res.uiScale(), type);
            icon->setTexture(tex);
            icon->fitContent();
            icon->setHeight(rowHeight);
            icon->setVisible(c > 0);

            const auto nameStr = eResourceTypeHelpers::typeName(type);
            eLabel* n = nullptr;
            if(board.editorMode()) {
                const auto button = new eButton(window());
                button->setPressAction([stor, type, count, icon]() {
                    stor->add(type, 1);
                    const int c = stor->count(type);
                    count->setText(std::to_string(c));
                    icon->setVisible(c > 0);
                });
                n = button;
            } else {
                n = new eLabel(window());
            }
            n->setSmallFontSize();
            n->setText(nameStr);
            n->fitContent();
            n->setHeight(rowHeight);

            const auto b = new eSwitchButton(window());
            b->setUnderline(false);

            b->setSwitchAction([b, changed](const int i) {
                if(i == 0 || i == 3) {
                    b->setDarkFontColor();
                } else {
                    b->setLightFontColor();
                }
                changed();
            });

            b->setSmallFontSize();
            b->addValue(eLanguage::zeusText(130, 1)); // don't accept
            b->addValue(eLanguage::zeusText(130, 0)); // accept
            b->addValue(eLanguage::zeusText(130, 2)); // get
            b->addValue(eLanguage::zeusText(130, 3)); // empty
            b->fitContent();
            b->setHeight(rowHeight);

            buttons[type] = b;

            const auto s = new eSpinBox(window());
            s->setHeight(rowHeight);
            s->setWidth(spinsWidth);
            s->initialize(changed);
            s->label()->setSmallFontSize();
            const int space = stor->spaceCount();
            if(type == eResourceType::sculpture) {
                s->setRange(0, space);
            } else {
                s->setRange(0, 4*space);
                s->setIncrement(4);
            }
            s->setValue(maxCount.at(type));

            spinBoxes[type] = s;

            countW->addWidget(count);
            iconsW->addWidget(icon);
            namesW->addWidget(n);
            buttonsW->addWidget(b);
            spinsW->addWidget(s);

            icon->align(eAlignment::left);
            n->align(eAlignment::left);
            b->align(eAlignment::hcenter);

            if(static_cast<bool>(get & type)) {
                b->setValue(2);
            } else if(static_cast<bool>(empty & type)) {
                b->setValue(3);
                b->setDarkFontColor();
            } else if(static_cast<bool>(accept & type)) {
                b->setValue(1);
            } else {
                b->setValue(0);
                b->setDarkFontColor();
            }
        }

        {
            const auto b = new eFramedButton(window());
            b->setUnderline(false);
            b->setSmallFontSize();
            b->setText(eLanguage::zeusText(130, 1));
            b->setDarkFontColor();
            buttonsW->addWidget(b);
            const auto it = buttons.begin();
            const auto b0 = it->second;
            const int w = b0->width();
            const int h = b0->height();
            b->resize(w, h);
            b->align(eAlignment::hcenter);

            b->setPressAction([changed, buttons]() {
                for(const auto it : buttons) {
                    const auto b = it.second;
                    b->setValue(0);
                    b->setDarkFontColor();
                }
                changed();
            });
        }

        countW->stackVertically();
        iconsW->stackVertically();
        namesW->stackVertically();
        buttonsW->stackVertically();
        spinsW->stackVertically();

        const int h = types.size()*rowHeight;
        countW->setHeight(h);
        iconsW->setHeight(h);
        namesW->setHeight(h);
        const int bh = h + rowHeight;
        buttonsW->setHeight(bh);
        spinsW->setHeight(h);

        countW->setWidth(countWidth);
        iconsW->setWidth(iconsWidth);
        namesW->setWidth(namesWidth);
        buttonsW->setWidth(buttonsWidth);
        spinsW->setWidth(spinsWidth);

        addWidget(countW);
        addWidget(iconsW);
        addWidget(namesW);
        addWidget(buttonsW);
        addWidget(spinsW);

        stackHorizontally();
        setNoPadding();
        fitContent();
    }
};

eStorageInfoWidget::eStorageInfoWidget(
        eMainWindow* const window, eMainWidget* const mw) :
    eEmployingBuildingInfoWidget(window, mw, true, false) {}

void eStorageInfoWidget::initialize(eStorageBuilding* const stor) {
    const auto title = eBuilding::sNameForBuilding(stor);

    eEmployingBuildingInfoWidget::initialize(title, stor);

    eResourceType get;
    eResourceType empty;
    eResourceType accept;
    stor->getOrders(get, empty, accept);
    const auto all = stor->canAccept();
    const auto& maxCount = stor->maxCount();

    const auto stWid = centralWidget();

    const auto types = eResourceTypeHelpers::extractResourceTypes(all);

    const auto changed = [this, stor]() {
        std::map<eResourceType, int> maxCount;
        eResourceType get;
        eResourceType empty;
        eResourceType accept;
        eResourceType dontaccept;
        this->get(get, empty, accept, dontaccept, maxCount);
        stor->setOrders(get, empty, accept);
        stor->setMaxCount(maxCount);
    };

    const auto controls = new eWidget(window());
    controls->setNoPadding();
    stWid->addWidget(controls);
    const auto actions = new eWidget(window());
    actions->setNoPadding();
    controls->addWidget(actions);
    const auto feedback = new eLabel(window());
    feedback->setSmallFontSize();
    feedback->setText("Copy or save storage settings");
    feedback->fitContent();
    controls->addWidget(feedback);
    const auto setFeedback = [feedback](const std::string& message) {
        feedback->setText(message);
        feedback->fitContent();
    };

    const auto addAction = [this, actions](const std::string& name, const eAction& act) {
        const auto button = new eFramedButton(window());
        button->setUnderline(false);
        button->setSmallFontSize();
        button->setSmallPadding();
        button->setText(name);
        button->fitContent();
        button->setPressAction(act);
        actions->addWidget(button);
    };

    addAction("Copy", [stor, setFeedback]() {
        storageClipboard() = captureStoragePreset(*stor);
        setFeedback("Storage settings copied");
    });
    addAction("Paste", [this, stor, setFeedback]() {
        const auto& clipboard = storageClipboard();
        if(!clipboard) {
            setFeedback("Nothing copied yet");
            return;
        }
        const bool success = applyStoragePreset(*clipboard, *stor, mButtons, mSpinBoxes);
        setFeedback(success ? "Settings pasted" : "Incompatible storage type");
    });

    const auto chooseSlot = [this, stor, setFeedback](const bool save) {
        std::map<int, eStoragePreset> presets;
        if(!loadStoragePresets(presets)) {
            setFeedback("Cannot read storage-presets.txt");
            return;
        }
        std::vector<std::string> labels;
        for(int i = 0; i < kStoragePresetSlots; ++i) {
            std::string label = "Slot " + std::to_string(i + 1);
            const auto it = presets.find(i);
            if(it == presets.end()) label += " (empty)";
            else label += (it->second.fKind == eBuildingType::granary) ? " (Granary)" : " (Warehouse)";
            labels.push_back(label);
        }
        const auto chooser = new eChooseButton(window());
        chooser->initialize(kStoragePresetSlots, labels,
            [this, stor, setFeedback, save](const int slot) {
                std::map<int, eStoragePreset> updated;
                if(!loadStoragePresets(updated)) {
                    setFeedback("Cannot read storage-presets.txt");
                    return;
                }
                if(save) {
                    // Never overwrite user presets unless the slot was chosen.
                    updated[slot] = captureStoragePreset(*stor);
                    if(saveStoragePresets(updated)) {
                        setFeedback("Saved preset slot " + std::to_string(slot + 1));
                    } else {
                        setFeedback("Could not save preset file");
                    }
                } else {
                    const auto found = updated.find(slot);
                    if(found == updated.end()) {
                        setFeedback("Selected preset slot is empty");
                        return;
                    }
                    const bool success = applyStoragePreset(
                        found->second, *stor, mButtons, mSpinBoxes);
                    setFeedback(success ? "Storage preset loaded" : "Incompatible storage type");
                }
            }, true);
        window()->execDialog(chooser);
        chooser->align(eAlignment::center);
    };
    addAction("Save Slot", [chooseSlot]() { chooseSlot(true); });
    addAction("Load Slot", [chooseSlot]() { chooseSlot(false); });
    actions->stackHorizontally(padding());
    actions->fitContent();
    controls->stackVertically(padding());
    controls->fitContent();
    feedback->align(eAlignment::hcenter);
    controls->align(eAlignment::hcenter);
    controls->setY(0);

    const auto scrolling = new eScrollWidget(window());
    scrolling->setNoPadding();
    scrolling->resize(stWid->width(),
        std::max(1, stWid->height() - controls->height() - padding()));
    scrolling->move(0, controls->height() + padding());
    stWid->addWidget(scrolling);

    const auto r = new eResourceStorageStack(window());
    r->initialize(stor, types, get, empty, accept,
                  mButtons, mSpinBoxes, maxCount, changed);
    r->move(std::max(0, (scrolling->width() - r->width()) / 2), 0);
    // The scroll widget's receiver is non-owning; keep the content owned
    // as a hidden child to prevent double paint outside the clip viewport.
    scrolling->addWidget(r);
    r->hide();
    scrolling->setScrollArea(r);
    scrolling->initializeButtons();
}

void eStorageInfoWidget::get(eResourceType& get,
                             eResourceType& empty,
                             eResourceType& accept,
                             eResourceType& dontaccept,
                             std::map<eResourceType, int>& count) const {
    get = eResourceType::none;
    empty = eResourceType::none;
    accept = eResourceType::none;
    dontaccept = eResourceType::none;
    for(const auto b : mButtons) {
        const auto type = b.first;
        const int val = b.second->currentValue();
        if(val == 0) {
            dontaccept = dontaccept | type;
        } else if(val == 1) {
            accept = accept | type;
        } else if(val == 2) {
            get = get | type;
        } else if(val == 3) {
            empty = empty | type;
        }
    }
    for(const auto s : mSpinBoxes) {
        const auto type = s.first;
        const int val = s.second->value();
        count[type] = val;
    }
}

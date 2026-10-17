#ifndef ETOPMENUMODEL_H
#define ETOPMENUMODEL_H

#include <vector>

namespace eTopMenuModel {
enum class eAction {
    none,
    display,
    sound,
    speed,
    difficulty,
    autosave,
    messages,
    about
};

struct eEntry {
    int fTextId;
    bool fEnabled;
    eAction fAction;
};

inline std::vector<eEntry> options(const bool autosaveEnabled) {
    return {
        {1, true, eAction::display},
        {2, true, eAction::sound},
        {3, true, eAction::speed},
        {4, false, eAction::none},
        {6, true, eAction::difficulty},
        {7, false, eAction::none},
        {autosaveEnabled ? 9 : 10, true, eAction::autosave},
        {11, false, eAction::none},
        {13, true, eAction::messages}
    };
}

inline std::vector<eEntry> help() {
    return {
        {1, false, eAction::none},
        {2, false, eAction::none},
        {5, false, eAction::none},
        {7, true, eAction::about}
    };
}
}

#endif // ETOPMENUMODEL_H

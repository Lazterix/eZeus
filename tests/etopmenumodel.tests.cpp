#include "widgets/etopmenumodel.h"
#include "widgets/esavepath.h"

#include <chrono>
#include <filesystem>
#include <fstream>

namespace {
bool testOptionsUseOneRowPerOriginalSetting() {
    const auto entries = eTopMenuModel::options(true);
    const int expectedTextIds[] = {1, 2, 3, 4, 6, 7, 9, 11, 13};
    if(entries.size() != std::size(expectedTextIds)) return false;
    for(std::size_t i = 0; i < entries.size(); ++i) {
        if(entries[i].fTextId != expectedTextIds[i]) return false;
    }
    return true;
}

bool testAutosaveRowReflectsCurrentState() {
    const auto on = eTopMenuModel::options(true);
    const auto off = eTopMenuModel::options(false);
    return on[6].fTextId == 9 && off[6].fTextId == 10 &&
           on[6].fAction == eTopMenuModel::eAction::autosave &&
           off[6].fAction == eTopMenuModel::eAction::autosave;
}

bool testOnlyImplementedOptionsAreEnabled() {
    const auto entries = eTopMenuModel::options(true);
    for(const auto& entry : entries) {
        const bool expected =
            entry.fAction == eTopMenuModel::eAction::display ||
            entry.fAction == eTopMenuModel::eAction::sound ||
            entry.fAction == eTopMenuModel::eAction::speed ||
            entry.fAction == eTopMenuModel::eAction::difficulty ||
            entry.fAction == eTopMenuModel::eAction::autosave ||
            entry.fAction == eTopMenuModel::eAction::messages;
        if(entry.fEnabled != expected) return false;
    }
    return true;
}

bool testDeleteOnlyAcceptsExistingSaveInsideLeaderFolder() {
    const auto suffix = std::to_string(
        std::chrono::steady_clock::now().time_since_epoch().count());
    const auto root = std::filesystem::temp_directory_path() /
                      ("ezeus-save-delete-test-" + suffix);
    std::filesystem::create_directories(root);
    const auto save = root / "city.ez";
    {
        std::ofstream out(save);
        out << "fixture";
    }

    const bool valid = eSavePath::isDeletable(root.string(), save.string());
    const bool rejectsOutside = !eSavePath::isDeletable(
        root.string(), (root.parent_path() / "outside.ez").string());
    const bool rejectsWrongExtension = !eSavePath::isDeletable(
        root.string(), (root / "city.txt").string());
    std::filesystem::remove_all(root);
    return valid && rejectsOutside && rejectsWrongExtension;
}

bool testHelpUsesOneRowPerOriginalSetting() {
    const auto entries = eTopMenuModel::help();
    const int expectedTextIds[] = {1, 2, 5, 7};
    if(entries.size() != std::size(expectedTextIds)) return false;
    for(std::size_t i = 0; i < entries.size(); ++i) {
        if(entries[i].fTextId != expectedTextIds[i]) return false;
        const bool expectedEnabled =
            entries[i].fAction == eTopMenuModel::eAction::about;
        if(entries[i].fEnabled != expectedEnabled) return false;
    }
    return true;
}
}

int main() {
    return testOptionsUseOneRowPerOriginalSetting() &&
           testAutosaveRowReflectsCurrentState() &&
           testOnlyImplementedOptionsAreEnabled() &&
           testDeleteOnlyAcceptsExistingSaveInsideLeaderFolder() &&
           testHelpUsesOneRowPerOriginalSetting() ? 0 : 1;
}

#ifndef ESETTINGSMENU_H
#define ESETTINGSMENU_H

#include "emainmenubase.h"
#include "esettings.h"

inline bool operator==(const eSettings& s0, const eSettings& s1) {
    if(s0.fRes != s1.fRes) return false;
    if(s0.fFullscreen != s1.fFullscreen) return false;
    return true;
}

class eSettingsMenu : public eMainMenuBase {
public:
    eSettingsMenu(const eSettings& iniSettings,
                  eMainWindow* const window);

    using eApplyAction = std::function<void(const eSettings&)>;
    void initialize(const eApplyAction& settingsA,
                    const eAction& cancelA);

protected:
    bool keyPressEvent(const eKeyPressEvent& e) override;
    bool mousePressEvent(const eMouseEvent& e) override;
private:
    void cancel();

    eSettings mSettings;
    eAction mCancelAction;
};

#endif // ESETTINGSMENU_H

#include "etopmenudropdown.h"

#include "eframedbutton.h"

#include <algorithm>

void eTopMenuDropdown::initialize(
        const std::vector<eTopMenuAction>& actions,
        const eAction& closeAction) {
    int iRes;
    int mult;
    iResAndMult(iRes, mult);

    const int frameWidth = 8*mult;
    const int rowHeight = 10*mult;
    int contentWidth = 0;
    std::vector<eFramedButton*> buttons;
    buttons.reserve(actions.size());

    setOuterFrameId(1);

    for(const auto& action : actions) {
        const auto button = new eFramedButton(action.fText, window());
        button->setNoPadding();
        button->setSmallFontSize();
        button->fitContent();
        button->setEnabled(action.fEnabled);
        if(!action.fEnabled) button->setDarkFontColor();
        button->setTextAlignment(eAlignment::left | eAlignment::vcenter);

        if(action.fEnabled) {
            const auto callback = action.fCallback;
            button->setPressAction([closeAction, callback]() {
                if(closeAction) closeAction();
                if(callback) callback();
            });
        }

        contentWidth = std::max(contentWidth, button->width());
        buttons.push_back(button);
        addWidget(button);
    }

    const int contentHeight = rowHeight*static_cast<int>(buttons.size());
    resize(contentWidth + 2*frameWidth,
           contentHeight + 2*frameWidth);

    int y = frameWidth;
    for(const auto button : buttons) {
        button->move(frameWidth, y);
        button->resize(contentWidth, rowHeight);
        y += rowHeight;
    }
}

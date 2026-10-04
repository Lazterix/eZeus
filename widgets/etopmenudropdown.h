#ifndef ETOPMENUDROPDOWN_H
#define ETOPMENUDROPDOWN_H

#include <string>
#include <vector>

#include "eframedwidget.h"

class eButton;

enum class eTopMenuId {
    file,
    options,
    help
};

struct eTopMenuAction {
    std::string fText;
    bool fEnabled = true;
    eAction fCallback;
};

class eTopMenuDropdown : public eFramedWidget {
public:
    using eFramedWidget::eFramedWidget;

    void initialize(const std::vector<eTopMenuAction>& actions,
                    const eAction& closeAction);
};

#endif // ETOPMENUDROPDOWN_H

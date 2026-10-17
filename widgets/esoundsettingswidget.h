#ifndef ESOUNDSETTINGSWIDGET_H
#define ESOUNDSETTINGSWIDGET_H

#include "eframedwidget.h"
#include "esettings.h"

class eSoundSettingsWidget : public eFramedWidget {
public:
    using eFramedWidget::eFramedWidget;
    using eApplyAction = std::function<void(const eSettings&)>;

    void initialize(const eSettings& settings,
                    const eApplyAction& applyAction);
private:
    eSettings mSettings;
};

#endif // ESOUNDSETTINGSWIDGET_H

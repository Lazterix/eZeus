#ifndef EEVENTWIDGET_H
#define EEVENTWIDGET_H

#include "ewidget.h"
#include "ebutton.h"
#include "esidebarnotificationstate.h"

class eGameBoard;
class eTile;
enum class eEvent;
struct eEventData;

class eEventButton : public eButton {
public:
    eEventButton(const eEvent e,
                 eMainWindow* const window);
};

class eEventWidget : public eWidget {
public:
    using eWidget::eWidget;

    void pushEvent(const eEvent e, const eEventData& ed);
    void incUiFrame();
    void clear();
    bool empty() const { return mAlertState.empty(); }
    std::size_t size() const { return mAlertState.size(); }

    using eEmptyChangedAction = std::function<void(bool)>;
    void setEmptyChangedAction(const eEmptyChangedAction& action) {
        mEmptyChangedAction = action;
    }

    using eViewTileHandler = std::function<void(eTile*)>;
    void setViewTileHandler(const eViewTileHandler& h);
private:
    struct eButtonState {
        eEventButton* fButton;
        int fAlertId;
    };

    void removeAlert(const int id, const bool dismiss);

    eViewTileHandler mViewTileHandler;
    eEmptyChangedAction mEmptyChangedAction;
    int mFrame = 0;
    eSidebarAlertState mAlertState;
    std::vector<eButtonState> mButtons;
};

#endif // EEVENTWIDGET_H

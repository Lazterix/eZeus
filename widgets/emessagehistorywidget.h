#ifndef EMESSAGEHISTORYWIDGET_H
#define EMESSAGEHISTORYWIDGET_H

#include <cstddef>
#include <functional>

#include "eframedwidget.h"

class eMessageHistory;

class eMessageHistoryWidget : public eFramedWidget {
public:
    using eFramedWidget::eFramedWidget;

    using eOpenMessageAction = std::function<void(std::size_t)>;
    void initialize(const eMessageHistory& history,
                    const eOpenMessageAction& openMessage,
                    const eAction& closeAction);
};

#endif // EMESSAGEHISTORYWIDGET_H

#ifndef EFRAMEDWIDGET_H
#define EFRAMEDWIDGET_H

#include "ewidget.h"

enum class eFrameType {
    outer, message, inner
};

class eFramedWidget : public eWidget {
public:
    using eWidget::eWidget;

    void setType(const eFrameType type);
    void setOuterFrameId(const int id) { mOuterFrameId = id; }
protected:
    void paintEvent(ePainter& p);
private:
    eFrameType mType = eFrameType::outer;
    int mOuterFrameId = 1;
};

#endif // EFRAMEDWIDGET_H

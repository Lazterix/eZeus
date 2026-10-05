#ifndef ETOPBARWIDGET_H
#define ETOPBARWIDGET_H

#include <algorithm>

#include "eframedwidget.h"
#include "elabel.h"

class eGameBoard;
class eGameWidget;
class eButton;
class eLabel;

class eTopBarClippedLabel : public eLabel {
public:
    using eLabel::eLabel;
protected:
    void paintEvent(ePainter& p) override;
};

class eTopWidget : public eWidget {
public:
    using eWidget::eWidget;

    void initialize(const std::shared_ptr<eTexture>& icon,
                    const std::string& text) {
        setPadding(0);
        mIcon = new eLabel(window());
        mIcon->setPadding(0);
        mIcon->setTexture(icon);
        mIcon->fitContent();
        mText = new eTopBarClippedLabel(window());
        mText->setX(1.5*mIcon->width());
        mText->setPadding(0);
        mText->setSmallFontSize();

        addWidget(mIcon);
        addWidget(mText);

        setText(text);

        mIcon->align(eAlignment::vcenter);
        mText->align(eAlignment::vcenter);
    }

    void setText(const std::string& text) {
        mIcon->show();
        mText->setX(1.5*mIcon->width());
        mText->setText(text);
        mText->fitContent();
        fitContent();
    }

    int preferredWidth() const {
        return mText->x() + mText->width();
    }

    void constrainWidth(const int width) {
        setWidth(width);
        if(width < mIcon->width()) {
            mIcon->hide();
            mText->setX(0);
        } else {
            mIcon->show();
            mText->setX(1.5*mIcon->width());
        }
        mText->setWidth(std::max(0, width - mText->x()));
    }
private:
    eLabel* mIcon = nullptr;
    eLabel* mText = nullptr;
};

class eTopBarWidget : public eWidget {
public:
    using eWidget::eWidget;

    void initialize();
    void setBoard(eGameBoard* const board);
    void setGameWidget(eGameWidget* const gw);
    bool triggerMenuClick(const eWidget* const from,
                          const int pressX, const int pressY,
                          const int releaseX, const int releaseY);

    void paintEvent(ePainter& p);
private:
    void layoutContents();

    eGameBoard* mBoard = nullptr;
    eGameWidget* mGW = nullptr;
    eButton* mFileButton = nullptr;
    eButton* mOptionsButton = nullptr;
    eButton* mHelpButton = nullptr;
    eLabel* mCityLabel = nullptr;
    eTopWidget* mDrachmasWidget = nullptr;
    eTopWidget* mPopulationWidget = nullptr;
    eButton* mDateLabel = nullptr;
    int mTime = 0;
};

#endif // ETOPBARWIDGET_H

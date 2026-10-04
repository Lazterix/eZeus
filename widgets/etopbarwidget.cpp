#include "etopbarwidget.h"

#include "engine/egameboard.h"
#include "engine/boardData/epopulationdata.h"
#include "textures/egametextures.h"
#include "ebutton.h"
#include "edatewidget.h"
#include "egamewidget.h"
#include "etopmenudropdown.h"

#include "emainwindow.h"

#include <algorithm>

namespace {
class eClippedButton : public eButton {
public:
    using eButton::eButton;
protected:
    void paintEvent(ePainter& p) override {
        const auto clipRect = rect();
        p.setClipRect(&clipRect);
        eButton::paintEvent(p);
        p.setClipRect(nullptr);
    }
};
}

void eTopBarClippedLabel::paintEvent(ePainter& p) {
    const auto clipRect = rect();
    p.setClipRect(&clipRect);
    eLabel::paintEvent(p);
    p.setClipRect(nullptr);
}

void eTopBarWidget::initialize() {
    const auto& intrfc = eGameTextures::interface();
    const auto uiScale = resolution().uiScale();
    const int icoll = static_cast<int>(uiScale);
    const int mult = icoll + 1;
    const auto& coll = intrfc[icoll];
    setPadding(0);

    const auto createMenuButton = [this, mult](const std::string& text,
                                               const eTopMenuId id) {
        const auto button = new eButton(text, window());
        button->setNoPadding();
        button->setSmallFontSize();
        button->fitContent();
        button->setWidth(button->width() + 4*mult);
        button->setPressAction([this, button, id]() {
            if(!mGW) return;

            std::vector<eTopMenuAction> actions;
            switch(id) {
            case eTopMenuId::file:
                actions = {{"Save", true, nullptr},
                           {"Load", true, nullptr},
                           {"Leave Greece", true, nullptr}};
                break;
            case eTopMenuId::options:
                actions = {{"Display", true, nullptr},
                           {"Speed", true, nullptr},
                           {"Sound", true, nullptr}};
                break;
            case eTopMenuId::help:
                actions = {{"Help", true, nullptr}};
                break;
            }
            mGW->toggleTopMenu(id, button, actions);
        });
        addWidget(button);
        return button;
    };

    // Stage A placeholders prove menu geometry and input only.
    mFileButton = createMenuButton("FILE", eTopMenuId::file);
    mOptionsButton = createMenuButton("OPTIONS", eTopMenuId::options);
    mHelpButton = createMenuButton("HELP", eTopMenuId::help);

    mMenuSeparator = new eLabel("|", window());
    mMenuSeparator->setNoPadding();
    mMenuSeparator->setSmallFontSize();
    mMenuSeparator->fitContent();
    addWidget(mMenuSeparator);

    mDrachmasWidget = new eTopWidget(window());
    mDrachmasWidget->initialize(coll.fDrachmasTopMenu, "-");

    mCityLabel = new eTopBarClippedLabel("-", window());
    mCityLabel->setSmallFontSize();
    mCityLabel->setNoPadding();
    mCityLabel->fitContent();

    mPopulationWidget = new eTopWidget(window());
    mPopulationWidget->initialize(coll.fPopulationTopMenu, "-");

    mDateLabel = new eClippedButton(window());
    mDateLabel->setPressAction([this]() {
        if(!mBoard) return;
        const auto dw = new eDateWidget(window());
        dw->initialize([this](const eDate& d) {
            mBoard->setDate(d);
            mDateLabel->setText(d.shortString());
        }, false);
        dw->setDate(mBoard->date());
        window()->execDialog(dw);
        dw->align(eAlignment::center);
    });
    const eDate date(30, eMonth::january, -1500);
    mDateLabel->setSmallFontSize();
    mDateLabel->setText(date.shortString());
    mDateLabel->fitContent();
    mDateLabel->setEnabled(false);

    addWidget(mCityLabel);
    addWidget(mDrachmasWidget);
    addWidget(mPopulationWidget);
    addWidget(mDateLabel);

    setHeight(12*mult);

    layoutContents();
}

void eTopBarWidget::layoutContents() {
    int iRes;
    int mult;
    iResAndMult(iRes, mult);

    const int menuGap = 2*mult;
    const int statusGap = 4*mult;
    int x = 2*mult;
    for(const auto button : {mFileButton, mOptionsButton, mHelpButton}) {
        button->move(x, (height() - button->height())/2);
        x += button->width() + menuGap;
    }
    mMenuSeparator->move(x, (height() - mMenuSeparator->height())/2);
    x += mMenuSeparator->width() + statusGap;

    const int statusLeft = std::min(x, width());
    const int statusRight = std::max(statusLeft, width() - 2*mult);
    const int availableWidth = statusRight - statusLeft;
    const int gap = std::min(statusGap, availableWidth/3);
    const int contentWidth = std::max(0, availableWidth - 3*gap);

    const int drachmasPreferred = mDrachmasWidget->preferredWidth();
    const int populationPreferred = mPopulationWidget->preferredWidth();
    mDateLabel->fitContent();
    const int datePreferred = mDateLabel->width();
    const int fixedPreferred = drachmasPreferred +
                               populationPreferred +
                               datePreferred;

    int drachmasWidth = drachmasPreferred;
    int populationWidth = populationPreferred;
    int dateWidth = datePreferred;
    int cityWidth = contentWidth - fixedPreferred;
    if(cityWidth < 0) {
        cityWidth = 0;
        if(fixedPreferred > 0) {
            drachmasWidth = contentWidth*drachmasPreferred/fixedPreferred;
            populationWidth = contentWidth*populationPreferred/fixedPreferred;
            dateWidth = contentWidth - drachmasWidth - populationWidth;
        } else {
            drachmasWidth = 0;
            populationWidth = 0;
            dateWidth = 0;
        }
    }

    mCityLabel->move(statusLeft, (height() - mCityLabel->height())/2);
    mCityLabel->setWidth(cityWidth);
    x = statusLeft + cityWidth + gap;

    mDrachmasWidget->move(x, (height() - mDrachmasWidget->height())/2);
    mDrachmasWidget->constrainWidth(drachmasWidth);
    x += drachmasWidth + gap;

    mPopulationWidget->move(x, (height() - mPopulationWidget->height())/2);
    mPopulationWidget->constrainWidth(populationWidth);
    x += populationWidth + gap;

    mDateLabel->move(x, (height() - mDateLabel->height())/2);
    mDateLabel->setWidth(dateWidth);
}

void eTopBarWidget::setBoard(eGameBoard* const board) {
    mBoard = board;
}

void eTopBarWidget::setGameWidget(eGameWidget* const gw) {
    mGW = gw;
}

bool eTopBarWidget::triggerMenuClick(
        const eWidget* const from,
        const int pressX, const int pressY,
        const int releaseX, const int releaseY) {
    for(const auto button : {mFileButton, mOptionsButton, mHelpButton}) {
        int buttonPressX = pressX;
        int buttonPressY = pressY;
        button->mapFrom(from, buttonPressX, buttonPressY);
        if(!button->contains(buttonPressX, buttonPressY)) continue;

        int buttonReleaseX = releaseX;
        int buttonReleaseY = releaseY;
        button->mapFrom(from, buttonReleaseX, buttonReleaseY);
        if(!button->contains(buttonReleaseX, buttonReleaseY)) continue;

        button->trigger();
        return true;
    }
    return false;
}

void eTopBarWidget::paintEvent(ePainter& p) {
    // const bool update = (++mTime % 60) == 0;
    if(mBoard) {
        const auto cid = mGW->viewedCity();
        const auto pid = mBoard->personPlayer();
//        const auto pid = mBoard->cityIdToPlayerId(cid);
        const auto& wb = mBoard->world();
        const auto c = wb.cityWithId(cid);

        const auto label = c ? c->name() : "-";
        mCityLabel->setText(label);
        mCityLabel->fitContent();

        const auto popData = mBoard->populationData(cid);
        if(popData) {
            const int pop = popData->population();
            mPopulationWidget->setText(std::to_string(pop));
        } else {
            mPopulationWidget->setText("-");
        }

        const int d = mBoard->drachmas(pid);
        mDrachmasWidget->setText(std::to_string(d));

        mDateLabel->setText(mBoard->date().shortString());
        mDateLabel->setEnabled(mBoard->editorMode());

        layoutContents();

        int iRes;
        int mult;
        iResAndMult(iRes, mult);
        const auto& intrfc = eGameTextures::interface()[iRes];
        const auto& tex = intrfc.fGameTopBar;
        const int texWidth = tex->width();
        const auto& rend = p.renderer();
        bool flip = false;
        for(int x = width() - texWidth; x > -texWidth; x -= texWidth) {
            tex->render(rend, x, 0, flip);
            flip = !flip;
        }
    } else {
        mDateLabel->setEnabled(false);
    }
}

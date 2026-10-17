#include "emessagehistorywidget.h"

#include "ebutton.h"
#include "elabel.h"
#include "elanguage.h"
#include "eokbutton.h"
#include "escrollwidgetcomplete.h"
#include "estringhelpers.h"
#include "characters/gods/egod.h"
#include "characters/monsters/emonster.h"
#include "engine/eworldcity.h"
#include "widgets/emessagehistory.h"

std::string eMessageSubject(const eSavedMessage& saved) {
    auto subject = saved.fMsg.fTitle;
    const auto& ed = saved.fEd;
    if(ed.fCity) {
        eStringHelpers::replaceAll(subject, "[city_name]", ed.fCity->name());
    }
    if(ed.fRivalCity) {
        eStringHelpers::replaceAll(subject, "[rival_city_name]",
                                   ed.fRivalCity->name());
    }
    eStringHelpers::replaceAll(subject, "[itemshort]",
                               eResourceTypeHelpers::typeName(ed.fResourceType));
    eStringHelpers::replaceAll(subject, "[god]", eGod::sGodName(ed.fGod));
    eStringHelpers::replaceAll(subject, "[monster]",
                               eMonster::sMonsterName(ed.fMonster));
    return subject;
}

void eMessageHistoryWidget::initialize(
        const eMessageHistory& history,
        const eOpenMessageAction& openMessage,
        const eAction& closeAction) {
    setType(eFrameType::message);

    const int p = padding();
    const auto res = resolution();
    const int dialogWidth = res.centralWidgetSmallWidth();
    const int dialogHeight = res.centralWidgetSmallHeight();
    resize(dialogWidth, dialogHeight);

    const auto title = new eLabel(eLanguage::zeusText(63, 0), window());
    title->setHugeFontSize();
    title->fitContent();
    addWidget(title);
    title->align(eAlignment::top | eAlignment::hcenter);
    title->setY(p);

    const int contentTop = title->y() + title->height() + p;
    const int contentHeight = dialogHeight - contentTop - 6*p;

    if(history.empty()) {
        const auto empty = new eLabel(window());
        empty->setSmallFontSize();
        empty->setWrapWidth(dialogWidth - 6*p);
        empty->setText(eLanguage::zeusText(63, 1));
        empty->fitContent();
        addWidget(empty);
        empty->align(eAlignment::center);
    } else {
        const auto headings = new eWidget(window());
        headings->setNoPadding();
        headings->setWidth(dialogWidth - 4*p);
        const auto date = new eLabel(eLanguage::zeusText(63, 2), window());
        date->setSmallFontSize();
        date->fitContent();
        headings->addWidget(date);
        const auto subject = new eLabel(eLanguage::zeusText(63, 3), window());
        subject->setSmallFontSize();
        subject->fitContent();
        subject->setX(headings->width()/3);
        headings->addWidget(subject);
        headings->fitHeight();
        headings->move(2*p, contentTop);
        addWidget(headings);

        const auto scroll = new eScrollWidgetComplete(window());
        scroll->setNoPadding();
        scroll->resize(dialogWidth - 4*p,
                       contentHeight - headings->height() - p);
        scroll->move(2*p, headings->y() + headings->height() + p);
        scroll->initialize();
        addWidget(scroll);

        const auto rows = new eWidget(window());
        rows->setNoPadding();
        rows->setWidth(scroll->areaWidth());
        int y = 0;
        for(std::size_t i = 0; i < history.size(); ++i) {
            const auto saved = history.newest(i);
            if(!saved) continue;
            const auto text = saved->fEd.fDate.shortString() + "     " +
                              eMessageSubject(*saved);
            const auto row = new eButton(text, window());
            row->setNoPadding();
            row->setSmallFontSize();
            row->setUnderline(false);
            row->setTextAlignment(eAlignment::left | eAlignment::vcenter);
            row->fitContent();
            row->setWidth(rows->width());
            row->setY(y);
            row->setPressAction([openMessage, i]() {
                if(openMessage) openMessage(i);
            });
            rows->addWidget(row);
            y += row->height();
        }
        rows->setHeight(y);
        scroll->setScrollArea(rows);
    }

    const auto ok = new eOkButton(window());
    ok->setPressAction(closeAction);
    addWidget(ok);
    ok->align(eAlignment::bottom | eAlignment::right);
    ok->move(ok->x() - 2*p, ok->y() - 2*p);
}

#include "esoundsettingswidget.h"

#include "ecancelbutton.h"
#include "eframedbutton.h"
#include "elabel.h"
#include "eokbutton.h"
#include "espinbox.h"

#include "elanguage.h"

#include <algorithm>

namespace {
eWidget* volumeRow(eMainWindow* const window,
                   const std::string& name,
                   const int value,
                   const std::function<void(int)>& changed) {
    const auto row = new eWidget(window);
    row->setNoPadding();

    const auto label = new eLabel(name, window);
    label->setSmallFontSize();
    label->fitContent();
    row->addWidget(label);

    const auto spin = new eSpinBox(window);
    spin->initialize([spin, changed]() { changed(spin->value()); });
    spin->setRange(0, 128);
    spin->setIncrement(8);
    spin->setValue(value);
    // eSpinBox::initialize() centers its children before the spin box has
    // a size. Their negative Y coordinates previously made the volume
    // controls overlap the buttons above them. Measure from Y=0 first.
    spin->setNoPadding();
    for(const auto child : spin->children()) child->setY(0);
    spin->fitContent();
    for(const auto child : spin->children()) {
        child->align(eAlignment::vcenter);
    }
    row->addWidget(spin);

    row->stackHorizontally(label->padding());
    row->fitContent();
    label->align(eAlignment::vcenter);
    return row;
}
}

void eSoundSettingsWidget::initialize(
        const eSettings& settings,
        const eApplyAction& applyAction) {
    mSettings = settings;
    setType(eFrameType::message);

    const int p = padding();
    const auto content = new eWidget(window());
    content->setNoPadding();

    const auto title = new eLabel(eLanguage::zeusText(46, 0), window());
    title->setHugeFontSize();
    title->fitContent();
    content->addWidget(title);

    const auto music = new eFramedButton(window());
    music->setUnderline(false);
    music->setText(eLanguage::zeusText(
        46, mSettings.fMusicEnabled ? 2 : 1));
    music->fitContent();
    music->setPressAction([this, music]() {
        mSettings.fMusicEnabled = !mSettings.fMusicEnabled;
        music->setText(eLanguage::zeusText(
            46, mSettings.fMusicEnabled ? 2 : 1));
        music->fitContent();
        music->align(eAlignment::hcenter);
    });
    content->addWidget(music);

    const auto musicVolume = volumeRow(
        window(), eLanguage::zeusText(46, 11), mSettings.fMusicVolume,
        [this](const int value) { mSettings.fMusicVolume = value; });
    content->addWidget(musicVolume);

    const auto sounds = new eFramedButton(window());
    sounds->setUnderline(false);
    sounds->setText(eLanguage::zeusText(
        46, mSettings.fSoundsEnabled ? 6 : 5));
    sounds->fitContent();
    sounds->setPressAction([this, sounds]() {
        mSettings.fSoundsEnabled = !mSettings.fSoundsEnabled;
        sounds->setText(eLanguage::zeusText(
            46, mSettings.fSoundsEnabled ? 6 : 5));
        sounds->fitContent();
        sounds->align(eAlignment::hcenter);
    });
    content->addWidget(sounds);

    const auto soundsVolume = volumeRow(
        window(), eLanguage::zeusText(46, 11), mSettings.fSoundsVolume,
        [this](const int value) { mSettings.fSoundsVolume = value; });
    content->addWidget(soundsVolume);

    content->stackVertically(std::max(2, p/2));
    content->fitContent();
    musicVolume->align(eAlignment::hcenter);
    soundsVolume->align(eAlignment::hcenter);
    addWidget(content);
    content->move(2*p, 2*p);

    const auto cancel = new eCancelButton(window());
    cancel->setPressAction([this]() { deleteLater(); });
    addWidget(cancel);

    const auto ok = new eOkButton(window());
    ok->setPressAction([this, applyAction]() {
        applyAction(mSettings);
        deleteLater();
    });
    addWidget(ok);

    const int buttonsHeight = std::max(cancel->height(), ok->height());
    resize(content->width() + 4*p,
           content->height() + buttonsHeight + 6*p);
    title->align(eAlignment::hcenter);
    music->align(eAlignment::hcenter);
    sounds->align(eAlignment::hcenter);
    cancel->align(eAlignment::bottom | eAlignment::left);
    cancel->move(cancel->x() + 2*p, cancel->y() - 2*p);
    ok->align(eAlignment::bottom | eAlignment::right);
    ok->move(ok->x() - 2*p, ok->y() - 2*p);
}

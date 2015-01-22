/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Rara

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Rara
    *********************************************************************/

#include "configdialog.h"
#include "ui_configdialog.h"
#include "settings.h"
#include "stylehelper.h"
#include "audio.h"
#include "roomscene.h"

#include <QFileDialog>
#include <QDesktopServices>
#include <QFontDialog>
#include <QColorDialog>
#include <QTextStream>
#include <QLineEdit>

ConfigDialog::ConfigDialog(QWidget *parent)
    : FlatDialog(parent, false), ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);
    
    connect(this, &ConfigDialog::windowTitleChanged, ui->windowTitle, &QLabel::setText);

    // tab 1
    ui->bgPathLineEdit->setText(Config.BackgroundImage);

    ui->tableBgPathLineEdit->setText(Config.TableBgImage);

    QFont font = Config.AppFont;
    showFont(ui->appFontLineEdit, font);

    font = Config.UIFont;
    showFont(ui->textEditFontLineEdit, font);

    QPalette palette;
    palette.setColor(QPalette::Text, Config.TextEditColor);
    QColor color = Config.TextEditColor;
    int aver = (color.red() + color.green() + color.blue()) / 3;
    palette.setColor(QPalette::Base, aver >= 208 ? Qt::black : Qt::white);
    ui->textEditFontLineEdit->setPalette(palette);

    // tab 2
    ui->bgMusicPathLineEdit->setText(Config.value("BackgroundMusic", "audio/system/background.ogg").toString());

    ui->enableEffectCheckBox->setChecked(Config.EnableEffects);
    connect(ui->enableEffectCheckBox, &QCheckBox::toggled, this,
            &ConfigDialog::onEffectsEnabledChanged);

    ui->enableLastWordCheckBox->setChecked(Config.EnableLastWord);
    connect(ui->enableLastWordCheckBox, &QCheckBox::toggled, this,
            &ConfigDialog::onLastWordEnabledChanged);

    ui->enableBgMusicCheckBox->setChecked(Config.EnableBgMusic);
    connect(ui->enableBgMusicCheckBox, &QCheckBox::toggled, this,
            &ConfigDialog::setBGMEnabled);

    ui->noIndicatorCheckBox->setChecked(Config.value("NoIndicator", false).toBool());
    ui->noEquipAnimCheckBox->setChecked(Config.value("NoEquipAnim", false).toBool());

    ui->bgmVolumeSlider->setValue(100 * Config.BGMVolume);
    connect(ui->bgmVolumeSlider, &QSlider::valueChanged, this,
            &ConfigDialog::onBGMVolumeChanged);

    ui->effectVolumeSlider->setValue(100 * Config.EffectVolume);
    connect(ui->effectVolumeSlider, &QSlider::valueChanged, this,
            &ConfigDialog::onEffectVolumeChanged);

    // tab 3
    ui->neverNullifyMyTrickCheckBox->setChecked(Config.NeverNullifyMyTrick);
    ui->autoTargetCheckBox->setChecked(Config.EnableAutoTarget);
    ui->intellectualSelectionCheckBox->setChecked(Config.EnableIntellectualSelection);
    ui->superDragCheckBox->setChecked(Config.EnableSuperDrag);
    ui->doubleClickCheckBox->setChecked(Config.EnableDoubleClick);
    ui->autoPreshowCheckBox->setChecked(Config.EnableAutoPreshow);
    ui->bubbleChatBoxKeepSpinBox->setValue(Config.BubbleChatBoxKeepSeconds);
    ui->ignoreChangingSkinCheckBox->setChecked(Config.IgnoreOthersSwitchesOfSkin);

    ui->enableAutoSaveCheckBox->setChecked(Config.EnableAutoSaveRecord);
    ui->networkOnlyCheckBox->setChecked(Config.NetworkOnly);

    ui->networkOnlyCheckBox->setEnabled(ui->enableAutoSaveCheckBox->isChecked());
    ui->recordPathSetupLabel->setEnabled(ui->enableAutoSaveCheckBox->isChecked());
    ui->recordPathSetupLineEdit->setEnabled(ui->enableAutoSaveCheckBox->isChecked());
    ui->browseRecordPathButton->setEnabled(ui->enableAutoSaveCheckBox->isChecked());
    ui->resetRecordPathButton->setEnabled(ui->enableAutoSaveCheckBox->isChecked());

    connect(ui->enableAutoSaveCheckBox, &QCheckBox::toggled, ui->networkOnlyCheckBox, &QCheckBox::setEnabled);
    connect(ui->enableAutoSaveCheckBox, &QCheckBox::toggled, ui->recordPathSetupLabel, &QLabel::setEnabled);
    connect(ui->enableAutoSaveCheckBox, &QCheckBox::toggled, ui->recordPathSetupLineEdit, &QLineEdit::setEnabled);
    connect(ui->enableAutoSaveCheckBox, &QCheckBox::toggled, ui->browseRecordPathButton, &QPushButton::setEnabled);
    connect(ui->enableAutoSaveCheckBox, &QCheckBox::toggled, ui->resetRecordPathButton, &QPushButton::setEnabled);

    ui->recordPathSetupLineEdit->setText(Config.RecordSavePath);

    connect(this, &ConfigDialog::accepted, this, &ConfigDialog::saveConfig);
    connect(this, &ConfigDialog::rejected, this, &ConfigDialog::discardSettings);
}

void ConfigDialog::showFont(QLineEdit *lineedit, const QFont &font) {
    lineedit->setFont(font);
    lineedit->setText(QString("%1 %2").arg(font.family()).arg(font.pointSize()));
}

void ConfigDialog::doCallback(ConfigDialog::Callback callback, const QVariant &oldValue, const QVariant &newValue)
{
    if (!resetCallbacks.contains(callback)) {
        resetCallbacks << callback;
        callbackArgs << oldValue;
    }
    (this->*callback)(newValue);
}

void ConfigDialog::setBackground(const QVariant &path)
{
    QString fileName = path.toString();
    ui->bgPathLineEdit->setText(fileName);
    Config.BackgroundImage = fileName;

    emit bg_changed();
}

void ConfigDialog::setTableBg(const QVariant &path)
{
    QString fileName = path.toString();
    ui->tableBgPathLineEdit->setText(fileName);
    Config.TableBgImage = fileName;

    emit tableBg_changed();
}

void ConfigDialog::setTooltipBackgroundColor(const QVariant &color)
{
    Config.ToolTipBackgroundColor = color.toString();

    QFile file("style-sheet/sanguosha.qss");
    static QString styleSheet;
    if (styleSheet.isEmpty()) {
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream stream(&file);
            styleSheet = stream.readAll();
        }

#ifdef Q_OS_WIN
        QFile winFile("style-sheet/windows-extra.qss");
        if (winFile.open(QIODevice::ReadOnly)) {
            QTextStream winStream(&winFile);
            styleSheet += winStream.readAll();
        }
#endif
    }

    qApp->setStyleSheet(styleSheet + StyleHelper::styleSheetOfTooltip());
}

void ConfigDialog::setAppFont(const QVariant &font)
{
    QFont newFont = font.value<QFont>();
    Config.AppFont = newFont;
    showFont(ui->appFontLineEdit, newFont);

    QApplication::setFont(newFont);
}

void ConfigDialog::setTextEditFont(const QVariant &font)
{
    QFont newFont = font.value<QFont>();
    Config.UIFont = newFont;
    showFont(ui->textEditFontLineEdit, newFont);

    QApplication::setFont(newFont, "QTextEdit");
}

void ConfigDialog::setTextEditColor(const QVariant &color)
{
    QColor newColor = color.toString();
    Config.TextEditColor = newColor;
    QPalette palette;
    palette.setColor(QPalette::Text, newColor);
    int aver = (newColor.red() + newColor.green() + newColor.blue()) / 3;
    palette.setColor(QPalette::Base, aver >= 208 ? Qt::black : Qt::white);
    ui->textEditFontLineEdit->setPalette(palette);
}

void ConfigDialog::setTooltipFontColor(const QVariant &color)
{
    Config.SkillDescriptionInToolTipColor = color.toString();
}

void ConfigDialog::setOverviewFontColor(const QVariant &color)
{
    Config.SkillDescriptionInOverviewColor = color.toString();
}

void ConfigDialog::setBgMusic(const QVariant &path)
{
    QString fileName = path.toString();
    ui->bgMusicPathLineEdit->setText(fileName);
    Config.setValue("BackgroundMusic", fileName);

#ifdef AUDIO_SUPPORT
    Audio::stopBGM();
    Audio::playBGM(fileName);
#endif // AUDIO_SUPPORT
}

void ConfigDialog::setEffectsEnabled(const QVariant &enabled)
{
    Config.EnableEffects = enabled.toBool();
    ui->enableEffectCheckBox->setChecked(Config.EnableEffects);
}

void ConfigDialog::setLastWordEnabled(const QVariant &enabled)
{
    Config.EnableLastWord = enabled.toBool();
    ui->enableLastWordCheckBox->setChecked(Config.EnableLastWord);
}

void ConfigDialog::setBGMEnabled(const QVariant &enabled)
{
#ifdef AUDIO_SUPPORT
    if (RoomSceneInstance != NULL) {
        bool play = enabled.toBool();
        if (play) {
            Audio::playBGM(Config.value("BackgroundMusic",
                                        "audio/system/background.ogg").toString());
        } else {
            Audio::stopBGM();
        }
    }
#endif // AUDIO_SUPPORT

    Config.EnableBgMusic = enabled.toBool();
    ui->enableBgMusicCheckBox->setChecked(Config.EnableBgMusic);
}

void ConfigDialog::setBGMVolume(const QVariant &volume)
{
    float vol = volume.toInt() / 100.0;
#ifdef AUDIO_SUPPORT
    Audio::setBGMVolume(vol);
#endif // AUDIO_SUPPORT

    Config.BGMVolume = vol;
    ui->bgmVolumeSlider->setValue(volume.toInt());
}

void ConfigDialog::setEffectVolume(const QVariant &volume)
{
    float vol = volume.toInt() / 100.0;
    Config.EffectVolume = vol;
    ui->effectVolumeSlider->setValue(volume.toInt());
}

void ConfigDialog::setRecordsSavePath(const QVariant &path)
{
    QString dir = path.toString();

    ui->recordPathSetupLineEdit->setText(dir);
    Config.RecordSavePath = dir;
}

ConfigDialog::~ConfigDialog() {
    delete ui;
}

void ConfigDialog::on_browseBgButton_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Select a background image"),
        "image/backdrop/",
        tr("Images (*.png *.bmp *.jpg)"));

    if (!fileName.isEmpty() && fileName != ui->bgPathLineEdit->text())
        doCallback(&ConfigDialog::setBackground, Config.BackgroundImage, fileName);
}

void ConfigDialog::on_resetBgButton_clicked() {
    QString fileName = "image/backdrop/bg.jpg";
    if (fileName != ui->bgPathLineEdit->text())
        doCallback(&ConfigDialog::setBackground, Config.BackgroundImage, fileName);
}

void ConfigDialog::on_browseTableBgButton_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Select a tableBg image"),
        "image/backdrop/",
        tr("Images (*.png *.bmp *.jpg)"));

    if (!fileName.isEmpty() && fileName != ui->tableBgPathLineEdit->text())
        doCallback(&ConfigDialog::setTableBg, Config.TableBgImage, fileName);
}

void ConfigDialog::on_resetTableBgButton_clicked() {
    QString fileName = "image/backdrop/table.jpg";
    if (fileName != ui->tableBgPathLineEdit->text())
        doCallback(&ConfigDialog::setTableBg, Config.TableBgImage, fileName);
}

void ConfigDialog::on_browseRecordPathButton_clicked() {
    QString path = QFileDialog::getExistingDirectory(this,
        tr("Select a Record Paths"),
        "records/");

    if (!path.isEmpty() && ui->recordPathSetupLineEdit->text() != path)
        doCallback(&ConfigDialog::setRecordsSavePath, Config.RecordSavePath, path);
}

void ConfigDialog::on_resetRecordPathButton_clicked() {
    ui->recordPathSetupLineEdit->clear();

    QString path = "records/";
    if (ui->recordPathSetupLineEdit->text() != path)
        doCallback(&ConfigDialog::setRecordsSavePath, Config.RecordSavePath, path);
}

void ConfigDialog::saveConfig() {
    Config.setValue("BGMVolume", Config.BGMVolume);

    Config.setValue("EffectVolume", Config.EffectVolume);

    Config.setValue("EnableEffects", ui->enableEffectCheckBox->isChecked());

    Config.setValue("EnableLastWord", ui->enableLastWordCheckBox->isChecked());

    Config.setValue("EnableBgMusic", ui->enableBgMusicCheckBox->isChecked());

    Config.setValue("NoIndicator", ui->noIndicatorCheckBox->isChecked());
    Config.setValue("NoEquipAnim", ui->noEquipAnimCheckBox->isChecked());

    Config.NeverNullifyMyTrick = ui->neverNullifyMyTrickCheckBox->isChecked();
    Config.setValue("NeverNullifyMyTrick", Config.NeverNullifyMyTrick);

    Config.EnableAutoTarget = ui->autoTargetCheckBox->isChecked();
    Config.setValue("EnableAutoTarget", Config.EnableAutoTarget);

    Config.EnableIntellectualSelection = ui->intellectualSelectionCheckBox->isChecked();
    Config.setValue("EnableIntellectualSelection", Config.EnableIntellectualSelection);

    Config.EnableSuperDrag = ui->superDragCheckBox->isChecked();
    Config.setValue("EnableSuperDrag", Config.EnableSuperDrag);

    Config.EnableDoubleClick = ui->doubleClickCheckBox->isChecked();
    Config.setValue("EnableDoubleClick", Config.EnableDoubleClick);

    Config.EnableAutoPreshow = ui->autoPreshowCheckBox->isChecked();
    Config.setValue("EnableAutoPreshowInConsoleMode",
                    Config.EnableAutoPreshow);

    Config.BubbleChatBoxKeepSeconds = ui->bubbleChatBoxKeepSpinBox->value();
    Config.setValue("BubbleChatBoxKeepSeconds", Config.BubbleChatBoxKeepSeconds);

    Config.IgnoreOthersSwitchesOfSkin = ui->ignoreChangingSkinCheckBox->isChecked();
    Config.setValue("IgnoreOthersSwitchesOfSkin",
                    Config.IgnoreOthersSwitchesOfSkin);

    Config.EnableAutoSaveRecord = ui->enableAutoSaveCheckBox->isChecked();
    Config.setValue("EnableAutoSaveRecord", Config.EnableAutoSaveRecord);

    Config.NetworkOnly = ui->networkOnlyCheckBox->isChecked();
    Config.setValue("NetworkOnly", Config.NetworkOnly);

    Config.setValue("SkillDescriptionInOverviewColor",
                    Config.SkillDescriptionInOverviewColor);
    Config.setValue("SkillDescriptionInToolTipColor",
                    Config.SkillDescriptionInToolTipColor);

    Config.setValue("TextEditColor", Config.TextEditColor);

    Config.setValue("UIFont", Config.UIFont);
    Config.setValue("AppFont", Config.AppFont);

    Config.setValue("ToolTipBackgroundColor", Config.ToolTipBackgroundColor);

    Config.setValue("TableBgImage", Config.TableBgImage);
    Config.setValue("BackgroundImage", Config.BackgroundImage);

    Config.setValue("RecordSavePath", Config.RecordSavePath);

    resetCallbacks.clear();
    callbackArgs.clear();
}

void ConfigDialog::discardSettings()
{
    const int n = resetCallbacks.size();
    for (int i = 0; i < n; ++i)
        (this->*(resetCallbacks[i]))(callbackArgs.at(i));

    resetCallbacks.clear();
    callbackArgs.clear();
}

void ConfigDialog::on_browseBgMusicButton_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Select a background music"),
        "audio/system",
        tr("Audio files (*.wav *.mp3 *.ogg)"));
    if (!fileName.isEmpty() && fileName != ui->bgMusicPathLineEdit->text()) {
        doCallback(&ConfigDialog::setBgMusic, Config.value("BackgroundMusic"),
                   fileName);
    }
}

void ConfigDialog::on_resetBgMusicButton_clicked() {
    QString defaultMusic = "audio/system/background.ogg";
    if (defaultMusic != ui->bgMusicPathLineEdit->text()) {
        doCallback(&ConfigDialog::setBgMusic, Config.value("BackgroundMusic"),
                   defaultMusic);
    }
}

void ConfigDialog::on_changeAppFontButton_clicked() {
    bool ok;
    QFont font = QFontDialog::getFont(&ok, Config.AppFont, this);
    if (ok && font != Config.AppFont)
        doCallback(&ConfigDialog::setAppFont, Config.AppFont, font);
}

void ConfigDialog::on_setTextEditFontButton_clicked() {
    bool ok;
    QFont font = QFontDialog::getFont(&ok, Config.UIFont, this);
    if (ok && font != Config.UIFont)
        doCallback(&ConfigDialog::setTextEditFont, Config.UIFont, font);
}

void ConfigDialog::on_setTextEditColorButton_clicked() {
    QColor color = QColorDialog::getColor(Config.TextEditColor, this);
    if (color.isValid() && color != Config.TextEditColor)
        doCallback(&ConfigDialog::setTextEditColor, Config.TextEditColor, color);
}

void ConfigDialog::on_toolTipFontColorButton_clicked()
{
    QColor color = QColorDialog::getColor(Config.SkillDescriptionInToolTipColor, this);
    if (color.isValid() && color != Config.SkillDescriptionInToolTipColor) {
        doCallback(&ConfigDialog::setTooltipFontColor, Config.SkillDescriptionInToolTipColor, color);
    }
}

void ConfigDialog::on_overviewFontColorButton_clicked()
{
    QColor color = QColorDialog::getColor(QColor(Config.SkillDescriptionInOverviewColor), this);
    if (color.isValid() && color != Config.SkillDescriptionInOverviewColor) {
        doCallback(&ConfigDialog::setOverviewFontColor,
                   Config.SkillDescriptionInOverviewColor, color);
    }
}

void ConfigDialog::on_toolTipBackgroundColorButton_clicked()
{
    QColor color = QColorDialog::getColor(QColor(Config.ToolTipBackgroundColor), this);
    if (color.isValid() && color != Config.ToolTipBackgroundColor) {
        doCallback(&ConfigDialog::setTooltipBackgroundColor,
                   Config.ToolTipBackgroundColor, color);
    }
}

void ConfigDialog::onEffectsEnabledChanged(bool enabled)
{
    doCallback(&ConfigDialog::setEffectsEnabled, Config.EnableEffects, enabled);
}

void ConfigDialog::onLastWordEnabledChanged(bool enabled)
{
    doCallback(&ConfigDialog::setLastWordEnabled, Config.EnableLastWord, enabled);
}

void ConfigDialog::onBGMVolumeChanged(int volume)
{
    doCallback(&ConfigDialog::setBGMVolume, Config.BGMVolume, volume);
}

void ConfigDialog::onEffectVolumeChanged(int volume)
{
    doCallback(&ConfigDialog::setEffectVolume, Config.EffectVolume, volume);
}

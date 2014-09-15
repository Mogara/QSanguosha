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
#include "StyleHelper.h"

#include <QFileDialog>
#include <QDesktopServices>
#include <QFontDialog>
#include <QColorDialog>
#include <QTextStream>

ConfigDialog::ConfigDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);

    // tab 1
    QString bg_path = Config.value("BackgroundImage").toString();
    if (!bg_path.startsWith(":"))
        ui->bgPathLineEdit->setText(bg_path);

    QString tableBg_path = Config.value("TableBgImage").toString();
    if (!tableBg_path.startsWith(":"))
        ui->tableBgPathLineEdit->setText(tableBg_path);

    ui->bgMusicPathLineEdit->setText(Config.value("BackgroundMusic", "audio/system/background.ogg").toString());

    ui->enableEffectCheckBox->setChecked(Config.EnableEffects);

    ui->enableLastWordCheckBox->setEnabled(Config.EnableEffects);
    ui->enableLastWordCheckBox->setChecked(Config.EnableLastWord);
    connect(ui->enableEffectCheckBox, SIGNAL(toggled(bool)), ui->enableLastWordCheckBox, SLOT(setEnabled(bool)));

    ui->enableBgMusicCheckBox->setChecked(Config.EnableBgMusic);
    ui->noIndicatorCheckBox->setChecked(Config.value("NoIndicator", false).toBool());
    ui->noEquipAnimCheckBox->setChecked(Config.value("NoEquipAnim", false).toBool());

    ui->bgmVolumeSlider->setValue(100 * Config.BGMVolume);
    ui->effectVolumeSlider->setValue(100 * Config.EffectVolume);

    // tab 2
    ui->neverNullifyMyTrickCheckBox->setChecked(Config.NeverNullifyMyTrick);
    ui->autoTargetCheckBox->setChecked(Config.EnableAutoTarget);
    ui->intellectualSelectionCheckBox->setChecked(Config.EnableIntellectualSelection);
    ui->superDragCheckBox->setChecked(Config.EnableSuperDrag);
    ui->doubleClickCheckBox->setChecked(Config.EnableDoubleClick);
    ui->autoPreshowCheckBox->setChecked(Config.EnableAutoPreshowInConsoleMode);
    ui->bubbleChatBoxKeepSpinBox->setValue(Config.BubbleChatBoxKeepSeconds);
    ui->ignoreChangingSkinCheckBox->setChecked(Config.IgnoreOthersSwitchesOfSkin);

    ui->enableAutoSaveCheckBox->setChecked(Config.EnableAutoSaveRecord);
    ui->networkOnlyCheckBox->setChecked(Config.NetworkOnly);

    ui->networkOnlyCheckBox->setEnabled(ui->enableAutoSaveCheckBox->isChecked());
    ui->recordPathsSetupLabel->setEnabled(ui->enableAutoSaveCheckBox->isChecked());
    ui->recordPathsSetupLineEdit->setEnabled(ui->enableAutoSaveCheckBox->isChecked());
    ui->browseRecordPathsButton->setEnabled(ui->enableAutoSaveCheckBox->isChecked());
    ui->resetRecordPathsButton->setEnabled(ui->enableAutoSaveCheckBox->isChecked());

    connect(ui->enableAutoSaveCheckBox, SIGNAL(toggled(bool)), ui->networkOnlyCheckBox, SLOT(setEnabled(bool)));
    connect(ui->enableAutoSaveCheckBox, SIGNAL(toggled(bool)), ui->recordPathsSetupLabel, SLOT(setEnabled(bool)));
    connect(ui->enableAutoSaveCheckBox, SIGNAL(toggled(bool)), ui->recordPathsSetupLineEdit, SLOT(setEnabled(bool)));
    connect(ui->enableAutoSaveCheckBox, SIGNAL(toggled(bool)), ui->browseRecordPathsButton, SLOT(setEnabled(bool)));
    connect(ui->enableAutoSaveCheckBox, SIGNAL(toggled(bool)), ui->resetRecordPathsButton, SLOT(setEnabled(bool)));

    QString record_path = Config.value("RecordSavePaths", "records/").toString();
    if (!record_path.startsWith(":"))
        ui->recordPathsSetupLineEdit->setText(record_path);

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

    connect(this, SIGNAL(accepted()), this, SLOT(saveConfig()));
}

void ConfigDialog::showFont(QLineEdit *lineedit, const QFont &font) {
    lineedit->setFont(font);
    lineedit->setText(QString("%1 %2").arg(font.family()).arg(font.pointSize()));
}

ConfigDialog::~ConfigDialog() {
    delete ui;
}

void ConfigDialog::on_browseBgButton_clicked() {
    QString filename = QFileDialog::getOpenFileName(this,
        tr("Select a background image"),
        "image/backdrop/",
        tr("Images (*.png *.bmp *.jpg)"));

    if (!filename.isEmpty()) {
        ui->bgPathLineEdit->setText(filename);

        Config.BackgroundImage = filename;
        Config.setValue("BackgroundImage", filename);

        emit bg_changed();
    }
}

void ConfigDialog::on_resetBgButton_clicked() {
    ui->bgPathLineEdit->clear();

    QString filename = "image/backdrop/new-version.jpg";
    Config.BackgroundImage = filename;
    Config.setValue("BackgroundImage", filename);

    emit bg_changed();
}

void ConfigDialog::on_browseTableBgButton_clicked() {
    QString filename = QFileDialog::getOpenFileName(this,
        tr("Select a tableBg image"),
        "image/backdrop/",
        tr("Images (*.png *.bmp *.jpg)"));

    if (!filename.isEmpty()) {
        ui->tableBgPathLineEdit->setText(filename);

        Config.TableBgImage = filename;
        Config.setValue("TableBgImage", filename);

        emit tableBg_changed();
    }
}

void ConfigDialog::on_resetTableBgButton_clicked() {
    ui->tableBgPathLineEdit->clear();

    QString filename = "image/backdrop/default.jpg";
    Config.TableBgImage = filename;
    Config.setValue("TableBgImage", filename);

    emit tableBg_changed();
}

void ConfigDialog::on_browseRecordPathsButton_clicked() {
    QString paths = QFileDialog::getExistingDirectory(this,
        tr("Select a Record Paths"),
        "records/");

    if (!paths.isEmpty()) {
        ui->recordPathsSetupLineEdit->setText(paths);

        Config.RecordSavePaths = paths;
        Config.setValue("RecordSavePaths", paths);
    }
}

void ConfigDialog::on_resetRecordPathsButton_clicked() {
    ui->recordPathsSetupLineEdit->clear();

    QString paths = "records/";
    ui->recordPathsSetupLineEdit->setText(paths);
    Config.RecordSavePaths = paths;
    Config.setValue("RecordSavePaths", paths);
}

void ConfigDialog::saveConfig() {
    float volume = ui->bgmVolumeSlider->value() / 100.0;
    Config.BGMVolume = volume;
    Config.setValue("BGMVolume", volume);
    volume = ui->effectVolumeSlider->value() / 100.0;
    Config.EffectVolume = volume;
    Config.setValue("EffectVolume", volume);

    bool enabled = ui->enableEffectCheckBox->isChecked();
    Config.EnableEffects = enabled;
    Config.setValue("EnableEffects", enabled);

    enabled = ui->enableLastWordCheckBox->isChecked();
    Config.EnableLastWord = enabled;
    Config.setValue("EnableLastWord", enabled);

    enabled = ui->enableBgMusicCheckBox->isChecked();
    Config.EnableBgMusic = enabled;
    Config.setValue("EnableBgMusic", enabled);

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

    Config.EnableAutoPreshowInConsoleMode = ui->autoPreshowCheckBox->isChecked();
    Config.setValue("EnableAutoPreshowInConsoleMode",
                    Config.EnableAutoPreshowInConsoleMode);

    Config.BubbleChatBoxKeepSeconds = ui->bubbleChatBoxKeepSpinBox->value();
    Config.setValue("BubbleChatBoxKeepSeconds", Config.BubbleChatBoxKeepSeconds);

    Config.IgnoreOthersSwitchesOfSkin = ui->ignoreChangingSkinCheckBox->isChecked();
    Config.setValue("IgnoreOthersSwitchesOfSkin",
                    Config.IgnoreOthersSwitchesOfSkin);

    Config.EnableAutoSaveRecord = ui->enableAutoSaveCheckBox->isChecked();
    Config.setValue("EnableAutoSaveRecord", Config.EnableAutoSaveRecord);

    Config.NetworkOnly = ui->networkOnlyCheckBox->isChecked();
    Config.setValue("NetworkOnly", Config.NetworkOnly);
}

void ConfigDialog::on_browseBgMusicButton_clicked() {
    QString filename = QFileDialog::getOpenFileName(this,
        tr("Select a background music"),
        "audio/system",
        tr("Audio files (*.wav *.mp3 *.ogg)"));
    if (!filename.isEmpty()) {
        ui->bgMusicPathLineEdit->setText(filename);
        Config.setValue("BackgroundMusic", filename);
    }
}

void ConfigDialog::on_resetBgMusicButton_clicked() {
    QString default_music = "audio/system/background.ogg";
    Config.setValue("BackgroundMusic", default_music);
    ui->bgMusicPathLineEdit->setText(default_music);
}

void ConfigDialog::on_changeAppFontButton_clicked() {
    bool ok;
    QFont font = QFontDialog::getFont(&ok, Config.AppFont, this);
    if (ok) {
        Config.AppFont = font;
        showFont(ui->appFontLineEdit, font);

        Config.setValue("AppFont", font);
        QApplication::setFont(font);
    }
}


void ConfigDialog::on_setTextEditFontButton_clicked() {
    bool ok;
    QFont font = QFontDialog::getFont(&ok, Config.UIFont, this);
    if (ok) {
        Config.UIFont = font;
        showFont(ui->textEditFontLineEdit, font);

        Config.setValue("UIFont", font);
        QApplication::setFont(font, "QTextEdit");
    }
}

void ConfigDialog::on_setTextEditColorButton_clicked() {
    QColor color = QColorDialog::getColor(Config.TextEditColor, this);
    if (color.isValid()) {
        Config.TextEditColor = color;
        Config.setValue("TextEditColor", color);
        QPalette palette;
        palette.setColor(QPalette::Text, color);
        int aver = (color.red() + color.green() + color.blue()) / 3;
        palette.setColor(QPalette::Base, aver >= 208 ? Qt::black : Qt::white);
        ui->textEditFontLineEdit->setPalette(palette);
    }
}

void ConfigDialog::on_toolTipFontColorButton_clicked()
{
    QColor color = QColorDialog::getColor(Config.SkillDescriptionInToolTipColor, this);
    if (color.isValid()) {
        Config.SkillDescriptionInToolTipColor = color;
        Config.setValue("SkillDescriptionInToolTipColor", color);
    }
}

void ConfigDialog::on_overviewFontColorButton_clicked()
{
    QColor color = QColorDialog::getColor(QColor(Config.SkillDescriptionInOverviewColor), this);
    if (color.isValid()) {
        Config.SkillDescriptionInOverviewColor = color;
        Config.setValue("SkillDescriptionInOverviewColor", color);
    }
}

void ConfigDialog::on_toolTipBackgroundColorButton_clicked()
{
    QColor color = QColorDialog::getColor(QColor(Config.ToolTipBackgroundColor), this);
    if (color.isValid()) {
        Config.ToolTipBackgroundColor = color;
        Config.setValue("ToolTipBackgroundColor", color);
        QFile file("sanguosha.qss");
        QString styleSheet;
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream stream(&file);
            styleSheet = stream.readAll();
        }
        qApp->setStyleSheet(styleSheet + StyleHelper::styleSheetOfTooltip());
    }
}

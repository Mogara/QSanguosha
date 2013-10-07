#include "configdialog.h"
#include "ui_configdialog.h"
#include "settings.h"

#include <QFileDialog>
#include <QDesktopServices>
#include <QFontDialog>
#include <QColorDialog>

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
    ui->doubleClickCheckBox->setChecked(Config.EnableDoubleClick);

    connect(this, SIGNAL(accepted()), this, SLOT(saveConfig()));

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
                                                    "backdrop/",
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

    QString filename = "backdrop/new-version.jpg";
    Config.BackgroundImage = filename;
    Config.setValue("BackgroundImage", filename);

    emit bg_changed();
}

void ConfigDialog::on_browseTableBgButton_clicked() {
    QString filename = QFileDialog::getOpenFileName(this,
        tr("Select a tableBg image"),
        "backdrop/",
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

    QString filename = "backdrop/default.jpg";
    Config.TableBgImage = filename;
    Config.setValue("TableBgImage", filename);

    emit tableBg_changed();
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

    Config.EnableDoubleClick = ui->doubleClickCheckBox->isChecked();
    Config.setValue("EnableDoubleClick", Config.EnableDoubleClick);
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


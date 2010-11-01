#include "configdialog.h"
#include "ui_configdialog.h"
#include "settings.h"

#include <QFileDialog>
#include <QDesktopServices>

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);

    // tab 1
    QString bg_path = Config.value("BackgroundBrush").toString();
    if(!bg_path.startsWith(":"))
        ui->bgPathLineEdit->setText(bg_path);

    ui->bgMusicPathLineEdit->setText(Config.value("BackgroundMusic").toString());

    ui->enableEffectCheckBox->setChecked(Config.EnableEffects);
    ui->enableBgMusicCheckBox->setChecked(Config.EnableBgMusic);
    ui->fitInViewCheckBox->setChecked(Config.FitInView);

    ui->volumeSlider->setValue(100 * Config.Volume);

    // tab 2
    ui->nullificationSpinBox->setValue(Config.NullificationCountDown);
    ui->neverNullifyMyTrickCheckBox->setChecked(Config.NeverNullifyMyTrick);

    connect(this, SIGNAL(accepted()), this, SLOT(saveConfig()));
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::on_browseBgButton_clicked()
{
    QString location = QDesktopServices::storageLocation(QDesktopServices::PicturesLocation);
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Select a background image"),
                                                    location,
                                                    tr("Images (*.png *.bmp *.jpg)"));

    if(!filename.isEmpty()){
        ui->bgPathLineEdit->setText(filename);
        Config.changeBackground(filename);

        emit bg_changed();
    }
}

void ConfigDialog::on_resetBgButton_clicked()
{
    ui->bgPathLineEdit->clear();
    Config.changeBackground(":/background.png");
    emit bg_changed();
}

void ConfigDialog::saveConfig()
{
    int count_down = ui->nullificationSpinBox->value();
    Config.NullificationCountDown = count_down;
    Config.setValue("NullificationCountDown", count_down);

    float volume = ui->volumeSlider->value() / 100.0;
    Config.Volume = volume;
    Config.setValue("Volume", volume);

    bool enabled = ui->enableEffectCheckBox->isChecked();
    Config.EnableEffects = enabled;
    Config.setValue("EnableEffects", enabled);

    enabled = ui->enableBgMusicCheckBox->isChecked();
    Config.EnableBgMusic = enabled;
    Config.setValue("EnableBgMusic", enabled);

    Config.FitInView = ui->fitInViewCheckBox->isChecked();
    Config.setValue("FitInView", Config.FitInView);

    Config.NeverNullifyMyTrick = ui->neverNullifyMyTrickCheckBox->isChecked();
    Config.setValue("NeverNullifyMyTrick", Config.NeverNullifyMyTrick);
}

void ConfigDialog::on_browseBgMusicButton_clicked()
{
    QString location = QDesktopServices::storageLocation(QDesktopServices::MusicLocation);
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Select a background music"),
                                                    location,
                                                    tr("Audio files (*.wav *.mp3)"));
    if(!filename.isEmpty()){
        ui->bgMusicPathLineEdit->setText(filename);
        Config.setValue("BackgroundMusic", filename);
    }
}

void ConfigDialog::on_resetBgMusicButton_clicked()
{
    QString default_music = "audio/background.mp3";
    Config.setValue("BackgroundMusic", default_music);
    ui->bgMusicPathLineEdit->setText(default_music);
}

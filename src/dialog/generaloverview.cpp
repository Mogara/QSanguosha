#include "generaloverview.h"
#include "engine.h"
#include "generalmodel.h"
#include "audio.h"

#include <QFileInfo>
#include <QLineEdit>
#include <QCompleter>
#include <QComboBox>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QButtonGroup>
#include <QRadioButton>
#include <QGridLayout>
#include <QPushButton>
#include <QAbstractItemView>
#include <QRegExpValidator>
#include <QLabel>
#include <QListView>
#include <QGroupBox>
#include <QMenu>
#include <QTextBrowser>

GeneralOverview::GeneralOverview()
{
    QHBoxLayout *layout = new QHBoxLayout;

    layout->addLayout(createLeft());
    layout->addLayout(createMiddle());
    layout->addLayout(createRight());
    layout->addStretch();

    setLayout(layout);

    doSearch();
    showGeneral("caocao");

    resize(900, 600);

    setWindowTitle(tr("General Overview"));
}

Q_GLOBAL_STATIC(GeneralOverview, GlobalGeneralOverview)

void GeneralOverview::display(const QString &name)
{
    GeneralOverview *instance = ::GlobalGeneralOverview();
    instance->show();

    if(!name.isNull())
        instance->showGeneral(name);
}

void GeneralOverview::showGeneral(const QString &name)
{
    const General *g = Sanguosha->getGeneral(name);
    if(g == NULL)
        return;

    generalImage->setPixmap(QPixmap(g->getPixmapPath("card")));

    InfoRows infoRows;
    infoRows.add(tr("Name"), g->objectName(), true)
            .add(tr("Gender"), g->getGenderString(), true)
            .add(tr("Kingdom"), g->getKingdom(), true)
            .add(tr("Package"), g->getPackage(), true)
            .add(tr("Max HP"), QString::number(g->getMaxHp()))
            .add(tr("Status"), g->isLord() ? tr("Lord") : tr("Non lord"))
            .add(tr("Designer"), g->getDesigner())
            .add(tr("Illustrator"), g->getIllustrator())
            .add(tr("CV"), g->getCV());

    generalInfo->setText(infoRows.toTableString());
    generalSkill->setText(g->getSkillDescription());

    InfoRows lines;
    foreach(const Skill *skill, g->getVisibleSkillList()){
        QStringList sources = skill->getSources();

        if(sources.isEmpty()){
            lines.add(Sanguosha->translate(skill->objectName()), tr("No effect"));
            continue;
        }

        for(int i=0; i<sources.length(); i++){
            QFileInfo info(sources.at(i));

            QString name;
            if(sources.length() == 1){
                name = Sanguosha->translate(skill->objectName());
            }else{
                name= QString("%1%2").arg(Sanguosha->translate(skill->objectName())).arg(i+1);
            }

            QString word = Sanguosha->translate("$" + info.baseName());
            QString path = sources.at(i);

            lines.add(name, CreateLinkString(path, word));
        }
    }

    lines << "<hr/>";

    QString winWord = g->getWinWord();
    if(!winWord.isEmpty())
        lines.add(tr("Win"), CreateLinkString(g->getWinEffectPath(), winWord));

    QString lastWord = g->getLastWord();
    if(!lastWord.isEmpty())
        lines.add(tr("Death"), CreateLinkString(g->getLastEffectPath(), lastWord));

#ifdef QT_DEBUG
    lines.add("", CreateLinkString("export", tr("Export all audio files")));
#endif

    effectBrowser->setText(lines.toTableString());
}

QHBoxLayout *GeneralOverview::addButtonsFromStringList(const QStringList &list, const char *configName)
{
    QHBoxLayout *hlayout = new QHBoxLayout;
    QButtonGroup *group = new QButtonGroup;

    group->setObjectName(configName);

    // add special item: all
    QRadioButton *allButton = new QRadioButton(tr("All"));
    hlayout->addWidget(allButton);
    group->addButton(allButton);

    foreach(QString elem, list){
        QRadioButton *button = new QRadioButton(Sanguosha->translate(elem));
        button->setObjectName(elem);
        hlayout->addWidget(button);
        group->addButton(button);
    }

    hlayout->addStretch();

    group->buttons().first()->setChecked(true);

    connect(group, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(onRadioButtonClicked(QAbstractButton*)));

    return hlayout;
}

QLayout *GeneralOverview::createLeft()
{
    QFormLayout *searchLayout = new QFormLayout;

    {
        // search box
        QLineEdit *box = new QLineEdit;

        box->setMinimumWidth(300);

        box->setValidator(new QRegExpValidator(QRegExp("[0-9a-z_]+"), box));

        GeneralCompleterModel *model = new GeneralCompleterModel;
        QCompleter *completer = new QCompleter(model);
        completer->popup()->setIconSize(General::TinyIconSize);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        completer->setModelSorting(QCompleter::CaseSensitivelySortedModel); // as our completer model is sorted, use this will improve performance

        box->setCompleter(completer);

        searchLayout->addRow(tr("Search"), box);

        connect(box, SIGNAL(returnPressed()), this, SLOT(onSearchBoxDone()));
    }

    {
        QStringList genders;
        genders << "male" << "female" << "neuter";
        searchLayout->addRow(tr("Gender"), addButtonsFromStringList(genders, "gender"));
    }

    {
        searchLayout->addRow(tr("Kingdom"), addButtonsFromStringList(Sanguosha->getKingdoms(), "kingdom"));
    }

    {
        searchLayout->addRow(tr("Status"), addButtonsFromStringList(QStringList() << "lord" << "nonlord", "status"));
    }

    {
        QComboBox *box = new QComboBox;
        box->addItem(tr("All"));
        for(int i=1; i<=8; i++){
            box->addItem(QString::number(i), i);
        }

        QHBoxLayout *hlayout = new QHBoxLayout;
        hlayout->addWidget(box);
        hlayout->addStretch();
        searchLayout->addRow(tr("Max HP"), hlayout);

        connect(box, SIGNAL(currentIndexChanged(int)), this, SLOT(onMaxHpIndexChanged(int)));
    }

    {
        QPushButton *button = new QPushButton;
        button->setObjectName("packageName");

        QMenu *menu = new QMenu;

        QActionGroup *group = new QActionGroup(this);

        QAction *allAction = menu->addAction(tr("All"));
        allAction->setActionGroup(group);
        allAction->setCheckable(true);

        button->setText(allAction->text());

        foreach(const Package *p, Sanguosha->findChildren<const Package *>()){
            if(p->getType() == Package::GeneralPack || p->getType() == Package::MixedPack){
                QAction *action = menu->addAction(Sanguosha->translate(p->objectName()));
                action->setObjectName(p->objectName());
                action->setCheckable(true);
                action->setActionGroup(group);
            }
        }

        connect(group, SIGNAL(triggered(QAction*)), this, SLOT(onPackageActionTriggered(QAction*)));

        button->setMenu(menu);

        QHBoxLayout *hlayout = new QHBoxLayout;
        hlayout->addWidget(button);
        hlayout->addStretch();
        searchLayout->addRow(tr("Package"), hlayout);
    }

    {
        generalLabel = new QLabel;
        generalLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        generalView = new QListView;

        generalView->setViewMode(QListView::IconMode);

        generalView->setGridSize(QSize(55, 60));
        generalView->setIconSize(General::TinyIconSize);
        generalView->setModel(new GeneralListModel);
        generalView->setMinimumSize(300, 400);

        searchLayout->addRow(generalLabel, generalView);

        connect(generalView, SIGNAL(clicked(QModelIndex)), this, SLOT(onGeneralViewClicked(QModelIndex)));
    }

    return searchLayout;
}

QLayout *GeneralOverview::createMiddle()
{
    QVBoxLayout *vlayout = new QVBoxLayout;

    generalImage = new QLabel;
    generalImage->setMinimumSize(QSize(200, 290));
    generalImage->setFrameShape(QFrame::Box);

    generalSkill = new QTextBrowser;
    generalSkill->setReadOnly(true);
    generalSkill->setMaximumWidth(200);

    vlayout->addWidget(generalImage);
    vlayout->addWidget(generalSkill);

    return vlayout;
}


QLayout *GeneralOverview::createRight()
{
    QGroupBox *infoBox = new QGroupBox(tr("Information"));
    QVBoxLayout *infoLayout = new QVBoxLayout;
    infoLayout->addWidget(generalInfo = new QLabel);
    infoBox->setLayout(infoLayout);

    QGroupBox *effectBox = new QGroupBox(tr("Effects"));
    QVBoxLayout *effectLayout = new QVBoxLayout;
    effectBrowser = new QTextBrowser;
    effectBrowser->setOpenLinks(false);
    effectLayout->addWidget(effectBrowser);

    connect(effectBrowser, SIGNAL(anchorClicked(QUrl)), this, SLOT(onEffectLabelClicked(QUrl)));
    effectBox->setLayout(effectLayout);

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addWidget(infoBox);
    vlayout->addWidget(effectBox);

    return vlayout;
}

void GeneralOverview::doSearch()
{
    GeneralListModel *model = qobject_cast<GeneralListModel *>(generalView->model());
    model->doSearch(options);

    generalLabel->setText(tr("Generals\n(%1)").arg(model->rowCount(QModelIndex())));
}

void GeneralOverview::onSearchBoxDone()
{
    QLineEdit *box = qobject_cast<QLineEdit *>(sender());
    if(box){
        showGeneral(box->text());
        box->clear();
    }
}

void GeneralOverview::onMaxHpIndexChanged(int index)
{
    QComboBox *box = qobject_cast<QComboBox *>(sender());
    QVariant data = box->itemData(index);
    if(data.isValid())
        options["maxhp"] = data.toString();
    else
        options.remove("maxhp");

    doSearch();
}

void GeneralOverview::onPackageActionTriggered(QAction *action)
{
    findChild<QPushButton *>("packageName")->setText(action->text());

    options["package"] = action->objectName();
    doSearch();
}

void GeneralOverview::onGeneralViewClicked(const QModelIndex &index)
{
    const General *g = static_cast<const General *>(index.internalPointer());
    showGeneral(g->objectName());
}

void GeneralOverview::onRadioButtonClicked(QAbstractButton *button)
{
    QButtonGroup *g = button->group();
    if(g){
        options[g->objectName()] = button->objectName();
        doSearch();
    }
}

#include <QFileDialog>

#include "crypto.h"

void GeneralOverview::onEffectLabelClicked(const QUrl &url)
{
    QString link = url.toString();
#ifdef QT_DEBUG

    if(link.startsWith("export")){
        QString dirname = QFileDialog::getExistingDirectory(this);
        if(dirname.isEmpty())
            return;

        QLabel *label = qobject_cast<QLabel *>(sender());
        QString str = label->text();
        QRegExp rx("href='([^']+)'");

        int pos = 0;
        QStringList filenames;
        while ((pos = rx.indexIn(str, pos)) != -1) {
            pos += rx.matchedLength();
            filenames << rx.capturedTexts()[1];
        }

        filenames.removeLast();

        QDir destDir(dirname);
        foreach(QString filename, filenames){
            QString path = QFileInfo(filename).dir().path();
            destDir.mkpath(path);

            QString newName = QString("%1/%2").arg(dirname).arg(filename);
            QFile::copy(filename, newName);

            if(newName.endsWith("dat")){
                QString oggName = newName;
                oggName.replace(QRegExp("dat$"), "ogg");
                Crypto::decryptFile(newName, oggName);
                QFile::remove(newName);
            }
        }

        return;
    }

#endif

    Audio::play(link);
}


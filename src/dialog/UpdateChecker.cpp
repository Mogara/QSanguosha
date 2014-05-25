#include "UpdateChecker.h"
#include "engine.h"

#include <QFormLayout>

UpdateChecker::UpdateChecker()
{
    state_label = new QLabel;
    address_label = new QLabel;
    list_widget = new QListWidget;
    list_widget->setViewMode(QListView::ListMode);
    list_widget->setMovement(QListView::Static);

    QFormLayout *layout = new QFormLayout;
    layout->addRow(state_label);
    layout->addRow(tr("Download Address"), address_label);
    layout->addRow(tr("What's New"), list_widget);

    setLayout(layout);
}

void UpdateChecker::fill( UpdateInfoStruct info )
{
    QString state;
    if (info.version_number > Sanguosha->getVersionNumber()) {
        if (info.is_patch)
            state = tr("New Patch Available");
        else
            state = tr("New Client Available");
    } else
        state = tr("Lastest Version Already");
    state_label->setText(state);

    address_label->setOpenExternalLinks(true);
    address_label->setText(QString("<a href='%1' style = \"color:#0072c1; \">%1</a> <br/>").arg(info.address));

    if (info.whats_new.isEmpty())
        list_widget->hide();
    else {
        foreach(QString line, info.whats_new)
            new QListWidgetItem(line, list_widget);
    }
}

void UpdateChecker::clear()
{
    state_label->clear();
    address_label->clear();
    list_widget->clear();
}



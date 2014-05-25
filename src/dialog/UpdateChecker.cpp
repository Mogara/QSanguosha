#include "UpdateChecker.h"
#include "engine.h"

#include <QFormLayout>
#include <QFile>

UpdateChecker::UpdateChecker()
{
    state_label = new QLabel(this);
    address_label = new QLabel(this);
    page = new QTextEdit(this);
    page->setReadOnly(true);

    QFormLayout *layout = new QFormLayout;
    layout->addRow(state_label);
    layout->addRow(tr("Download Address"), address_label);
    layout->addRow(tr("What's New"), page);

    setLayout(layout);
}

void UpdateChecker::fill( UpdateInfoStruct info )
{
    QString state;
    if (info.version_number > Sanguosha->getVersionNumber()) {
        QString postfix = " : " + info.version_number;
        if (info.is_patch)
            state = tr("New Patch Available") + postfix;
        else
            state = tr("New Client Available") + postfix;
    } else
        state = tr("Lastest Version Already");
    state_label->setText(state);

    address_label->setOpenExternalLinks(true);
    address_label->setText(QString("<a href='%1' style = \"color:#0072c1; \">%1</a> <br/>").arg(info.address));

    QFile file("info.html");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        QString content = stream.readAll();
        page->setHtml(content);
    }
}

void UpdateChecker::clear()
{
    state_label->clear();
    address_label->clear();
    page->clear();
}



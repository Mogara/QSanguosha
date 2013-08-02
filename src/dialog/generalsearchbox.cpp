#include "generalsearchbox.h"
#include "engine.h"
#include "generalmodel.h"

#include <QCompleter>
#include <QRegExpValidator>
#include <QAbstractItemView>

GeneralSearchBox::GeneralSearchBox(QWidget *parent) :
    QLineEdit(parent)
{
    setMinimumWidth(300);

    setValidator(new QRegExpValidator(QRegExp("[0-9a-z_]+"), this));

    QCompleter *completer = new QCompleter(GeneralCompleterModel::getInstance());
    completer->popup()->setIconSize(General::TinyIconSize);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setModelSorting(QCompleter::CaseSensitivelySortedModel); // as our completer model is sorted, use this will improve performance

    setCompleter(completer);

    connect(this, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()));
}

void GeneralSearchBox::onReturnPressed()
{
    QString name = text();

    const General *g = Sanguosha->getGeneral(name);
    if(g == NULL){
        const Skill *s = Sanguosha->getSkill(name);
        if(s){
            g = qobject_cast<const General *>(s->parent());
        }
    }

    if(g){
        emit generalInput(g->objectName());
        clear();
    }
}

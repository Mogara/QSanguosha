#include "generalmodel.h"
#include "engine.h"

GeneralModel::GeneralModel()
{
    generals = Sanguosha->findChildren<const General *>();
    QMutableListIterator<const General *> itor(generals);
    while(itor.hasNext()){
        itor.next();

        if(itor.value()->isTotallyHidden())
            itor.remove();
    }

    QSet<const Skill *> skillSet;
    foreach(const General *g, generals){
       skillSet += g->getVisibleSkills();
    }

    skills << skillSet.toList();
}

int GeneralModel::rowCount(const QModelIndex &parent) const
{
    if(parent.isValid())
        return 0;
    else
        return generals.length() + skills.length();
}

QVariant GeneralModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if(row < 0)
        return QVariant();

    if(row >= 0 && row < generals.length()){
        const General *g = generals[row];

        switch(role){
        case Qt::EditRole: return g->objectName();
        case Qt::DisplayRole: return QString("%1 (%2)").arg(Sanguosha->translate(g->objectName())).arg(g->objectName());
        case Qt::BackgroundRole: if(g->isHidden()) return QBrush(Qt::gray);
        case Qt::DecorationRole: return QIcon(g->getPixmapPath("tiny"));
        }
    }else if(row >= generals.length() && row <= generals.length() + skills.length()){
        const Skill *s = skills[row - generals.length()];
        switch(role){
        case Qt::EditRole: return s->objectName();
        case Qt::DisplayRole: {
            const General *g = qobject_cast<const General *>(s->parent());
            if(g){
                return tr("%1 has skill %2 (%3)")
                        .arg(Sanguosha->translate(g->objectName()))
                        .arg(Sanguosha->translate(s->objectName()))
                        .arg(s->objectName());
            }
        }
        case Qt::DecorationRole:{
            const General *g = qobject_cast<const General *>(s->parent());
            if(g){
                return QIcon(g->getPixmapPath("tiny"));
            }
        }
        }
    }

    return QVariant();
}



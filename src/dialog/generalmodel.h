#ifndef GENERALMODEL_H
#define GENERALMODEL_H

#include <QAbstractListModel>

#include "general.h"
#include "skill.h"

class GeneralModel : public QAbstractListModel{
    Q_OBJECT
public:

    explicit GeneralModel();

    virtual int rowCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

private:
    GeneralList generals;
    SkillList skills;
};

#endif // GENERALMODEL_H

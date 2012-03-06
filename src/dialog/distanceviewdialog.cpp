#include "distanceviewdialog.h"

#include "client.h"
#include "clientplayer.h"
#include "engine.h"
#include "roomscene.h"

#include <QFormLayout>
#include <QComboBox>
#include <QGroupBox>

struct DistanceViewDialogUI{
    DistanceViewDialogUI(){
        from = new QComboBox;
        to = new QComboBox;

        from_seat = new QLineEdit;
        to_seat = new QLineEdit;

        from_seat->setReadOnly(true);
        to_seat->setReadOnly(true);

        left = new QLineEdit;
        right = new QLineEdit;

        min = new QLineEdit;
        in_attack = new QLineEdit;

        QList<const DistanceSkill *> skills = Sanguosha->getDistanceSkills();
        foreach(const DistanceSkill *skill, skills){
            QLineEdit *distance_edit = new QLineEdit;
            distance_edit->setObjectName(skill->objectName());
            distance_edit->setReadOnly(true);

            distance_edits << distance_edit;
        }

        final = new QLineEdit;
        final->setReadOnly(true);
    }

    QComboBox *from, *to;
    QLineEdit *from_seat, *to_seat;
    QLineEdit *left, *right;
    QLineEdit *min;
    QList<QLineEdit *> distance_edits;
    QLineEdit *in_attack;
    QLineEdit *final;
};

DistanceViewDialog::DistanceViewDialog(QWidget *parent) :
    QDialog(parent)
{

    QFormLayout *layout = new QFormLayout;

    ui = new DistanceViewDialogUI;

    RoomScene::FillPlayerNames(ui->from, false);
    RoomScene::FillPlayerNames(ui->to, false);

    connect(ui->from, SIGNAL(currentIndexChanged(int)), this, SLOT(showDistance()));
    connect(ui->to, SIGNAL(currentIndexChanged(int)), this, SLOT(showDistance()));

    layout->addRow(tr("From"), ui->from);
    layout->addRow(tr("To"), ui->to);
    layout->addRow(tr("From seat"), ui->from_seat);
    layout->addRow(tr("To seat"), ui->to_seat);
    layout->addRow(tr("Left"), ui->left);
    layout->addRow(tr("Right"), ui->right);
    layout->addRow(tr("Minimum"), ui->min);

    QGroupBox *box = new QGroupBox();
    layout->addRow(tr("Distance correct"), box);

    QFormLayout *box_layout = new QFormLayout;
    foreach(QLineEdit *edit, ui->distance_edits){
        box_layout->addRow(Sanguosha->translate(edit->objectName()), edit);
    }

    box->setLayout(box_layout);

    layout->addRow(tr("In attack range"), ui->in_attack);

    layout->addRow(tr("Final"), ui->final);

    setLayout(layout);

    showDistance();
}

DistanceViewDialog::~DistanceViewDialog()
{
    delete ui;
}

void DistanceViewDialog::showDistance()
{
    QString from_name = ui->from->itemData(ui->from->currentIndex()).toString();
    QString to_name = ui->to->itemData(ui->to->currentIndex()).toString();

    const ClientPlayer *from = ClientInstance->getPlayer(from_name);
    const ClientPlayer *to = ClientInstance->getPlayer(to_name);

    ui->from_seat->setText(QString::number(from->getSeat()));
    ui->to_seat->setText(QString::number(to->getSeat()));

    int left_distance = qAbs(from->getSeat() +
                             ((from->getSeat()<to->getSeat())?from->aliveCount():-from->aliveCount())
                             - to->getSeat());
    ui->left->setText(QString("|%1%2%3-%4|=%5")
                      .arg(from->getSeat())
                      .arg((from->getSeat()<to->getSeat())?"+":"-")
                      .arg(from->aliveCount())
                      .arg(to->getSeat())
                      .arg(left_distance)
                      );

    int right_distance = qAbs(from->getSeat() - to->getSeat());
    ui->right->setText(QString("|%1-%2|=%3")
                       .arg(from->getSeat())
                       .arg(to->getSeat())
                       .arg(right_distance)
                       );

    int min = qMin(left_distance, right_distance);
    ui->min->setText(QString("min(%1, %2)=%3")
                     .arg(left_distance)
                     .arg(right_distance)
                     .arg(min)
                     );

    foreach(QLineEdit *edit, ui->distance_edits){
        const Skill *skill = Sanguosha->getSkill(edit->objectName());
        const DistanceSkill *distance_skill = qobject_cast<const DistanceSkill *>(skill);
        int correct = distance_skill->getCorrect(from, to);

        if(correct > 0)
            edit->setText(QString("+%1").arg(correct));
        else if(correct < 0)
            edit->setText(QString::number(correct));
    }

    ui->in_attack->setText(from->inMyAttackRange(to) ? tr("Yes") : tr("No"));

    ui->final->setText(QString::number(from->distanceTo(to)));
}

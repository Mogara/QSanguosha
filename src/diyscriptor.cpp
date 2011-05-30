#include "diyscriptor.h"
#include "ui_diyscriptor.h"
#include "mainwindow.h"

DIYScriptor::DIYScriptor(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DIYScriptor)
{
    ui->setupUi(this);
}

DIYScriptor::~DIYScriptor()
{
    delete ui;
}

void MainWindow::on_actionDIY_editor_triggered()
{
    DIYScriptor *scriptor = new DIYScriptor(this);
    scriptor->show();
}

void DIYScriptor::on__gb_clicked()
{
    QString output="";
    output.append(ui->_name->text());
    output.append("= sgs.CreateViewAsSkill{\r\n\r\n");

    output.append("name = \""+ui->_name->text()+"\",\r\n");

    output.append("n = "+ui->_n->text()+",\r\n");

    output.append("\r\n");
    output.append("view_filter = function(self, to_select, selected)\r\n");
    output.append(ui->_vf->toPlainText()+"\r\n");
    output.append("return condition\r\n");
    output.append("end,\r\n\r\n");

    output.append("view_as = function(self, cards)\r\n");
    output.append(ui->_va_1->toPlainText()+"\r\n");
    output.append("if invalid_condition then return nil end\r\n");
    output.append(ui->_va_2->toPlainText()+"\r\n");
    output.append("return view_as_card\r\n");
    output.append("end,\r\n\r\n");

    if(ui->_ea_1->toPlainText()!=""){
        output.append("enabled_at_play = function()\r\n");
        output.append(ui->_ea_1->toPlainText()+"\r\n");
        output.append("return condition\r\n");
        output.append("end,\r\n\r\n");
    }

    if(ui->_ea_2->toPlainText()!=""){
        output.append("enabled_at_response = function()\r\n");
        output.append(ui->_ea_1->toPlainText()+"\r\n");
        output.append("return condition\r\n");
        output.append("end,\r\n\r\n");
    }

    output.append("}");

    ui->_r->setText(output);
}


void DIYScriptor::syncText()
{
    ui->_cv->setText(editionArea->toPlainText());

}

void DIYScriptor::backSync()
{
    editionArea->setText(ui->_cv->toPlainText());
}

void DIYScriptor::on__vf_selectionChanged()
{
    editionArea=ui->_vf;
    syncText();
}

void DIYScriptor::on__va_1_selectionChanged()
{
    editionArea=ui->_va_1;
    syncText();
}

void DIYScriptor::on__va_2_selectionChanged()
{
    editionArea=ui->_va_2;
    syncText();
}

void DIYScriptor::on__ea_1_selectionChanged()
{
    editionArea=ui->_ea_1;
    syncText();
}

void DIYScriptor::on__ea_2_selectionChanged()
{
    editionArea=ui->_ea_2;
    syncText();
}

void DIYScriptor::on__cv_textChanged()
{
    backSync();
}

void DIYScriptor::on__sb_clicked()
{
//dummy
}

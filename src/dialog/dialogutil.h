#ifndef DIALOGUTIL_H
#define DIALOGUTIL_H

class QDialog;
class QHBoxLayout;
class QGroupBox;
class QWidget;

QHBoxLayout *CreateOKCancelLayout(QDialog *dialog);
QGroupBox *CreateGroupBoxWithWidget(QWidget *widget);

#endif // DIALOGUTIL_H

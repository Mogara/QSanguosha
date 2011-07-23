#include "optionbutton.h"
#include "settings.h"

OptionButton::OptionButton(QString icon_path, const QString &caption, QWidget *parent)
    :QToolButton(parent)
{
    QPixmap pixmap(icon_path);
    QIcon icon(pixmap);

    setIcon(icon);
    setIconSize(pixmap.size());

    if(!caption.isEmpty()){
        setText(caption);
        setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        if(caption.length()>= 4){
            QFont font = Config.SmallFont;
            font.setPixelSize(Config.SmallFont.pixelSize() - 5);
            setFont(font);
        }else
            setFont(Config.SmallFont);
    }
}

void OptionButton::mouseDoubleClickEvent(QMouseEvent *){
    emit double_clicked();
}

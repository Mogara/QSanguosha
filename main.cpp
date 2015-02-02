#include <QGuiApplication>
#include "cmainwindow.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQuickView window;
    window.setSource(QUrl(QStringLiteral("qrc:/main.qml")));
    window.show();

    return app.exec();
}

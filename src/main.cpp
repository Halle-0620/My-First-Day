#include <QApplication>

#include "gamewindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("MyFirstDay");
    QApplication::setOrganizationName("MyFirstDay");

    GameWindow window;
    window.show();

    return app.exec();
}

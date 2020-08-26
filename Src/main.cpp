#include <QApplication>

#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    // QCoreApplication::setOrganizationName("");
    QCoreApplication::setApplicationName("BooleanOffset");
    QCoreApplication::setApplicationVersion("v0.1");

    BooleanOffset::MainWindow mainWin;
    mainWin.show();
    return app.exec();
}

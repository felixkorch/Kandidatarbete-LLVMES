#include <QApplication>
#include "mainwindow.h"

#ifdef _WIN32
int WinMain(int argc, char *argv[]) // Required to remove the console window on Windows
#else
int main(int argc, char *argv[])
#endif
{
    QApplication a(argc, argv);

    MainWindow *mainWindow = new MainWindow;
    mainWindow->show();

    return a.exec();
}
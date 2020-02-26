#include <darkstyle.h>
#include <framelesswindow.h>

#include <QApplication>

#include "mainwindow.h"

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle(new DarkStyle);
    // create frameless window (and set windowState or title)
    FramelessWindow framelessWindow;
    framelessWindow.setWindowTitle("LLVMES - Debugger");
    // TODO: Make it scaleable
    framelessWindow.setFixedSize(700, 500);

    MainWindow *mainWindow = new MainWindow;
    framelessWindow.setContent(mainWindow);
    framelessWindow.show();

    return a.exec();
}

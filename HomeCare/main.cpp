#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{

    //char const *exit_strings = "/EXIT /Exit /exit /q /Q "; // strings for exit, check one of them
    QApplication a(argc, argv);
    MainWindow w;

    w.show();

    return a.exec();
}

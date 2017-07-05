#include "mainwindow.h"
#include <QApplication>
#include <QFont>
#include <QFontDatabase>
#include <QtGlobal>
int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    QFont font("Segoe UI");
    font.setPixelSize(11);
    QApplication::setFont(font);
    QApplication::setAttribute(Qt::AA_Use96Dpi);
    qSetMessagePattern("%{file} (%{function}, %{line}) : %{message}");
    MainWindow w;
    w.show();


    return a.exec();
}



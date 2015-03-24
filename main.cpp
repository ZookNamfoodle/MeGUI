#include "HPC.h"
#include <QApplication>
#include <QSplashScreen>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QPixmap pixmap(":/images/resources/splash.png");
    QSplashScreen splash(pixmap);
    splash.show();

    HPC w;

    QTimer::singleShot(2500,&splash,SLOT(close()));
    QTimer::singleShot(2500,&w,SLOT(show()));
    //w.show();

    return a.exec();
}

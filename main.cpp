#include "HPC.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    HPC w;
    w.show();

    return a.exec();
}

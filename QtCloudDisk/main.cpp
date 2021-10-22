#include "log.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    log w;

    w.show();
    return a.exec();
}

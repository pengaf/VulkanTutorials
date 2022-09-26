#include "Tutorial01.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Tutorial01 w;
    w.show();
    return a.exec();
}

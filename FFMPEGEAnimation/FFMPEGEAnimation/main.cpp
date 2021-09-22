#include "ffAnimationTest.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ffAnimationTest w;
    w.show();
    return a.exec();
}

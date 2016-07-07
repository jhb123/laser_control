/* main
 *
 * This file just starts the application and ui objects.
 * */

#include <QApplication>
#include "touchscreen.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TouchScreen w;
    w.show();

    return a.exec();
}

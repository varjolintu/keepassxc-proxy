#include <QApplication>
#include "chromelistener.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    ChromeListener listener;
    listener.run();
    return app.exec();
}

#include <QApplication>
#include "chromelistener.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    ChromeListener listener;
    listener.run();
#ifdef QT_DEBUG
    listener.show();
#endif
    return app.exec();
}

#include <QCoreApplication>
#include "NativeMessagingHost.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    NativeMessagingHost host;
    return a.exec();
}

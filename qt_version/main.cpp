#include <QCoreApplication>
#include "NativeMessagingHost.h"

#ifndef Q_OS_WIN
#include <initializer_list>
#include <signal.h>
#include <unistd.h>

// Gist: https://gist.github.com/azadkuh/a2ac6869661ebd3f8588
void ignoreUnixSignals(std::initializer_list<int> ignoreSignals) {
    for (int sig : ignoreSignals) {
        signal(sig, SIG_IGN);
    }
}

void catchUnixSignals(std::initializer_list<int> quitSignals) {
    auto handler = [](int sig) -> void {
        QCoreApplication::quit();
    };

    sigset_t blocking_mask;
    sigemptyset(&blocking_mask);
    for (auto sig : quitSignals) {
        sigaddset(&blocking_mask, sig);
    }

    struct sigaction sa;
    sa.sa_handler = handler;
    sa.sa_mask    = blocking_mask;
    sa.sa_flags   = 0;

    for (auto sig : quitSignals) {
        sigaction(sig, &sa, nullptr);
    }
}
#endif

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
#ifndef Q_OS_WIN
    catchUnixSignals({SIGQUIT, SIGINT, SIGTERM, SIGHUP});
#endif
    NativeMessagingHost host;
    return a.exec();
}

#ifndef NATIVEMESSAGINGHOST_H
#define NATIVEMESSAGINGHOST_H

#include <QObject>
#include <QLocalSocket>
#include <QSharedPointer>
#include <QSocketNotifier>
#include <QFile>
#include <iostream>
#include <unistd.h>

class NativeMessagingHost : public QObject
{
    Q_OBJECT
public:
    NativeMessagingHost();

public slots:
    void newMessage();
    void newLocalMessage();
    void deleteSocket();

private:
    void sendReply(const QString& reply);

private:
    QSharedPointer<QSocketNotifier>         m_notifier;
    QSharedPointer<QLocalSocket>            m_localSocket;

};

#endif // NATIVEMESSAGINGHOST_H

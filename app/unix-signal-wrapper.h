#ifndef UNIXSIGNALWRAPPER_H
#define UNIXSIGNALWRAPPER_H

#include <QObject>
#include <QSocketNotifier>

class UnixSignalWrapper : public QObject
{
    Q_OBJECT
public:
    explicit UnixSignalWrapper(QObject *parent = 0);

signals:
    void unixSignalReceived();

private slots:
    void handleUnixSignal();

private:
    static bool isHandlersInited;
    static int sigFd[2];
    static void initSignalHandlers();
    static void unixSignalHandler(int sigNum);

    QSocketNotifier *snUnixSignal;
};

#endif // UNIXSIGNALWRAPPER_H

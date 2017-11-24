#include "unix-signal-wrapper.h"
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>

/**
 *  Handle unix signals SIGINT, SIGKILL and SIGHUP
 * emit qt signal unixSignalReceived()
 */

// Init static member
int UnixSignalWrapper::sigFd[2];
bool UnixSignalWrapper::isHandlersInited = false;

UnixSignalWrapper::UnixSignalWrapper(QObject *parent) : QObject(parent)
{
    if (!isHandlersInited)
        initSignalHandlers();

    snUnixSignal = new QSocketNotifier(sigFd[1], QSocketNotifier::Read, this);
    connect(snUnixSignal, &QSocketNotifier::activated,
            this, &UnixSignalWrapper::handleUnixSignal);
}

void UnixSignalWrapper::initSignalHandlers()
{
    // Create socket pair and notifiers
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigFd))
    {
        qFatal("Couldn't create HUP socketpair");
        return;
    }

    struct sigaction action;
    memset(&action, 0, sizeof(action));

    action.sa_handler = unixSignalHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags |= SA_RESTART;
    if (sigaction(SIGHUP, &action, 0) < 0)
        qFatal("Failed to install SIGHUP handler");

    if (sigaction(SIGTERM, &action, 0) < 0)
        qFatal("Failed to install SIGTERM handler");

    if (sigaction(SIGINT, &action, 0) < 0)
        qFatal("Failed to install SIGINT handler");

    isHandlersInited = true;
}

void UnixSignalWrapper::handleUnixSignal()
{
    snUnixSignal->setEnabled(false);

    char tmp;
    ::read(sigFd[1], &tmp, sizeof(tmp));

    emit unixSignalReceived();

    snUnixSignal->setEnabled(true);
}

void UnixSignalWrapper::unixSignalHandler(int sigNum)
{
    char a = 1;
    ::write(sigFd[0], &a, sizeof(a));
}


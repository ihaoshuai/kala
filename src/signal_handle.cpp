#include <csignal>

#include "signal_handle.h"

void registerSignal(int sig, SignalHandleFunc fun)
{
    struct sigaction sa{};

    sa.sa_handler = fun;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(sig, &sa, nullptr);
}

void sendSignal(int pid, int sig)
{
    kill(pid, sig);
}

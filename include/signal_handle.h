#pragma once

using SignalHandleFunc = void(*)(int);

void registerSignal(int sig, SignalHandleFunc fun);
void sendSignal(int pid, int sig);

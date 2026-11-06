#include "lockfile.h"
#include "save.h"
#include "share_memory.h"
#include "signal_handle.h"
#include "timer.h"
#include <chrono>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>


const char* EXE_NAME = "kala";
const char* SAVE_PATH = "/home/booe/.local/share/kala/save";


using CommandFunc = int(*)(int, char**);

struct Command
{
    const char* s;
    CommandFunc f;
};

int command_start(int, char*[]);
int command_toggle(int, char*[]);
int command_reset(int, char*[]);
int command_elapsed(int, char*[]);
int command_exit(int, char*[]);

static const size_t COMMANDS_NUM = 5;
static const Command COMMANDS[] = {
    {"start", command_start},
    {"toggle", command_toggle},
    {"reset", command_reset},
    {"elapsed", command_elapsed},
    {"exit", command_exit}
};


static Timer timer{0.0, true};

/**
    Command: 
        kala [option]
    option:
        start
        toggle
        reset
        elapsed
        exit
*/
int main(int argc, char* argv[])
{
    if(argc < 2) {
        fprintf(stderr, "ERROR: lack option");
        return 1;
    }

    for(size_t i=0; i<COMMANDS_NUM; i++)
    {
        if(strcmp(argv[1], COMMANDS[i].s) == 0)
        {
            return COMMANDS[i].f(argc, argv);
        }
    }

    fprintf(stderr, "ERROR: no such command");
    return 1;
}

void overwrite(const std::string& text) {
    std::cout << "\r\033[2K" << text << std::flush;
}


bool done;

void finishProgram(int sig)
{
    done = true;
}

void toggleTimer(int sig)
{
    if(timer.isPause()) 
        timer.resume();
    else
        timer.pause();
}


void resetTimer(int sig)
{
    timer.reset();
}


int command_start(int argc, char** argv)
{
    if(!createAndHoldLockFile(EXE_NAME))
    {
        fprintf(stderr, "ERROR: failed to hold lock file");
        return 1;
    }

    if(createShm() != 0)
    {
        fprintf(stderr, "ERROR: failed to create Share Memory");
        return 1;
    }
    

    registerSignal(SIGINT, finishProgram);
    registerSignal(SIGTERM, finishProgram);
    registerSignal(SIGUSR1, toggleTimer);
    registerSignal(SIGUSR2, resetTimer);


    Save save;
    if(std::filesystem::exists(SAVE_PATH))
        save.load_from_file(SAVE_PATH);
    done = false;
    const auto interval = std::chrono::milliseconds(1000);
    if(argc > 2 && strcmp(argv[2], "-s")==0)
        timer = Timer(save.elapsed_time, true);
    else
        timer = Timer(save.elapsed_time, false);

    auto next = std::chrono::steady_clock::now();
    while(!done)
    {
        next += interval;
        const char* s = timer.display();
        overwrite(s);
        size_t pos = 0;
        pos += writeShm(s, pos);
        if(timer.isPause())
            writeShm("OFF", pos);
        else
            writeShm("ON", pos);
        std::this_thread::sleep_until(next);
    }
    printf("\n");
    save.elapsed_time = timer.elapsed();
    save.to_file(SAVE_PATH);
    releaseShm();
    releaseLockFile();
    return 0;
}



int command_toggle(int, char*[])
{
    int pid = loadLockFile(EXE_NAME);
    if(pid <= 0) {
        fprintf(stderr, "ERROR: no kala program instance running\n");
        return 1;
    }
    sendSignal(pid, SIGUSR1);
    return 0;
}

int command_reset(int, char*[])
{
    int pid = loadLockFile(EXE_NAME);
    if(pid <= 0) {
        fprintf(stderr, "ERROR: no kala program instance running\n");
        return 1;
    }
    sendSignal(pid, SIGUSR2);
    return 0;
}

int command_elapsed(int, char*[])
{
    int pid = loadLockFile(EXE_NAME);
    if(pid <= 0) 
    {
        fprintf(stderr, "ERROR: no kala program instance running\n");
        return 1;
    }
    std::optional<std::string> time, status;
    time = readShm(0);

    if(time == std::nullopt)
    {
        fprintf(stderr, "ERROR: failed to get time\n");
        return 1;
    }
    status = readShm(time->length() + 1);
    if(!status) {
        fprintf(stderr, "ERROR: failed to get status\n");
        return 1;
    }
    printf("%s %s\n", status->c_str(), time->c_str());
    return 0;


}

int command_exit(int, char*[])
{
    int pid = loadLockFile(EXE_NAME);
    if(pid <= 0) {
        fprintf(stderr, "ERROR: no kala program instance running");
        return 1;
    }
    sendSignal(pid, SIGTERM);
    return 0;
}


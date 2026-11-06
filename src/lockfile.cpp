#include "lockfile.h"
#include <charconv>
#include <cstdio>
#include <optional>
#include <string>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

static int fd = -1;

static std::optional<int> parse_pid(const char* buf, size_t len) {
    if (len == 0) return std::nullopt;
    int value = 0;
    auto [ptr, ec] = std::from_chars(buf, buf + len, value);
    if (ec != std::errc{} || ptr != buf + len)
        return std::nullopt;   // 解析失败或有残留字符
    return value;
}

bool createAndHoldLockFile(const std::string& name)
{
    auto path = "/tmp/" + name + ".lock";
    fd = open(path.c_str(), O_RDWR | O_CREAT, 0644);
    if(fd < 0)
        return false;
    if(flock(fd, LOCK_EX | LOCK_NB) != 0)
    {
        close(fd);
        fd = -1;
        return false;
    }
    ftruncate(fd, 0);
    char buf[32];
    int n = std::snprintf(buf, sizeof(buf), "%d", ::getpid());
    if (n > 0) {
        ::write(fd, buf, static_cast<size_t>(n));
    }
    return true;
}


int loadLockFile(const std::string& name)
{
    auto path = "/tmp/" + name + ".lock";
    int fd = ::open(path.c_str(), O_RDONLY);
    int pid = -1;
    if (fd >= 0) {
        char buf[64];
        ssize_t n = ::read(fd, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            if (auto v = parse_pid(buf, static_cast<size_t>(n)))
                pid = *v;
        }
        ::close(fd);
    }
    return pid;
}


void releaseLockFile()
{
    if (fd >= 0) {
        ftruncate(fd, 0);

        struct flock fl {};
        fl.l_type   = F_UNLCK;
        fl.l_whence = SEEK_SET;
        fl.l_start  = 0;
        fl.l_len    = 0;
        ::fcntl(fd, F_SETLK, &fl);
        ::close(fd);
    }
}
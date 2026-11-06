#include "share_memory.h"
#include <cstddef>
#include <optional>
#include <string>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>


static int fd = -1;
static const char* SHM_NAME = "/kala_shm";
static const size_t SHM_SIZE = 1024;
void* ptr = nullptr;


int createShm()
{
    fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if(fd == -1) {
        perror("shm_open");
        return 1;
    }
    if(ftruncate(fd, SHM_SIZE) == -1) {
        perror("ftruncate");
        return 1;
    }
    ptr = mmap(nullptr, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(ptr == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    return 0;
}

std::optional<std::string> readShm(size_t pos) 
{
    if(pos >= SHM_SIZE)
        return std::nullopt;
    int fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (fd == -1) { perror("shm_open"); return std::nullopt; }

    void* ptr = mmap(nullptr, SHM_SIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) { perror("mmap"); return std::nullopt; }

    std::string result(static_cast<char*>(ptr) + pos);
    munmap(ptr, SHM_SIZE);
    close(fd);
    return result;
}

size_t writeShm(const char* data, size_t pos)
{
    auto n = std::strlen(data) + 1;
    if(pos + n > SHM_SIZE)
        return 0;
    auto p = (char*)ptr + pos;
    std::memcpy(p, data, n);
    return n;
}

void releaseShm()
{
    if(ptr != nullptr)
        munmap(ptr, SHM_SIZE);
    if(fd != -1)
        close(fd);

    shm_unlink(SHM_NAME);
}

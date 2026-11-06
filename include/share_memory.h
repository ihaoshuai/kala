#pragma once


#include <cstddef>
#include <optional>
#include <string>



int createShm();
std::optional<std::string> readShm(size_t pos);
size_t writeShm(const char* data, size_t pos);
void releaseShm();

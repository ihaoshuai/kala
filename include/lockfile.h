#pragma once

#include <string>

bool createAndHoldLockFile(const std::string& name);
int loadLockFile(const std::string& name);
void releaseLockFile();
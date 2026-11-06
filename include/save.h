#pragma once

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <ios>

namespace fs = std::filesystem;

class Save
{
public:
    double elapsed_time;

    Save()
        :elapsed_time(0.0)
    {
    };
    
    bool load_from_file(const fs::path& save_path)
    {
        if(!fs::exists(save_path))
            return false;
        std::ifstream save_file(save_path, std::ios::binary);
        if(!save_file.is_open())
            return false;
        save_file.read(reinterpret_cast<char*>(this), sizeof(Save));
        save_file.close();
        return true;
    }

    bool to_file(const fs::path& save_path)
    {
        if(save_path.has_parent_path() && !fs::exists(save_path.parent_path())) {
            try {
                fs::create_directories(save_path.parent_path());
            }    
            catch (const std::filesystem::filesystem_error& e) {
                fprintf(stderr, "ERROR:[create dirs] %s\n", e.what());
                return false;
            }
        }
        std::ofstream save_file(save_path, std::ios::binary);
        if(!save_file.is_open())
            return false;
        save_file.write(reinterpret_cast<const char*>(this), sizeof(Save));
        save_file.close();
        return true;
    }
};

#pragma once
#include <fstream>
#include <string>

class fileReader
{
private:
    fileReader();
    ~fileReader();
    static std::string readString(std::ifstream*);
public:
    static void createFile(std::string);
};
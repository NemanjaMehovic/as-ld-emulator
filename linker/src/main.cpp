#include <iostream>
#include <string>
#include <cstring>
#include <regex>
#include "fileReader.h"
#include "linker.h"
#include "file.h"


bool checkPlace(std::string argument)
{
    return std::regex_match(argument, std::regex("-place=[_a-zA-Z][_0-9a-zA-Z]*@0[xX][0-9a-fA-F]+"));
}

std::pair<std::string, unsigned>* getValue(std::string argument)
{
    std::smatch match;
    std::regex_search(argument, match, std::regex("-place=([_a-zA-Z][_0-9a-zA-Z]*)@(0[xX][0-9a-fA-F]+)"));
    std::string section = match[1];
    unsigned location =  std::stoul(match[2], nullptr, 16);
    return new std::pair<std::string, unsigned>(section, location);
}

int main(int argc, char const *argv[])
{
    int i;
    std::string type = "";
    std::string out = "out";
    std::vector<std::pair<std::string, unsigned>*>* place = new std::vector<std::pair<std::string, unsigned>*>();
    for(i = 1; i < argc ; i++)
    {
        if(strcmp(argv[i], "-o") == 0)
        {
            if((i + 1) >= (argc - 1))
            {
                std::cout << "Not enough arguments" << std::endl;
                return 0;
            }
            out = argv[++i];
        }
        else if(strcmp(argv[i], "-hex") == 0)
        {
            if(type == "")
                type = argv[i];
            else
            {
                std::cout << "-hex or -linkable can only be used once" << std::endl;
                return 0;
            }
        }
        else if(strcmp(argv[i], "-linkable") == 0)
        {
            if(type == "")
                type = argv[i];
            else
            {
                std::cout << "-hex or -linkable can only be used once" << std::endl;
                return 0;
            }
        }
        else if(checkPlace(argv[i]))
            place->push_back(getValue(argv[i]));
        else
            break;
    }
    if(i == argc)
    {
        std::cout<< "No files given" << std::endl;
        return 0;
    }
    if(type == "")
    {
        std::cout<< "One of -hex or -linkable is mandatory" << std::endl;
        return 0;
    }
    for(; i < argc; i++)
        fileReader::createFile(argv[i]);
    linker::instance()->resolveSymbols();
    if(type == "-linkable")
    {
        linker::instance()->linkable(out);
        linker::instance()->linkableBin(out);
    }
    else
    {
        linker::instance()->setPlace(place);
        linker::instance()->fixHexValues();
        linker::instance()->hex(out);
        linker::instance()->hexBin(out);
    }
    linker::deleteInstance();
    return 0;
}

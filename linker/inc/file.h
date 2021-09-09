#pragma once
#include <vector>
#include <string>
#include "symbol.h"

class file
{
private:
    std::string name;
    std::vector<symbol*>* table;
    std::vector<section*>* sections;
public:
    file(std::string);
    ~file();
    symbol* findSymbol(std::string);
    section* findSection(std::string);
    std::vector<symbol*>* getTable();
    std::vector<section*>* getSections();
    void addSymbol(symbol*);
    void addSection(section*);
    std::string getName();
};
#include <vector>
#include <string>
#include "file.h"
#include "symbol.h"



file::file(std::string fname):name(fname)
{
    table = new std::vector<symbol*>();
    sections = new std::vector<section*>();
}

file::~file()
{
    for(auto tmp:*table)
        delete tmp;
    delete table;
    delete sections;
}

symbol* file::findSymbol(std::string name)
{
    for(auto tmp:*table)
        if(tmp->getName() == name)
            return tmp;
    return nullptr;
}

section* file::findSection(std::string name) 
{
    for(auto tmp:*sections)
        if(tmp->getName() == name)
            return tmp;
    return nullptr;
}

std::vector<symbol*>* file::getTable()
{
    return table;
}

std::vector<section*>* file::getSections()
{
    return sections;
}

void file::addSymbol(symbol* s)
{
    table->push_back(s);
}

void file::addSection(section* s)
{
    addSymbol(s);
    sections->push_back(s);
}

std::string file::getName()
{
    return name;
}
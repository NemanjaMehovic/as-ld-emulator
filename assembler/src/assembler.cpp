#include <vector>
#include <algorithm>
#include "assembler.h"
#include "pass.h"

assembler* assembler::singleton = nullptr;

assembler* assembler::instance()
{
    if(singleton == nullptr)
        singleton = new assembler();
    return singleton;
}

void assembler::deleteInstance()
{
    if(singleton != nullptr)
    {
        delete singleton;
        singleton = nullptr;
    }
}

assembler::assembler()
{
    methods = new firstPass();
    table = new std::vector<symbol*>();
    sections = new std::vector<section*>();
    sections->push_back(new section("E", "E", 0, -2, true));
    sections->push_back(new section("A", "A", 0, -1, true));
    currSection = new section("U", "U", 0, 0, true);
    sections->push_back(currSection);
}

assembler::~assembler()
{
    delete methods;
    for(auto tmp:*table)
        delete tmp;
    delete table;
    for(int i = 0; i < 3; i++)
        delete (*sections)[i];
    delete sections;
}

pass* assembler::getRules()
{
    return methods;
}

void assembler::setRules(pass* r)
{
    methods = r;
}

std::vector<symbol*>* assembler::getTable()
{
    return table;
}

std::vector<section*>* assembler::getSections()
{
    return sections;
}

section* assembler::getCurrSection()
{
    return currSection;
}

void assembler::setCurrSection(section* s)
{
    currSection = s;
}

symbol* assembler::findSymbol(std::string name)
{
    for(auto tmp:*table)
        if(tmp->getName() == name)
            return tmp;
    return nullptr;
}

section* assembler::findSection(std::string name) 
{
    for(auto tmp:*sections)
        if(tmp->getName() == name)
            return tmp;
    return nullptr;
}
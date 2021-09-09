#pragma once
#include <string>
#include <vector>
#include "pass.h"
#include "symbol.h"

class pass;

class assembler
{
private:
    static assembler* singleton;
    std::vector<symbol*>* table;
    std::vector<section*>* sections;
    section* currSection;
    pass* methods;
    assembler();
    ~assembler();
public:
    static assembler* instance();
    static void deleteInstance();
    pass* getRules();
    void setRules(pass*);
    std::vector<symbol*>* getTable();
    std::vector<section*>* getSections();
    section* getCurrSection();
    void setCurrSection(section*);
    symbol* findSymbol(std::string);
    section* findSection(std::string);
};

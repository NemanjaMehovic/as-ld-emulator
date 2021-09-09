#include <string>
#include <vector>
#include "symbol.h"
#include "instruction.h"

symbol::symbol(std::string n, std::string s, int v, int i, bool l):name(n), section(s), val(v), id(i), local(l){}

symbol::~symbol(){}

int symbol::getId()
{
    return id;
}

int symbol::getVal()
{
    return val;
}

std::string symbol::getName()
{
    return name;
}

std::string symbol::getSection()
{
    return section;
}

bool symbol::isLocal()
{
    return local;
}

void symbol::setLocal(bool l)
{
    local = l;
}

section::section(std::string n, std::string s, int v, int i, bool l):symbol(n, s, v, i, l)
{
    locationCounter = 0;
    inst = new std::vector<int8_t>();
    relData = new std::vector<rel*>();
}

section::~section()
{
    for(auto tmp:*relData)
        delete tmp;
    delete relData;
    delete inst;
}

void section::increaseCounter(int i)
{
    locationCounter += i;
}

void section::resetCounter()
{
    locationCounter = 0;
}

int section::getCounter()
{
    return locationCounter;
}

void section::addByte(int8_t byte)
{
    inst->push_back(byte);
}

void section::addPayload(int val)
{
    int8_t l = val & 0b11111111;
    int8_t h = (val >> 8) & 0b11111111;
    inst->push_back(l);
    inst->push_back(h);
}

void section::insertRel(rel* r)
{
    relData->push_back(r);
}

std::vector<int8_t>* section::getBytes()
{
    return inst;
}

std::vector<rel*>* section::getRel()
{
    return relData;
}
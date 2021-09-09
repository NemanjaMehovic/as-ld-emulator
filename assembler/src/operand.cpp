#include <string>
#include "operand.h"

operand::operand(int s, uint8_t r, uint8_t a, int l, std::string sym):size(s), regDesc(r), adrMode(a), literal(l), symbol(sym) {}

int operand::getSize()
{
    return size;
}

int operand::getLiteral()
{
    return literal;
}

uint8_t operand::getRegDesc()
{
    return regDesc;
}

uint8_t operand::getAdrMode()
{
    return adrMode;
}

std::string operand::getSymbol()
{
    return symbol;
}
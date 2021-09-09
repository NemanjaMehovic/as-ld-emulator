#pragma once
#include <string>

class operand
{
private:
    uint8_t regDesc;
    uint8_t adrMode;
    int literal;
    std::string symbol;
    int size;
public:
    operand(int, uint8_t, uint8_t, int, std::string);
    int getSize();
    uint8_t getRegDesc();
    uint8_t getAdrMode();
    int getLiteral();
    std::string getSymbol();
};
#pragma once
#include <unordered_map>
#include <string>
#include "operand.h"

#define ABS "R_X86_64_32"
#define REL "R_X86_64_PC32"

class instruction
{
private:
    static std::unordered_map<std::string, int8_t>* instructionMap;
    operand* op;
    int8_t code;
public:
    instruction(std::string, operand*);
    ~instruction();
    int getSize();
    int8_t getCode();
    std::string getAdrMode();
    bool hasSymbol();
    operand* getOperand();
};

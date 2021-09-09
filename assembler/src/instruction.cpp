#include <unordered_map>
#include <string>
#include <iomanip>
#include <sstream>
#include <iostream>
#include "assembler.h"
#include "instruction.h"
#include "operand.h"
#include "symbol.h"

using std::string;
using std::unordered_map;


unordered_map<string, int8_t>* instruction::instructionMap = new unordered_map<string, int8_t>({
    //no op
    {"halt", 0b00000000},
    {"iret", 0b00100000},
    {"ret", 0b01000000},
    //jumps
    {"call", 0b00110000},
    {"jmp", 0b01010000},
    {"jeq", 0b01010001},
    {"jne", 0b01010010},
    {"jgt", 0b01010011},
    //one reg
    {"int", 0b00010000},
    {"push", 0b10110000},
    {"pop", 0b10100000},
    {"not", 0b10000000},
    //two reg
    {"xchg", 0b01100000},
    {"add", 0b01110000},
    {"sub", 0b01110001},
    {"mul", 0b01110010},
    {"div", 0b01110011},
    {"cmp", 0b01110100},
    {"and", 0b10000001},
    {"or", 0b10000010},
    {"xor", 0b10000011},
    {"test", 0b10000100},
    {"shl", 0b10010000},
    {"shr", 0b10010001},
    //mem
    {"ldr", 0b10100000},
    {"str", 0b10110000}
});

instruction::instruction(std::string inst, operand* op)
{
    this->code = instructionMap->find(inst)->second;
    this->op = op;
}

instruction::~instruction()
{
    delete op;
}

int instruction::getSize()
{
    return 1 + op->getSize();
}

int8_t instruction::getCode()
{
    return code;
}

operand* instruction::getOperand()
{
    return op;
}

bool instruction::hasSymbol()
{
    return op->getSymbol() != "";
}

std::string instruction::getAdrMode()
{
    uint8_t tmpAdrDesc = op->getAdrMode() & 0b1111;
    uint8_t tmpRegDesc = op->getRegDesc() & 0b1111;
    if(tmpAdrDesc == 3 && tmpRegDesc == 7)
        return REL;
    return ABS;
}
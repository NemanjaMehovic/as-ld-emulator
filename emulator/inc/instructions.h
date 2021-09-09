#pragma once

//no adr
#define haltOp 0b00000000
#define iretOp 0b00100000
#define retOp 0b01000000
//jumps
#define callOp 0b00110000
#define jmpOp 0b01010000
#define jeqOp 0b01010001
#define jneOp 0b01010010
#define jgtOp 0b01010011
//one reg
#define intOp 0b00010000
#define notOp 0b10000000
//two reg
#define xchgOp 0b01100000
#define addOp 0b01110000
#define subOp 0b01110001
#define mulOp 0b01110010
#define divOp 0b01110011
#define cmpOp 0b01110100
#define andOp 0b10000001
#define orOp 0b10000010
#define xorOp 0b10000011
#define testOp 0b10000100
#define shlOp 0b10010000
#define shrOp 0b10010001
//mem
#define loadOp 0b10100000
#define storeOp 0b10110000

#define imm 0b0000
#define regDir 0b0001
#define regInDir 0b0010
#define regInDirAdd 0b0011
#define regDirAdd 0b0101
#define memDirect 0b0100

#define noUpdate 0b00000000
#define subBefore 0b00010000
#define addBefore 0b00100000
#define subAfter 0b00110000
#define addAfter 0b01000000
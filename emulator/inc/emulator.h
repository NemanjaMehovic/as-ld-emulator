#pragma once
#include <cstdint>
#include <string>
#include <map>
#include <mutex>

#define MEM_SIZE 1 << 16
#define NUM_OF_REG 9
#define sp 6
#define pc 7
#define psw 8 



class emulator
{
private:
    typedef void (emulator::*MFP)();
    static std::map<uint8_t, MFP>* methodsMap;
    static emulator* em;
    uint8_t* mem;
    uint16_t* reg;
    std::mutex memMutex;
    bool running;
    bool terminalInt;
    bool timerInt;
    bool badOpCodeInt;
    emulator();
    ~emulator();
    uint16_t getWord(uint8_t, uint8_t);
    uint16_t popWord();
    void pushWord(uint16_t);
    void getReg(uint8_t, uint8_t&, uint8_t&);
    void setPswZ(bool);
    void setPswO(bool);
    void setPswC(bool);
    void setPswN(bool);
    void setPswTr(bool);
    void setPswTi(bool);
    void setPswI(bool);
    uint16_t* resolveAddr(uint8_t, uint16_t&, bool);
    bool resolveU(uint8_t, uint16_t&, bool);
    void resolveInterrupt();
public:
    static emulator* instance();
    static void deleteInstance();
    void loadMem(std::string);
    void start();
    void readByte(uint8_t&);
    void haltInst();
    void iretInst();
    void retInst();
    void intInst();
    void notInst();
    void xchgInst();
    void addInst();
    void subInst();
    void mulInst();
    void divInst();
    void cmpInst();
    void andInst();
    void orInst();
    void xorInst();
    void testInst();
    void shlInst();
    void shrInst();
    void loadInst();
    void storeInst();
    void callInst();
    void jmpInst();
    void jeqInst();
    void jneInst();
    void jgtInst();
    void timer();
    void terminal();
};

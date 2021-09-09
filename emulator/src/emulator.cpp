#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <thread>
#include <termios.h>
#include <unistd.h>
#include "emulator.h"
#include "instructions.h"

emulator* emulator::em = nullptr;

std::map<uint8_t, emulator::MFP>* emulator::methodsMap = new std::map<uint8_t, emulator::MFP>({
    {haltOp, &emulator::haltInst},
    {retOp, &emulator::retInst},
    {iretOp, &emulator::iretInst},
    {callOp, &emulator::callInst},
    {jmpOp, &emulator::jmpInst},
    {jeqOp, &emulator::jeqInst},
    {jneOp, &emulator::jneInst},
    {jgtOp, &emulator::jgtInst},
    {intOp, &emulator::intInst},
    {notOp, &emulator::notInst},
    {xchgOp, &emulator::xchgInst},
    {addOp, &emulator::addInst},
    {subOp, &emulator::subInst},
    {mulOp, &emulator::mulInst},
    {divOp, &emulator::divInst},
    {cmpOp, &emulator::cmpInst},
    {andOp, &emulator::andInst},
    {orOp, &emulator::orInst},
    {xorOp, &emulator::xorInst},
    {testOp, &emulator::testInst},
    {shlOp, &emulator::shlInst},
    {shrOp, &emulator::shrInst},
    {loadOp, &emulator::loadInst},
    {storeOp, &emulator::storeInst}
});

emulator::emulator()
{
    mem = new uint8_t[MEM_SIZE];
    reg = new uint16_t[NUM_OF_REG];
    running = false;
    badOpCodeInt = false;
    timerInt = false;
    terminalInt = false;
    reg[sp] = 0xFF00;
}

emulator::~emulator()
{
    delete[] mem;
    delete[] reg;
}

emulator* emulator::instance()
{
    if(em == nullptr)
        em = new emulator();
    return em;
}

void emulator::deleteInstance()
{
    delete em;
}

uint16_t emulator::getWord(uint8_t l, uint8_t h)
{
    uint16_t val = l & 0x00FF;
    val |= h << 8;
    return val;
}

uint16_t emulator::popWord()
{
    memMutex.lock();
    uint8_t l = mem[reg[sp]++];
    uint8_t h = mem[reg[sp]++];
    memMutex.unlock();
    return getWord(l, h);
}

void emulator::pushWord(uint16_t data)
{
    uint8_t l = data & 0x00FF;
    uint8_t h = (data >> 8) & 0x00FF;
    memMutex.lock();
    mem[--reg[sp]] = h;
    mem[--reg[sp]] = l;
    memMutex.unlock();
}

void emulator::getReg(uint8_t regDescrip, uint8_t& regD, uint8_t& regS)
{
    regS = regDescrip & 0x0F;
    regD = (regDescrip >> 4) & 0x0F;
}

void emulator::setPswZ(bool zer)
{
    static const uint16_t z = 1;
    if(zer)
        reg[psw] |= z;
    else 
        reg[psw] &= ~z; 
}

void emulator::setPswO(bool over)
{
    static const uint16_t o = 1 << 1;
    if(over)
        reg[psw] |= o;
    else 
        reg[psw] &= ~o; 
}

void emulator::setPswC(bool car)
{
    static const uint16_t c = 1 << 2;
    if(car)
        reg[psw] |= c;
    else 
        reg[psw] &= ~c; 
}

void emulator::setPswN(bool neg)
{
    static const uint16_t n = 1 << 3;
    if(neg)
        reg[psw] |= n;
    else 
        reg[psw] &= ~n; 
}

void emulator::setPswTr(bool timer)
{
    static const uint16_t t = 1 << 13;
    if(!timer)
        reg[psw] |= t;
    else 
        reg[psw] &= ~t; 
}

void emulator::setPswTi(bool ter)
{
    static const uint16_t t = 1 << 14;
    if(!ter)
        reg[psw] |= t;
    else 
        reg[psw] &= ~t; 
}

void emulator::setPswI(bool interr)
{
    static const uint16_t i = 1 << 15;
    if(!interr)
        reg[psw] |= i;
    else 
        reg[psw] &= ~i; 
}

void emulator::loadMem(std::string fileName)
{
    std::ifstream fileStream(fileName, std::ios::in | std::ios::binary);
    uint16_t location;
    while(fileStream.read((char*) &location, sizeof(location)))
    {
        for(int i = location; i < location + 8; i++)
        {
            uint8_t tmp;
            fileStream.read((char*) &tmp, sizeof(tmp));
            mem[i] = tmp;
        }
    }
    fileStream.close();
    reg[pc] = getWord(mem[0], mem[1]);
}

void emulator::start()
{
    uint8_t opCode;
    running = true;
    int i = 0;

    struct termios oldt;
    struct termios newt;
    tcgetattr( STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON);
    setvbuf(stdout, NULL, _IONBF, 0);
    newt.c_lflag ^= ECHO;
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);

    std::thread timerThread(&emulator::timer, this);
    std::thread terminalThread(&emulator::terminal, this);
    timerThread.detach();
    terminalThread.detach();
    while(running)
    {
        //printf("%04X:", reg[pc]);
        readByte(opCode);
        auto tmp = methodsMap->find(opCode);
        if(tmp == methodsMap->end())
            badOpCodeInt = true;
        else
        {
            //printf("%d: ",i);
            MFP fp = tmp->second;
            (this->*fp)();
        }
        //printf("\n");
        resolveInterrupt();
        i++;
    }
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
    std::cout<<std::endl;
}

void emulator::readByte(uint8_t& data)
{
    memMutex.lock();
    data = mem[reg[pc]];
    memMutex.unlock();
    reg[pc]++;
}

void emulator::haltInst()
{
    //printf("halt");
    running = false;
}

void emulator::retInst()
{
    //printf("ret");
    reg[pc] = popWord();
}

void emulator::iretInst()
{
    //printf("iret");
    reg[pc] = popWord();
    reg[psw] = popWord(); 
}

void emulator::intInst()
{
    //printf("int");
    uint8_t regDescrip;
    uint8_t regD;
    uint8_t regS;
    readByte(regDescrip);
    getReg(regDescrip, regD, regS);
    pushWord(reg[psw]);
    pushWord(reg[pc]);
    memMutex.lock();
    uint8_t l = mem[(reg[regD] % 8) * 2];
    uint8_t h = mem[(reg[regD] % 8) * 2 + 1];
    memMutex.unlock();
    reg[pc] = ((h << 8) & 0xFF00) | (l & 0x00FF);
}

void emulator::notInst()
{
    //printf("not");
    uint8_t regDescrip;
    uint8_t regD;
    uint8_t regS;
    readByte(regDescrip);
    getReg(regDescrip, regD, regS);
    reg[regD] = ~reg[regD];
}

void emulator::xchgInst()
{
    //printf("xchg");
    uint8_t regDescrip;
    uint8_t regD;
    uint8_t regS;
    readByte(regDescrip);
    getReg(regDescrip, regD, regS);
    uint16_t tmp = reg[regS];
    reg[regS] = reg[regD];
    reg[regD] = tmp;
}

void emulator::addInst()
{
    //printf("add");
    uint8_t regDescrip;
    uint8_t regD;
    uint8_t regS;
    readByte(regDescrip);
    getReg(regDescrip, regD, regS);
    reg[regD] = (int16_t)reg[regD] + (int16_t)reg[regS];
}

void emulator::subInst()
{
    //printf("sub");
    uint8_t regDescrip;
    uint8_t regD;
    uint8_t regS;
    readByte(regDescrip);
    getReg(regDescrip, regD, regS);
    reg[regD] = (int16_t)reg[regD] - (int16_t)reg[regS];
}

void emulator::mulInst()
{
    //printf("mul");
    uint8_t regDescrip;
    uint8_t regD;
    uint8_t regS;
    readByte(regDescrip);
    getReg(regDescrip, regD, regS);
    reg[regD] = (int16_t)reg[regD] * (int16_t)reg[regS];
}

void emulator::divInst()
{
    //printf("div");
    uint8_t regDescrip;
    uint8_t regD;
    uint8_t regS;
    readByte(regDescrip);
    getReg(regDescrip, regD, regS);
    reg[regD] = (int16_t)reg[regD] / (int16_t)reg[regS];
}

void emulator::cmpInst()
{
    ////printf("cmp");
    uint8_t regDescrip;
    uint8_t regD;
    uint8_t regS;
    readByte(regDescrip);
    getReg(regDescrip, regD, regS);
    int16_t tmp = (int16_t)reg[regD] - (int16_t)reg[regS];
    setPswZ(tmp == 0);
    setPswN(tmp < 0);
    setPswC(reg[regD] < reg[regS]);
    int16_t tmpD = reg[regD];
    int16_t tmpS = reg[regS];
    setPswO((tmp > 0 && tmpD < 0 && tmpS > 0) || (tmp < 0 && tmpD > 0 && tmpS < 0));
}

void emulator::andInst()
{
    //printf("and");
    uint8_t regDescrip;
    uint8_t regD;
    uint8_t regS;
    readByte(regDescrip);
    getReg(regDescrip, regD, regS);
    reg[regD] &= reg[regS];
}

void emulator::orInst()
{
    //printf("or");
    uint8_t regDescrip;
    uint8_t regD;
    uint8_t regS;
    readByte(regDescrip);
    getReg(regDescrip, regD, regS);
    reg[regD] |= reg[regS];
}

void emulator::xorInst()
{
    //printf("xor");
    uint8_t regDescrip;
    uint8_t regD;
    uint8_t regS;
    readByte(regDescrip);
    getReg(regDescrip, regD, regS);
    reg[regD] ^= reg[regS];
}

void emulator::testInst()
{
    //printf("test");
    uint8_t regDescrip;
    uint8_t regD;
    uint8_t regS;
    readByte(regDescrip);
    getReg(regDescrip, regD, regS);
    int16_t tmp = reg[regD] & reg[regS];
    setPswZ(tmp == 0);
    setPswN(tmp < 0);
}

void emulator::shlInst()
{
    //printf("shl");
    uint8_t regDescrip;
    uint8_t regD;
    uint8_t regS;
    readByte(regDescrip);
    getReg(regDescrip, regD, regS);
    int16_t tmpD = reg[regD];
    reg[regD] <<= reg[regS];
    setPswZ(reg[regD] == 0);
    setPswN(((int16_t)reg[regD]) < 0);
    int flag = 0;
    for(int i = 0; i < reg[regS]; i++)
    {
        flag = ((tmpD & 0x8000) >> 15) & 1;
        tmpD <<= 1;
    }
    setPswC(flag == 1);
}

void emulator::shrInst()
{
    //printf("shr");
    uint8_t regDescrip;
    uint8_t regD;
    uint8_t regS;
    readByte(regDescrip);
    getReg(regDescrip, regD, regS);
    int16_t tmpD = reg[regD];
    reg[regD] >>= reg[regS];
    setPswZ(reg[regD] == 0);
    setPswN(((int16_t)reg[regD]) < 0);
    int flag = 0;
    for(int i = 0; i < reg[regS]; i++)
    {
        flag = tmpD & 1;
        tmpD >>= 1;
    }
    setPswC(flag == 1);
}

uint16_t* emulator::resolveAddr(uint8_t addrDescrip, uint16_t& regS, bool firstCall)
{
    uint8_t tmpAddrDescrip = addrDescrip & 0x0F;
    bool flag = true;
    static bool deleteOnNextCall = false;
    static uint16_t* operand = nullptr;
    if(deleteOnNextCall)
    {
        delete operand;
        operand = nullptr;
        deleteOnNextCall = false;
    }
    switch (tmpAddrDescrip)
    {
    case imm:
        if(firstCall)
        {
            uint8_t l;
            uint8_t h;
            readByte(l);
            readByte(h);
            operand = new uint16_t;
            *operand = l & 0x00FF;
            *operand |= (h << 8) & 0xFF00;
            deleteOnNextCall = true;
        }
        break;
    case regDir:
        flag = resolveU(addrDescrip, regS, firstCall);
        if(!flag)
        {
            operand = nullptr;
            break;
        }
        if(firstCall)
            operand = &regS;
        break;
    case regInDir:
        flag = resolveU(addrDescrip, regS, firstCall);
        if(!flag)
        {
            operand = nullptr;
            break;
        }
        if(firstCall)
        {
            memMutex.lock();
            uint8_t l = mem[regS];
            uint8_t h = mem[regS + 1];
            memMutex.unlock();
            operand = new uint16_t;
            *operand = l & 0x00FF;
            *operand |= (h << 8) & 0xFF00;
            deleteOnNextCall = true;
        }
        break;
    case regDirAdd:
        flag = resolveU(addrDescrip, regS, firstCall);
        if(!flag)
        {
            operand = nullptr;
            break;
        }
        if(firstCall)
        {
            uint8_t l;
            uint8_t h;
            readByte(l);
            readByte(h);
            int16_t append = l & 0x00FF;
            append |= (h << 8) & 0xFF00;
            operand = new uint16_t;
            *operand = (int16_t)regS + append;
            deleteOnNextCall = true;
        }
        break;
    case regInDirAdd:
        flag = resolveU(addrDescrip, regS, firstCall);
        if(!flag)
        {
            operand = nullptr;
            break;
        }
        if(firstCall)
        {
            uint8_t l;
            uint8_t h;
            readByte(l);
            readByte(h);
            int16_t append = l & 0x00FF;
            append |= (h << 8) & 0xFF00;
            append = (int16_t)regS + append;
            memMutex.lock();
            l = mem[(uint16_t)append];
            h = mem[(uint16_t)append + 1];
            memMutex.unlock();
            operand = new uint16_t;
            *operand = l & 0x00FF;
            *operand |= (h << 8) & 0xFF00;
            deleteOnNextCall = true;
        }
        break;
    case memDirect:
        if(firstCall)
        {
            uint8_t l;
            uint8_t h;
            readByte(l);
            readByte(h);
            uint16_t tmpAddr = l & 0x00FF;
            tmpAddr |= (h << 8) & 0xFF00;
            memMutex.lock();
            l = mem[tmpAddr];
            h = mem[tmpAddr + 1];
            memMutex.unlock();
            operand = new uint16_t;
            *operand = l & 0x00FF;
            *operand |= (h << 8) & 0xFF00;
            deleteOnNextCall = true;
        }
        break;
    default:
        badOpCodeInt = true;
        operand = nullptr;
        break;
    }
    return operand;
}

bool emulator::resolveU(uint8_t addrDescrip, uint16_t& reg, bool firstCall)
{
    uint8_t tmpAddrDescrip = addrDescrip & 0xF0;
    switch (tmpAddrDescrip)
    {
    case noUpdate:
        break;
    case addAfter:
        if(!firstCall)
            reg += 2;
        break;
    case addBefore:
        if(firstCall)
            reg += 2;
        break;
    case subAfter:
        if(!firstCall)
            reg -= 2;
        break;
    case subBefore:
        if(firstCall)
            reg -= 2;
        break;
    default:
        badOpCodeInt = true;
        return false;
        break;
    }
    return true;
}

void emulator::loadInst()
{
    //printf("load");
    uint8_t regDescrip;
    uint8_t addrDescrip;
    readByte(regDescrip);
    uint8_t regD;
    uint8_t regS;
    getReg(regDescrip, regD, regS);
    readByte(addrDescrip);
    uint16_t* operand = resolveAddr(addrDescrip, reg[regS], true);
    if(operand == nullptr)
        return;
    reg[regD] = *operand;
    resolveAddr(addrDescrip, reg[regS], false);
}

void emulator::storeInst()
{
    //printf("store");
    uint8_t regDescrip;
    uint8_t addrDescrip;
    readByte(regDescrip);
    uint8_t regD;
    uint8_t regS;
    getReg(regDescrip, regD, regS);
    readByte(addrDescrip);
    uint16_t tmpAddrDescrip = addrDescrip & 0x0F;
    bool flag = false;
    int16_t operand;
    uint8_t h;
    uint8_t l;
    switch (tmpAddrDescrip)
    {
    case imm:
        badOpCodeInt = true;
        break;
    case regDir:
        flag = resolveU(addrDescrip, reg[regS], true);
        if(!flag)
            break;
        reg[regS] = reg[regD];
        flag = resolveU(addrDescrip, reg[regS], false);
        break;
    case regInDir:
        flag = resolveU(addrDescrip, reg[regS], true);
        if(!flag)
            break;
        l = reg[regD] & 0x00FF;
        h = (reg[regD] >> 8) & 0x00FF;
        memMutex.lock();
        if((uint16_t)reg[regS] == 0xFF00)
            std::cout << (char)reg[regD];
        mem[reg[regS]] = l;
        mem[reg[regS] + 1] = h;
        memMutex.unlock();
        flag = resolveU(addrDescrip, reg[regS], false);
        break;
    case regDirAdd:
        badOpCodeInt = true;
        break;
    case regInDirAdd:
        flag = resolveU(addrDescrip, reg[regS], true);
        if(!flag)
            break;
        readByte(l);
        readByte(h);
        operand = l & 0x00FF;
        operand |= (h << 8) & 0xFF00;
        operand = (int16_t)reg[regS] + operand;
        l = reg[regD] & 0x00FF;
        h = (reg[regD] >> 8) & 0x00FF;
        memMutex.lock();
        if((uint16_t)operand == 0xFF00)
            std::cout << (char)reg[regD];
        mem[(uint16_t)operand] = l;
        mem[(uint16_t)operand + 1] = h;
        memMutex.unlock();
        flag = resolveU(addrDescrip, reg[regS], false);
        break;
    case memDirect:
        readByte(l);
        readByte(h);
        operand = l & 0x00FF;
        operand |= (h << 8) & 0xFF00;
        l = reg[regD] & 0x00FF;
        h = (reg[regD] >> 8) & 0x00FF;
        memMutex.lock();
        if((uint16_t)operand == 0xFF00)
            std::cout << (char)reg[regD];
        mem[operand] = l;
        mem[operand + 1] = h;
        memMutex.unlock();
        break;
    default:
        badOpCodeInt = true;
        break;
    }
}

void emulator::callInst()
{
    //printf("call");
    uint8_t regDescrip;
    uint8_t addrDescrip;
    readByte(regDescrip);
    uint8_t regD;
    uint8_t regS;
    getReg(regDescrip, regD, regS);
    readByte(addrDescrip);
    uint16_t* operand = resolveAddr(addrDescrip, reg[regS], true);
    if(operand == nullptr)
        return;
    pushWord(reg[pc]);
    reg[pc] = *operand;
    resolveAddr(addrDescrip, reg[regS], false);
}

void emulator::jmpInst()
{
    //printf("jmp");
    uint8_t regDescrip;
    uint8_t addrDescrip;
    readByte(regDescrip);
    uint8_t regD;
    uint8_t regS;
    getReg(regDescrip, regD, regS);
    readByte(addrDescrip);
    uint16_t* operand = resolveAddr(addrDescrip, reg[regS], true);
    if(operand == nullptr)
        return;
    reg[pc] = *operand;
    resolveAddr(addrDescrip, reg[regS], false);
}

void emulator::jeqInst()
{
    //printf("jeq");
    uint8_t regDescrip;
    uint8_t addrDescrip;
    readByte(regDescrip);
    uint8_t regD;
    uint8_t regS;
    getReg(regDescrip, regD, regS);
    readByte(addrDescrip);
    uint16_t* operand = resolveAddr(addrDescrip, reg[regS], true);
    if(operand == nullptr)
        return;
    if(reg[psw] & 1)
        reg[pc] = *operand;
    resolveAddr(addrDescrip, reg[regS], false);
}

void emulator::jneInst()
{
    //printf("jne");
    uint8_t regDescrip;
    uint8_t addrDescrip;
    readByte(regDescrip);
    uint8_t regD;
    uint8_t regS;
    getReg(regDescrip, regD, regS);
    readByte(addrDescrip);
    uint16_t* operand = resolveAddr(addrDescrip, reg[regS], true);
    if(operand == nullptr)
        return;
    if(!(reg[psw] & 1))
        reg[pc] = *operand;
    resolveAddr(addrDescrip, reg[regS], false);
}

void emulator::jgtInst()
{
    //printf("jgt");
    uint8_t regDescrip;
    uint8_t addrDescrip;
    readByte(regDescrip);
    uint8_t regD;
    uint8_t regS;
    getReg(regDescrip, regD, regS);
    readByte(addrDescrip);
    uint16_t* operand = resolveAddr(addrDescrip, reg[regS], true);
    if(operand == nullptr)
        return;
    int z = reg[psw] & 1;
    int n = reg[psw] & (1 << 3);
    int o = reg[psw] & (1 << 1);
    if( ((n ^ o) | z) == 0)
        reg[pc] = *operand;
    resolveAddr(addrDescrip, reg[regS], false);
}

void emulator::resolveInterrupt()
{
    if((reg[psw] & (1<<15)) == 0)
    {
        if(badOpCodeInt)
        {
            pushWord(reg[psw]);
            pushWord(reg[pc]);
            memMutex.lock();
            uint8_t l = mem[2];
            uint8_t h = mem[3];
            memMutex.unlock();
            reg[pc] = ((h << 8) & 0xFF00) | (l & 0x00FF);
            badOpCodeInt = false;
        }
        else if((reg[psw] & (1 << 13)) == 0 && timerInt)
        {
            pushWord(reg[psw]);
            pushWord(reg[pc]);
            memMutex.lock();
            uint8_t l = mem[4];
            uint8_t h = mem[5];
            memMutex.unlock();
            reg[pc] = ((h << 8) & 0xFF00) | (l & 0x00FF);
            timerInt = false;
            //printf("timer interrupt\n");
        }
        else if((reg[psw] & (1 << 14)) == 0 && terminalInt)
        {
            pushWord(reg[psw]);
            pushWord(reg[pc]);
            memMutex.lock();
            uint8_t l = mem[6];
            uint8_t h = mem[7];
            memMutex.unlock();
            reg[pc] = ((h << 8) & 0xFF00) | (l & 0x00FF);
            terminalInt = false;
        }
    }
}

void emulator::timer()
{
    std::map<int, int> timerMap({
        {0, 500},
        {1, 1000},
        {2, 1500},
        {3, 2000},
        {4, 5000},
        {5, 10000},
        {6, 30000},
        {7, 50000}
    });
    while(running)
    {
        uint16_t val;
        memMutex.lock();
        uint8_t l = mem[0xFF10];
        uint8_t h = mem[0xFF11];
        memMutex.unlock();
        val = l & 0x00FF;
        val |= (h << 8) & 0xFF00;
        auto tmp = timerMap.find(val);
        if(tmp == timerMap.end())
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        else
            std::this_thread::sleep_for(std::chrono::milliseconds(tmp->second));
        timerInt = true;
    }
}

void emulator::terminal()
{
    while(running)
    {
        char c = getc(stdin);
        memMutex.lock();
        mem[0xFF02] = c;
        memMutex.unlock();
        terminalInt = true;
    }
}
#pragma once
#include <string>
#include "assembler.h"
#include "list.h"
#include "instruction.h"

class pass
{
public:
    virtual void inst(instruction*) = 0;
    virtual void label(std::string) = 0;
    virtual void sectionD(std::string) = 0;
    virtual void globalD(list*) = 0;
    virtual void externD(list*) = 0;
    virtual void wordD(int) = 0;
    virtual void wordD(list*) = 0;
    virtual void skipD(int) = 0;
    virtual void equD(std::string, int) = 0;
    virtual void endD() = 0;
};

class firstPass: public pass
{
public:
    void inst(instruction*);
    void label(std::string);
    void sectionD(std::string);
    void globalD(list*);
    void externD(list*);
    void wordD(int);
    void wordD(list*);
    void skipD(int);
    void equD(std::string, int);
    void endD();
};

class secondPass: public pass
{
public:
    void inst(instruction*);
    void label(std::string);
    void sectionD(std::string);
    void globalD(list*);
    void externD(list*);
    void wordD(int);
    void wordD(list*);
    void skipD(int);
    void equD(std::string, int);
    void endD();
};

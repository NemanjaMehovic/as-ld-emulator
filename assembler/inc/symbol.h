#pragma once
#include <string>
#include <vector>
#include "instruction.h"

typedef struct r{
    int val;
    std::string type;
    int id;
}rel;

class symbol
{
private:
    std::string name;
    std::string section;
    int id;
    int val;
    bool local;
public:
    symbol(std::string, std::string, int, int, bool);
    virtual ~symbol();
    virtual bool isLocal();
    virtual void setLocal(bool);
    virtual int getVal();
    virtual int getId();
    virtual std::string getName();
    virtual std::string getSection();
};

class section:public symbol
{
private:
    std::vector<int8_t>* inst;
    std::vector<rel*>* relData;
    int locationCounter;
public:
    section(std::string, std::string, int, int, bool);
    ~section();
    void increaseCounter(int);
    void resetCounter();
    int getCounter();
    void addByte(int8_t);
    void addPayload(int);
    void insertRel(rel*);
    std::vector<int8_t>* getBytes();
    std::vector<rel*>* getRel();
};
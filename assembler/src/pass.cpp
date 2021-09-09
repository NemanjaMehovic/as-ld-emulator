#include <iostream>
#include "pass.h"
#include "list.h"
#include "instruction.h"
#include "assembler.h"
#include "symbol.h"

void firstPass::inst(instruction* inst)
{
    if(assembler::instance()->getCurrSection()->getName() == "U")
    {
        std::cout << "No section defined" << std::endl;
        exit(-1);
    }
    assembler::instance()->getCurrSection()->increaseCounter(inst->getSize());
    delete inst;
}

void firstPass::label(std::string s)
{
    if(assembler::instance()->getCurrSection()->getName() == "U")
    {
        std::cout << "No section defined" << std::endl;
        exit(-1);
    }
    if(assembler::instance()->findSymbol(s) != nullptr)
    {
        std::cout << "symbol " << s << " already defined" << std::endl;
        exit(-1);
    }
    if(s == "A" || s == "U" || s == "E")
    {
        std::cout << "symbol " << s << " reserved" << std::endl;
        exit(-1);
    }
    assembler::instance()->getTable()->push_back(new symbol(s, 
        assembler::instance()->getCurrSection()->getName(), 
        assembler::instance()->getCurrSection()->getCounter(), 
        assembler::instance()->getTable()->size() + 1, 
        true));
}

void firstPass::sectionD(std::string s)
{
    if(assembler::instance()->findSymbol(s) != nullptr || s == "A" || s == "U" || s == "E")
    {
        std::cout << "symbol " << s << " already defined" << std::endl;
        exit(-1);
    }
    section* tmpS = new section(s, s, 0, assembler::instance()->getTable()->size() + 1, true);
    assembler::instance()->getTable()->push_back(tmpS);
    assembler::instance()->getSections()->push_back(tmpS);
    assembler::instance()->setCurrSection(tmpS);
}

void firstPass::globalD(list* l){/*does nothing on first pass*/}

void firstPass::externD(list* l){/*does nothing on first pass*/}

void firstPass::wordD(int i)
{
    if(assembler::instance()->getCurrSection()->getName() == "U")
    {
        std::cout << "No section defined" << std::endl;
        exit(-1);
    }
    assembler::instance()->getCurrSection()->increaseCounter(2);
}

void firstPass::wordD(list* l)
{
    if(assembler::instance()->getCurrSection()->getName() == "U")
    {
        std::cout << "No section defined" << std::endl;
        exit(-1);
    }
    assembler::instance()->getCurrSection()->increaseCounter(l->get()->size() * 2);
}

void firstPass::skipD(int i)
{
    if(assembler::instance()->getCurrSection()->getName() == "U")
    {
        std::cout << "No section defined" << std::endl;
        exit(-1);
    }
    assembler::instance()->getCurrSection()->increaseCounter(i);
}

void firstPass::equD(std::string s, int i)
{
    if(assembler::instance()->findSymbol(s) != nullptr)
    {
        std::cout << "symbol " << s << " already defined" << std::endl;
        exit(-1);
    }
    assembler::instance()->getTable()->push_back(new symbol(s, "A", i, assembler::instance()->getTable()->size() + 1, true));
}

void firstPass::endD()
{
    for(auto tmp:*assembler::instance()->getSections())
        tmp->resetCounter();
    assembler::instance()->setRules(new secondPass());
    delete this;
}

void secondPass::inst(instruction* inst)
{
    assembler::instance()->getCurrSection()->addByte(inst->getCode());
    if(!inst->hasSymbol())
    {
        for(int i = 1; i < inst->getSize();)
        {
            switch (i)
            {
            case 1:
                assembler::instance()->getCurrSection()->addByte(inst->getOperand()->getRegDesc());
                i++;
                break;
            case 2:
                assembler::instance()->getCurrSection()->addByte(inst->getOperand()->getAdrMode());
                i++;
                break;
            case 3:
                assembler::instance()->getCurrSection()->addPayload(inst->getOperand()->getLiteral());
                i += 2;
                break;
            }
        }
    }
    else
    {
        assembler::instance()->getCurrSection()->addByte(inst->getOperand()->getRegDesc());
        assembler::instance()->getCurrSection()->addByte(inst->getOperand()->getAdrMode());
        symbol* tmpSymbol = assembler::instance()->findSymbol(inst->getOperand()->getSymbol());
        if(tmpSymbol == nullptr)
        {
            tmpSymbol = new symbol(inst->getOperand()->getSymbol(), "E", 0, assembler::instance()->getTable()->size() + 1, false);
            assembler::instance()->getTable()->push_back(tmpSymbol);
        }
        if(inst->getAdrMode() == ABS)
        {
            if(tmpSymbol->getSection() == "A")
                assembler::instance()->getCurrSection()->addPayload(tmpSymbol->getVal());
            else
            {
                int num = 0;
                if(tmpSymbol->isLocal())
                {
                    assembler::instance()->getCurrSection()->addPayload(tmpSymbol->getVal());
                    num = assembler::instance()->findSection(tmpSymbol->getSection())->getId();
                }
                else
                {
                    assembler::instance()->getCurrSection()->addPayload(0);
                    num = tmpSymbol->getId();
                }
                rel* tmpRel = new rel();
                tmpRel->id = num;
                tmpRel->type = ABS;
                tmpRel->val = assembler::instance()->getCurrSection()->getCounter() + 3;
                assembler::instance()->getCurrSection()->insertRel(tmpRel);
            }
        }
        else
        {
            if(tmpSymbol->getSection() == "A")
            {
                std::cout << "Using absolute section for relative address" << std::endl;
                exit(-1);
            }
            if(tmpSymbol->getSection() == assembler::instance()->getCurrSection()->getName())
                assembler::instance()->getCurrSection()->addPayload(tmpSymbol->getVal() - (assembler::instance()->getCurrSection()->getCounter() + 5));
            else
            {
                int num = 0;
                if(tmpSymbol->isLocal())
                    num = assembler::instance()->findSection(tmpSymbol->getSection())->getId();
                else
                    num = tmpSymbol->getId();
                assembler::instance()->getCurrSection()->addPayload(-2);
                rel* tmpRel = new rel();
                tmpRel->id = num;
                tmpRel->type = REL;
                tmpRel->val = assembler::instance()->getCurrSection()->getCounter() + 3;
                assembler::instance()->getCurrSection()->insertRel(tmpRel);
            }
        }
    }
    assembler::instance()->getCurrSection()->increaseCounter(inst->getSize());
    delete inst;
}

void secondPass::label(std::string s){/*does nothing on second pass*/}

void secondPass::sectionD(std::string s)
{
    assembler::instance()->setCurrSection(assembler::instance()->findSection(s));
}

void secondPass::globalD(list* l)
{
    for(auto tmp:*l->get())
    {
        symbol* tmpSymbol = assembler::instance()->findSymbol(tmp);
        if(tmpSymbol == nullptr)
        {
            std::cout << "Symbol " << tmp << " declared global but never defined" << std::endl;
            exit(-1);
        }
        tmpSymbol->setLocal(false);
    }
    delete l;
}

void secondPass::externD(list* l)
{
    for(auto tmp:*l->get())
    {
        symbol* tmpSymbol = assembler::instance()->findSymbol(tmp);
        if(tmpSymbol != nullptr)
        {
            std::cout << "Symbol " << tmp << " already defined" << std::endl;
            exit(-1);
        }
        tmpSymbol = new symbol(tmp, "E", 0, assembler::instance()->getTable()->size() + 1, false);
        assembler::instance()->getTable()->push_back(tmpSymbol);
    }
    delete l;
}

void secondPass::wordD(int i)
{
    assembler::instance()->getCurrSection()->addPayload(i);
    assembler::instance()->getCurrSection()->increaseCounter(2);
}

void secondPass::wordD(list* l)
{
    for(auto tmp:*l->get())
    {
        symbol* tmpSymbol = assembler::instance()->findSymbol(tmp);
        if(tmpSymbol == nullptr)
        {
            tmpSymbol = new symbol(tmp, "E", 0, assembler::instance()->getTable()->size() + 1, false);
            assembler::instance()->getTable()->push_back(tmpSymbol);
        }
        if(tmpSymbol->getSection() == "A")
            assembler::instance()->getCurrSection()->addPayload(tmpSymbol->getVal());
        else
        {
            int num = 0;
            if(tmpSymbol->isLocal())
            {
                assembler::instance()->getCurrSection()->addPayload(tmpSymbol->getVal());
                num = assembler::instance()->findSection(tmpSymbol->getSection())->getId();
            }
            else
            {
                assembler::instance()->getCurrSection()->addPayload(0);
                num = tmpSymbol->getId();
            }
            rel* tmpRel = new rel();
            tmpRel->id = num;
            tmpRel->type = ABS;
            tmpRel->val = assembler::instance()->getCurrSection()->getCounter();
            assembler::instance()->getCurrSection()->insertRel(tmpRel);
        }
        assembler::instance()->getCurrSection()->increaseCounter(2);
    }
}

void secondPass::skipD(int i)
{
    for(int tmp = 0; tmp < i; tmp++)
        assembler::instance()->getCurrSection()->addByte(0);
    assembler::instance()->getCurrSection()->increaseCounter(i);
}

void secondPass::equD(std::string s, int i){/*does nothing on second pass*/}

void secondPass::endD(){/*does nothing on second pass*/}
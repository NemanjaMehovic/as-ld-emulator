#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include "linker.h"
#include "file.h"
#include "symbol.h"

linker* linker::singleton = nullptr;

linker* linker::instance()
{
    if(singleton == nullptr)
        singleton = new linker();
    return singleton;
}

void linker::deleteInstance()
{
    if(singleton != nullptr)
    {
        delete singleton;
        singleton = nullptr;
    }
}

linker::linker()
{
    files = new std::vector<file*>();
    table = new std::vector<symbol*>();
    sections = new std::vector<section*>();
}

linker::~linker()
{
    delete files;
    for(auto tmp:*table)
        delete tmp;
    delete table;
    delete sections;
}

void linker::addFile(file* f)
{
    files->push_back(f);
}

    
std::vector<file*>* linker::getFiles()
{
    return files;
}

file* linker::getFile(std::string fileName)
{
    for(auto tmp:*files)
        if(tmp->getName() == fileName)
            return tmp;
    return nullptr;
}

bool linker::checkSymbolExistsInFiles(symbol* s)
{
    for(auto tmpFile:*files)
    {
        symbol* tmpSymbol = tmpFile->findSymbol(s->getName());
        if(tmpSymbol->getSection() != "E" && !tmpSymbol->isLocal())
            return true;
    }
    return false;
}

std::vector<symbol*>* linker::getTable()
{
    return table;
}

std::vector<section*>* linker::getSections()
{
    return sections;
}

symbol* linker::findSymbol(std::string name)
{
    for(auto tmp:*table)
        if(tmp->getName() == name)
            return tmp;
    return nullptr;
}

section* linker::findSection(std::string name) 
{
    for(auto tmp:*sections)
        if(tmp->getName() == name)
            return tmp;
    return nullptr;
}

void linker::resolveSymbols()
{
    for(auto currFile:*files)
    {
        for(auto currSymbol:*currFile->getTable())
        {
            if(currSymbol->isLocal() && !currSymbol->isSection())
                continue;
            if(currSymbol->isSection())
            {
                section* workingSection = findSection(currSymbol->getName());
                if(workingSection == nullptr)
                {
                    workingSection = new section(currSymbol->getName(), currSymbol->getName(), 0, table->size() + 1, true);
                    sections->push_back(workingSection);
                    table->push_back(workingSection);
                }
                section* currSection = (section*) currSymbol;
                currSection->setLocation(workingSection->getBytes()->size());
                for(auto rel:*currSection->getRel())
                    rel->val = rel->val + currSection->getLocation();
                for(auto tmpSection:*currFile->getSections())
                    for(auto rel:*tmpSection->getRel())
                        if(!rel->modified && rel->id == currSection->getId())
                        {
                            rel->modified = true;
                            rel->id = workingSection->getId();
                            int8_t l = (*tmpSection->getBytes())[rel->val - tmpSection->getLocation()];
                            int8_t h = (*tmpSection->getBytes())[rel->val - tmpSection->getLocation() + 1];
                            int data = (h << 8) | l;
                            data += currSection->getLocation();
                            l = data & 0b11111111;
                            h = (data >> 8) & 0b11111111;
                            (*tmpSection->getBytes())[rel->val - tmpSection->getLocation()] = l;
                            (*tmpSection->getBytes())[rel->val - tmpSection->getLocation() + 1] = h;
                        }
            }
            else
            {
                symbol* workingSymbol = findSymbol(currSymbol->getName());
                if(workingSymbol == nullptr)
                {
                    if(currSymbol->getSection() == "E")
                        workingSymbol = new symbol(currSymbol->getName(), currSymbol->getSection(), 0, table->size() + 1, false);
                    else if(currSymbol->getSection() == "A")//new line
                        workingSymbol = new symbol(currSymbol->getName(), currSymbol->getSection(), currSymbol->getVal(), table->size() + 1, false);//new line
                    else
                    {
                        section* tmpSection = currFile->findSection(currSymbol->getSection());
                        workingSymbol = new symbol(currSymbol->getName(), currSymbol->getSection(), tmpSection->getLocation() + currSymbol->getVal(), table->size() + 1, false);
                    }
                    table->push_back(workingSymbol);
                }
                else if(workingSymbol->getSection() == "E" && currSymbol->getSection() != "E")
                {
                    workingSymbol->setSection(currSymbol->getSection());
                    section* tmpSection = currFile->findSection(currSymbol->getSection());
                    workingSymbol->setVal(currSymbol->getVal() + tmpSection->getLocation());
                }
                for(auto tmpSection:*currFile->getSections())
                    for(auto rel:*tmpSection->getRel())
                        if(!rel->modified && rel->id == currSymbol->getId())
                        {
                            rel->modified = true;
                            rel->id = workingSymbol->getId();
                        }
            }
        }
        for(auto tmpSection:*currFile->getSections())
        {
            section* workingSection = findSection(tmpSection->getName());
            for(auto tmpRel:*tmpSection->getRel())
            {
                rel* currRel = new rel();
                currRel->id = tmpRel->id;
                currRel->type = tmpRel->type;
                currRel->val = tmpRel->val;
                workingSection->insertRel(currRel);
            }
            for(auto data:*tmpSection->getBytes())
                workingSection->addByte(data);
        }
    }
    for(auto tmpFile:*files)
        delete tmpFile;
}

void linker::linkable(std::string fileName)
{
    FILE* file = fopen(fileName.c_str(), "w");
    if(!file)
    {
        std::cout << "Failed to open file: " << fileName << std::endl;
        exit(-1);
    }
    fprintf(file, "symbol table\n");
    fprintf(file, "name                section             val                 local               id\n");
    for(auto tmp:*table)
        fprintf(file, "%-20s%-20s%-20X%-20s%-20X\n",tmp->getName().c_str(), tmp->getSection().c_str(), tmp->getVal(), tmp->isLocal() ? "local" : "global", tmp->getId());
    fprintf(file, "\n");
    for(auto tmp:*sections)
        if(tmp->getRel()->size() != 0)
        {
            fprintf(file, ".rel.%s\n", tmp->getName().c_str());
            fprintf(file, "val                 type                id\n");
            for(auto tmp2:*tmp->getRel())
                fprintf(file, "%-20X%-20s%-20X\n", tmp2->val, tmp2->type.c_str(), tmp2->id);
            fprintf(file, "\n");
        }
    for(auto tmp:*sections)
        if(tmp->getBytes()->size() != 0)
        {
            fprintf(file, "%s\n", tmp->getName().c_str());
            for(auto tmp2:*tmp->getBytes())
            {
                unsigned tmp3 = tmp2;
                tmp3 &= 0b11111111;
                fprintf(file, "%02X ", tmp3);
            }
            fprintf(file, "\n");
        }
    fclose(file);
}

void linker::linkableBin(std::string fileName)
{
    std::ofstream fileBin(fileName + ".bin", std::ios::out | std::ios::binary);
    if(!fileBin) 
    {
        std::cout << "Failed to open file: " << fileName + ".bin" << std::endl;
        exit(-1);
    }
    int tmp = table->size();
    fileBin.write((char*) &tmp, sizeof(tmp));
    for(auto tmpSymbol:*table)
    {
        tmp = tmpSymbol->getName().size();
        fileBin.write((char*) &tmp, sizeof(tmp));
        fileBin.write(tmpSymbol->getName().c_str(), sizeof(char) * tmp);
        tmp = tmpSymbol->getSection().size();
        fileBin.write((char*) &tmp, sizeof(tmp));
        fileBin.write(tmpSymbol->getSection().c_str(), sizeof(char) * tmp);
        tmp = tmpSymbol->getVal();
        fileBin.write((char*) &tmp, sizeof(tmp));
        tmp = tmpSymbol->isLocal() ? 1 : 0;
        fileBin.write((char*) &tmp, sizeof(tmp));
        tmp = tmpSymbol->getId();
        fileBin.write((char*) &tmp, sizeof(tmp));
    }
    tmp = 0;
    for(auto tmpSection:*sections)
        if(tmpSection->getRel()->size() != 0)
            tmp++;
    fileBin.write((char*) &tmp, sizeof(tmp));
    for(auto tmpSection:*sections)
        if(tmpSection->getRel()->size() != 0)
        {
            tmp = tmpSection->getName().size();
            fileBin.write((char*) &tmp, sizeof(tmp));
            fileBin.write(tmpSection->getName().c_str(), sizeof(char) * tmp);
            tmp = tmpSection->getRel()->size();
            fileBin.write((char*) &tmp, sizeof(tmp));
            for(auto tmpRel:*tmpSection->getRel())
            {
                tmp = tmpRel->val;
                fileBin.write((char*) &tmp, sizeof(tmp));
                tmp = tmpRel->type.size();
                fileBin.write((char*) &tmp, sizeof(tmp));
                fileBin.write(tmpRel->type.c_str(), sizeof(char) * tmp);
                tmp = tmpRel->id;
                fileBin.write((char*) &tmp, sizeof(tmp));
            }
        }
    tmp = 0;
    for(auto tmpSection:*sections)
        if(tmpSection->getBytes()->size() != 0)
            tmp++;
    fileBin.write((char*) &tmp, sizeof(tmp));
    for(auto tmpSection:*sections)
        if(tmpSection->getBytes()->size() != 0)
        {
            tmp = tmpSection->getName().size();
            fileBin.write((char*) &tmp, sizeof(tmp));
            fileBin.write(tmpSection->getName().c_str(), sizeof(char) * tmp);
            tmp = tmpSection->getBytes()->size();
            fileBin.write((char*) &tmp, sizeof(tmp));
            for(auto tmpData:*tmpSection->getBytes())
                fileBin.write((char*) &tmpData, sizeof(tmpData));
        }
    fileBin.close();
}

void linker::setPlace(std::vector<std::pair<std::string, unsigned>*>* place)
{
    const static unsigned reg_map = 0xff00;
    const static unsigned stack = reg_map - 0x800;
    unsigned nextLocation = 0;
    for(auto tmp:*place)
    {
        section* workingSection = findSection(tmp->first);
        if(workingSection != nullptr)
        {
            workingSection->setLocation(tmp->second);
            workingSection->increaseCounter(1);
            if((workingSection->getBytes()->size() + workingSection->getLocation()) >= stack && (workingSection->getBytes()->size() + workingSection->getLocation()) < reg_map)
            {
                std::cout << "Section: " << workingSection->getName() << " overlaps with stack" << std::endl;
                exit(-1);
            }
            if((workingSection->getBytes()->size() + workingSection->getLocation()) >= reg_map)
            {
                std::cout << "Section: " << workingSection->getName() << " overlaps with mapped registers" << std::endl;
                exit(-1);
            }
            if((workingSection->getBytes()->size() + workingSection->getLocation()) > nextLocation)
                nextLocation = workingSection->getBytes()->size() + workingSection->getLocation();
        }
    }
    for(auto tmp:*sections)
        if(tmp->getCounter() == 0)
        {
            tmp->setLocation(nextLocation);
            tmp->increaseCounter(1);
            if((tmp->getBytes()->size() + tmp->getLocation()) >= stack && (tmp->getBytes()->size() + tmp->getLocation()) < reg_map)
            {
                std::cout << "Section: " << tmp->getName() << " overlaps with stack" << std::endl;
                exit(-1);
            }
            if((tmp->getBytes()->size() + tmp->getLocation()) >= reg_map)
            {
                std::cout << "Section: " << tmp->getName() << " overlaps with mapped registers" << std::endl;
                exit(-1);
            }
            nextLocation = tmp->getBytes()->size() + tmp->getLocation();
        }
    for(int i = 0; i < sections->size() - 1; i++)
    {
        section* tmp1 = (*sections)[i];
        for(int j = i + 1; j < sections->size(); j++)
        {
            section* tmp2 = (*sections)[j];
            if((tmp1->getLocation() >= tmp2->getLocation() && tmp1->getLocation() < (tmp2->getLocation() + tmp2->getBytes()->size()))
                ||
                (tmp2->getLocation() >= tmp1->getLocation() && tmp2->getLocation() < (tmp1->getLocation() + tmp1->getBytes()->size())))
            {
                std::cout << "Sections: " << tmp1->getName() << " and " << tmp2->getName() << " overlap" << std::endl;
                exit(-1);
            }
        }
    }
    for(int i = 0; i < sections->size() - 1; i++)
        for(int j = i + 1; j < sections->size(); j++)
            if((*sections)[i]->getLocation() > (*sections)[j]->getLocation())
            {
                section* tmp = (*sections)[i];
                (*sections)[i] = (*sections)[j];
                (*sections)[j] = tmp;
            }
}

symbol* linker::findSymbolById(int id)
{
    for(auto tmp:*table)
        if(tmp->getId() == id)
            return tmp;
    return nullptr;
}

void linker::fixHexValues()
{
    const static std::string absT = "R_X86_64_32";
    const static std::string relT = "R_X86_64_PC32";
    for(auto tmp:*sections)
    {
        for(auto relEnt:*tmp->getRel())
        {
            symbol* workingSymbol = findSymbolById(relEnt->id);
            section* workingSection = findSection(workingSymbol->getSection());
            int8_t l = (*tmp->getBytes())[relEnt->val];
            int8_t h = (*tmp->getBytes())[relEnt->val + 1];
            int addend = (h << 8) | l;
            int newData;
            if(relEnt->type == absT)
                newData = workingSection->getLocation() + workingSymbol->getVal() + addend;
            else if(relEnt->type == relT)
                newData = workingSection->getLocation() + workingSymbol->getVal() + addend - (tmp->getLocation() + relEnt->val);
            l = newData & 0b11111111;
            h = (newData >> 8) & 0b11111111;
            (*tmp->getBytes())[relEnt->val] = l;
            (*tmp->getBytes())[relEnt->val + 1] = h;
        }
    }
}

void linker::hex(std::string fileName)
{
    FILE* file = fopen(fileName.c_str(), "w");
    if(!file)
    {
        std::cout << "Failed to open file: " << fileName << std::endl;
        exit(-1);
    }
    std::map<uint16_t, uint8_t> data;
    for(auto tmp:*sections)
        for(int i = 0; i < tmp->getBytes()->size(); i++)
            data.insert(std::make_pair(tmp->getLocation()+i, (*tmp->getBytes())[i]));
    int counter = 0;
    uint16_t prevAddr = -1;
    for(auto tmp:data)
    {
        uint16_t useLocation = tmp.first / 8;
        useLocation *= 8;
        if((prevAddr + 1) != tmp.first && counter != 0)
        {
            for(int i = counter; i < 8; i++);
                fprintf(file, "%02X", 0);
            fprintf(file, "\n");
            counter = 0;
        }
        if(counter == 0)
        {
            fprintf(file, "%04X:", useLocation);
            for(unsigned i = useLocation; i < tmp.first; i++)
            {
                counter++;
                fprintf(file, " %02X", 0);
            }
        }
        fprintf(file, " %02X", tmp.second);
        prevAddr = tmp.first;
        counter++;
        if(counter == 8)
        {
            fprintf(file, "\n");
            counter = 0;
        }
    }
    if(counter != 0)
    	for(int i = counter; i < 8; i++)
        	fprintf(file, " %02X", 0);
    fclose(file);
}

void linker::hexBin(std::string fileName)
{
    std::ofstream fileBin(fileName + ".bin", std::ios::out | std::ios::binary);
    if(!fileBin) 
    {
        std::cout << "Failed to open file: " << fileName + ".bin" << std::endl;
        exit(-1);
    }
    std::map<uint16_t, uint8_t> data;
    for(auto tmp:*sections)
        for(int i = 0; i < tmp->getBytes()->size(); i++)
            data.insert(std::make_pair(tmp->getLocation()+i, (*tmp->getBytes())[i]));
    int counter = 0;
    uint16_t prevAddr = -1;
    const int zero = 0;
    for(auto tmp:data)
    {
        uint16_t useLocation = tmp.first / 8;
        useLocation *= 8;
        if((prevAddr + 1) != tmp.first && counter != 0)
        {
            for(int i = counter; i < 8; i++);
                fileBin.write((char*) &zero, sizeof(int8_t));
            counter = 0;
        }
        if(counter == 0)
        {
            fileBin.write((char*) &useLocation, sizeof(useLocation));
            for(unsigned i = useLocation; i < tmp.first; i++)
            {
                counter++;
                fileBin.write((char*) &zero, sizeof(int8_t));
            }
        }
        fileBin.write((char*) &tmp.second, sizeof(tmp.second));
        prevAddr = tmp.first;
        counter = (counter + 1) % 8;
    }
    if(counter != 0)
    	for(int i = counter; i < 8; i++)
        	fileBin.write((char*) &zero, sizeof(int8_t));
    fileBin.close();
}

#include <iostream>
#include <fstream>
#include <string>
#include "fileReader.h"
#include "linker.h"
#include "file.h"

std::string fileReader::readString(std::ifstream* fileStream)
{
    int stringLength;
    fileStream->read((char*) &stringLength, sizeof(stringLength));
    char* tmpString = new char[stringLength+1];
    fileStream->read(tmpString, stringLength);
    tmpString[stringLength] = '\0';
    std::string s = tmpString;
    delete [] tmpString;
    return s;
}

void fileReader::createFile(std::string fileName)
{
    std::ifstream fileStream(fileName, std::ios::in | std::ios::binary);
    if(!fileStream)
    {
        std::cout << "Failed to open file: " << fileName << std::endl;
        exit(-1);
    }
    if(linker::instance()->getFile(fileName) != nullptr)
        return;
    file* currFile = new file(fileName);
    int tmpCount;
    fileStream.read((char*) &tmpCount, sizeof(tmpCount));
    for(int i = 0; i < tmpCount; i++)
    {
        std::string symbolName = readString(&fileStream);
        std::string symbolSection = readString(&fileStream);

        int symbolVal;
        fileStream.read((char*) &symbolVal, sizeof(symbolVal));

        int symbolLocal;
        fileStream.read((char*) &symbolLocal, sizeof(symbolLocal));

        int symbolId;
        fileStream.read((char*) &symbolId, sizeof(symbolId));

        if(symbolName == symbolSection)
        {
            section* currSymbol = new section(symbolName, symbolSection, symbolVal, symbolId, symbolLocal == 1);
            currFile->addSection(currSymbol);
        }
        else
        {
            symbol* currSymbol = new symbol(symbolName, symbolSection, symbolVal, symbolId, symbolLocal == 1);
            currFile->addSymbol(currSymbol);
            if(symbolLocal != 1 && symbolSection != "E")
                if(linker::instance()->checkSymbolExistsInFiles(currSymbol))
                {
                    std::cout << "Symbol: " << currSymbol->getName() << " already exists and is defind as global in another file" << std::endl;
                    exit(-1);
                }
        }
    }

    fileStream.read((char*) &tmpCount, sizeof(tmpCount));
    for(int i = 0; i < tmpCount; i++)
    {
        std::string sectionName = readString(&fileStream);

        section* currSection = currFile->findSection(sectionName);
        int currCount;
        fileStream.read((char*) &currCount, sizeof(currCount));
        for(int i = 0; i < currCount; i++)
        {
            int relVal;
            fileStream.read((char*) &relVal, sizeof(relVal));

            std::string relType = readString(&fileStream);

            int relId;
            fileStream.read((char*) &relId, sizeof(relId));

            rel* currRel = new rel();
            currRel->id = relId;
            currRel->type = relType;
            currRel->val = relVal;
            currSection->insertRel(currRel);
        }
    }

    fileStream.read((char*) &tmpCount, sizeof(tmpCount));
    for(int i = 0; i < tmpCount; i++)
    {
        std::string sectionName = readString(&fileStream);

        section* currSection = currFile->findSection(sectionName);
        int currCount;
        fileStream.read((char*) &currCount, sizeof(currCount));
        for(int i = 0; i < currCount; i++)
        {
            int8_t byte;
            fileStream.read((char*) &byte, sizeof(byte));
            currSection->addByte(byte);
        }
    }
    fileStream.close();
    if(!fileStream.good())
    {
        std::cout << "Something went wrong while reading: " << fileName << std::endl;
        exit(-1);
    }
    linker::instance()->addFile(currFile);
}
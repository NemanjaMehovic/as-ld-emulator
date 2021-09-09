#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include "lexer.h"
#include "parser.h"
#include "assembler.h"

extern void cisti();

int main(int argc, char const *argv[])
{
    if(argc < 2)
    {
        std::cout << "No file given" << std::endl;
        return -1;
    }
    std::string out = "out.o";
    for(int i = 1; i < argc - 1; i++)
    {
        if(strcmp(argv[i], "-o") == 0)
        {
            if(argc < 4)
            {
                std::cout << "Not enough arguments" << std::endl;
                return -1;
            }
            out = argv[++i];
        }
        else
        {
            std::cout << "Unknown option" << std::endl;
            return -1;
        }
    }

    for(int i = 0 ; i < 2; i++)
    {
        FILE *myfile = fopen(argv[argc - 1], "r");
        if (!myfile) 
        {
            std::cout << "Failed to open file: " << argv[argc - 1] << std::endl;
            return -1;
        }
        yyin = myfile;
        cisti();
        yyparse();
        fclose(myfile);
    }
    //print txt
    FILE* file = fopen(out.c_str(), "w");
    if(!file)
    {
        std::cout << "Failed to open file: " << out << std::endl;
        return -1;
    }
    fprintf(file, "symbol table\n");
    fprintf(file, "name                section             val                 local               id\n");
    for(auto tmp:*assembler::instance()->getTable())
        fprintf(file, "%-20s%-20s%-20X%-20s%-20X\n",tmp->getName().c_str(), tmp->getSection().c_str(), tmp->getVal(), tmp->isLocal() ? "local" : "global", tmp->getId());
    fprintf(file, "\n");
    for(auto tmp:*assembler::instance()->getSections())
        if(tmp->getRel()->size() != 0)
        {
            fprintf(file, ".rel.%s\n", tmp->getName().c_str());
            fprintf(file, "val                 type                id\n");
            for(auto tmp2:*tmp->getRel())
                fprintf(file, "%-20X%-20s%-20X\n", tmp2->val, tmp2->type.c_str(), tmp2->id);
            fprintf(file, "\n");
        }
    for(auto tmp:*assembler::instance()->getSections())
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
    //print bin
    std::ofstream fileBin(out + ".bin", std::ios::out | std::ios::binary);
    if(!fileBin) 
    {
        std::cout << "Failed to open file: " << out + ".bin" << std::endl;
        return -1;
    }
    int tmp = assembler::instance()->getTable()->size();
    fileBin.write((char*) &tmp, sizeof(tmp));
    for(auto tmpSymbol:*assembler::instance()->getTable())
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
    for(auto tmpSection:*assembler::instance()->getSections())
        if(tmpSection->getRel()->size() != 0)
            tmp++;
    fileBin.write((char*) &tmp, sizeof(tmp));
    for(auto tmpSection:*assembler::instance()->getSections())
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
    for(auto tmpSection:*assembler::instance()->getSections())
        if(tmpSection->getBytes()->size() != 0)
            tmp++;
    fileBin.write((char*) &tmp, sizeof(tmp));
    for(auto tmpSection:*assembler::instance()->getSections())
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

    assembler::deleteInstance();

    return 0;
}

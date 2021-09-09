#pragma once
#include <vector>
#include <string>
#include "file.h"
#include "symbol.h"


class linker
{
private:
    linker();
    ~linker();
    static linker* singleton;
    std::vector<file*>* files;
    std::vector<symbol*>* table;
    std::vector<section*>* sections;
public:
    static linker* instance();
    static void deleteInstance();
    void addFile(file*);
    std::vector<file*>* getFiles();
    file* getFile(std::string);
    bool checkSymbolExistsInFiles(symbol*);
    std::vector<symbol*>* getTable();
    std::vector<section*>* getSections();
    symbol* findSymbol(std::string);
    section* findSection(std::string);
    void resolveSymbols();
    void linkable(std::string);
    void linkableBin(std::string);
    void setPlace(std::vector<std::pair<std::string, unsigned>*>*);
    symbol* findSymbolById(int);
    void fixHexValues();
    void hex(std::string);
    void hexBin(std::string);
};

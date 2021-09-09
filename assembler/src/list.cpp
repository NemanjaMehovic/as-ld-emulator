#include "list.h"

list::list()
{
    l = new std::vector<std::string>();
}

list::~list()
{
    delete l;
}

void list::add(const char* c)
{
    std::string tmp(c);
    l->push_back(tmp);
}

std::vector<std::string>* list::get()
{
    return l;
}
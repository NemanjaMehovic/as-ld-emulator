#pragma once
#include <vector>
#include <string>

class list
{
private:
    std::vector<std::string>* l;
public:
    list();
    ~list();
    void add(const char* c);
    std::vector<std::string>* get();
};

#pragma once
#ifdef IBM
    #include <Windows.h>
#endif
#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#ifdef IBM
    std::wstring s2ws(const std::string& s);
#endif
std::stringstream loadFile(std::string filepath);
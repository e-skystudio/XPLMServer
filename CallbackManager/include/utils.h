#pragma once
#include <string>
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

std::wstring s2ws(const std::string& s);
std::stringstream loadFile(std::string filepath);
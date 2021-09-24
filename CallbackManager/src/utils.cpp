#include "../include/utils.h"


std::wstring s2ws(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

std::stringstream loadFile(std::string filepath)
{
    std::stringstream data;
    std::ifstream myfile(filepath);
    if (myfile.is_open())
    {
        std::string line;
        while (std::getline(myfile, line))
        {
            data << line << '\n';
        }
        myfile.close();
    }

    return data;
}
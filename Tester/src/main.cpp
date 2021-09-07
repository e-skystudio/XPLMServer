#include <iostream>
#include "Logger.h"


int main(int argc, char* argv[])
{
	Logger l("testlog.log", true);
	l.Log("This is log 1");
	l += "This is log2";
	return 0;
}
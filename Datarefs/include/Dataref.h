#pragma once
#include <string>
#include <Logger.h>
#include <XPLM/XPLMDataAccess.h>

class Dataref
//This class represent an X-Plane dataref in OOP format.
{
public:
	enum class Type {
		Unknown = 0,
		Int = 1,
		Float = 2,
		Double = 4,
		FloatArray=8,
		IntArray=16,
		Data=32
	};

	Dataref();
	Dataref(const Dataref& rhs);
	~Dataref();

	bool Load(std::string path);
	bool CanWrite();
	bool IsGood();
	Dataref::Type GetType();
	Dataref::Type LoadType();
	void SetType(Dataref::Type newType); 
protected:
	XPLMDataRef m_dataref;
	Dataref::Type m_type;
	Logger m_logger;
};


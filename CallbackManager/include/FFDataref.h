#pragma once
#include "Logger.h"
#include <string>
#include <nlohmann/json.hpp>
#include "SharedValue.h"

class FFDataref
{
public:
	enum class Type {
		Deleted = Value_Type_Deleted,
		Object = Value_Type_Object,
		Char = Value_Type_sint8,
		uChar = Value_Type_uint8,
		Short = Value_Type_sint16,
		uShort = Value_Type_uint16,
		Int = Value_Type_sint32,
		uInt = Value_Type_uint32,
		Float = Value_Type_float32,
		Double = Value_Type_float64,
		String = Value_Type_String,
		Time = Value_Type_Time,
	};
	FFDataref();
	FFDataref(SharedValuesInterface* FF_A320_api);
	FFDataref(const FFDataref& rhs);
	bool Load(std::string path);
	FFDataref::Type GetType();
	FFDataref::Type LoadType();
	std::string GetValue();
	void SetValue(std::string value);
	void BindAPI(SharedValuesInterface* FF_A320_api);
	void SetConversionFactor(std::string conversionFactor);
protected:
	std::string m_link;
	int m_id;
	Type m_type;
	std::string m_conversionFactor;
	SharedValuesInterface* m_ffapi;
	Logger m_logger;

};

static std::map<std::string, FFDataref::Type> const FFStringToType{
	{"Unknow", FFDataref::Type::Deleted},
	{"Object", FFDataref::Type::Object},
	{"Char", FFDataref::Type::Char},
	{"uChar", FFDataref::Type::uChar},
	{"Short", FFDataref::Type::Short},
	{"uShort", FFDataref::Type::uShort},
	{"Int", FFDataref::Type::Int},
	{"uInt", FFDataref::Type::uInt},
	{"Float", FFDataref::Type::Float},
	{"Double", FFDataref::Type::Double},
	{"String", FFDataref::Type::String},
	{"Time", FFDataref::Type::Time},
};

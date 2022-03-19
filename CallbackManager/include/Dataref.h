#pragma once
#include <string>
#include <map>

#include <nlohmann/json.hpp>

#include <XPLMDataAccess.h>
#include <XPLMUtilities.h>

#include "Logger.h"
#include "AbstractDataref.h"


using json = nlohmann::json;

///<summary>
/// This class represent an X-Plane dataref in OOP format.
///</summary>
class Dataref : public AbstractDataref
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

	///<summary>
	/// Default empty consturctor
	///</summary>
	Dataref();
	///<summary>
	/// Copy Consturctor
	///</summary>
	///<param name="rhs">The dataref to be copied.</param>
	Dataref(const Dataref& rhs);
	///<summary>
	/// Destructor
	///</summary>
	~Dataref();
	///<summary>
	/// Load a dataref from the path.
	///</summary>
	///
	///
	///<returns>True if sucessfully loaded</returns>
	bool Load(std::string path);
	///<summary>
	///Check if the dataref can be written to:
	///Dataref is not null and XPLMCanWriteDataRef() return true.]]>
	///</summary>
	///<returns>True if dataref is not readonly</returns>
	bool CanWrite();
	///<summary>
	///Check if the dataref is accessible:
	///Dataref is not null and XPLMIsDataRefGood() return true.
	///</summary>
	///<returns>True if dataref is valid</returns>
	bool IsGood();
	///<summary>
	///Return the type of the dataref stored in memory
	///</summary>
	///<returns>Dataref::Type</returns>
	Dataref::Type GetType();
	///<summary>
	/// Ask X-Plane SDK to return the type of the dataref
	///</summary>
	///<returns>The dataref type according the SDK</returns>
	Dataref::Type LoadType();
	///<summary>
	/// Override the type of the dataref.
	///</summary>
	///<param name="newType">The new type of the dataref</param>
	void SetType(Dataref::Type newType);
	///<summary>
	/// Override the type of the dataref.
	///</summary>
	///<param name="newType">The new type as string</param>
	void SetType(std::string newType);
	///<summary>
	/// Return the current value of the dataref (JSON formated).
	///</summary>
	///<returns>The value of the dataref as JSON</returns>
	std::string GetValue();
	///<summary>
	/// Send a new value to the dataref.
	///</summary>
	///<param name="value">The value to be sent to the dataref (as JSON)</param>
	void SetValue(std::string value);
	///<summary>
	/// Set a conversion factor to a dataref.
	///</summary>
	///<param name="conversionFactor">The multiplication factor to be applied to the dataref on get and set (as a division)</param>
	void SetConversionFactor(std::string conversionFactor);
protected:
	XPLMDataRef m_dataref;	/* Represent a void pointer locating the dataref as X - Plane SDK */
	Dataref::Type m_type;	/* Represent the underlying data type of the dataref */
	Logger m_logger;		/* The logger */
	std::string m_link;
	std::string m_conversionFactor;
	int setFloatArrayFromJson(int offset, std::string value);
	int setIntArrayFromJson(int offset, std::string value);
};


static std::map<std::string, Dataref::Type> const StringToType{
	{"Unknown", Dataref::Type::Unknown},
	{"INT", Dataref::Type::Int},
	{"FLOAT", Dataref::Type::Float},
	{"DOUBLE", Dataref::Type::Double},
	{"FLOAT_ARRAY", Dataref::Type::FloatArray},
	{"INT_ARRAY", Dataref::Type::IntArray},
	{"DATA", Dataref::Type::Data},
};
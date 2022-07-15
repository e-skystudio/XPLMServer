// ReSharper disable CppClangTidyClangDiagnosticShadowField
#pragma once
#include <string>
#include <map>

#include <nlohmann/json.hpp>
#include <string.h>

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
		Unknown = xplmType_Unknown,
		Int = xplmType_Int,
		Float = xplmType_Float,
		Double = xplmType_Double,
		FloatArray= xplmType_FloatArray,
		IntArray= xplmType_IntArray,
		Data=xplmType_Data,
	};

	///<summary>
	/// Default empty constructor
	///</summary>
	Dataref();
	///<summary>
	/// Copy Constructor
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
	///<returns>True if successfully loaded</returns>
	bool Load(const std::string path) override;
	///<summary>
	///Check if the dataref can be written to:
	///Dataref is not null and XPLMCanWriteDataRef() return true.]]>
	///</summary>
	///<returns>True if dataref is not readonly</returns>
	bool CanWrite() const;
	///<summary>
	///Check if the dataref is accessible:
	///Dataref is not null and XPLMIsDataRefGood() return true.
	///</summary>
	///<returns>True if dataref is valid</returns>
	bool IsGood() const;
	///<summary>
	///Return the type of the dataref stored in memory
	///</summary>
	///<returns>Dataref::Type</returns>
	Type GetType() const;
	///<summary>
	/// Ask X-Plane SDK to return the type of the dataref
	///</summary>
	///<returns>The dataref type according the SDK</returns>
	Type LoadType();
	///<summary>
	/// Override the type of the dataref.
	///</summary>
	///<param name="newType">The new type of the dataref</param>
	void SetType(Dataref::Type newType);
	///<summary>
	/// Override the type of the dataref.
	///</summary>
	///<param name="newType">The new type as string</param>
	void SetType(const std::string& newType);
	///<summary>
	/// Return the current value of the dataref (JSON formated).
	///</summary>
	///<returns>The value of the dataref as JSON</returns>
	std::string GetValue() override;
	///<summary>
	/// Send a new value to the dataref.
	///</summary>
	///<param name="value">The value to be sent to the dataref (as JSON)</param>
	void SetValue(std::string value) override;
	///<summary>
	/// Set a conversion factor to a dataref.
	///</summary>
	///<param name="conversionFactor">The multiplication factor to be applied to the dataref on get and set (as a division)</param>
	void SetConversionFactor(std::string conversionFactor) override;
protected:
	XPLMDataRef m_dataref = nullptr;	/* Represent a void pointer locating the dataref as X - Plane SDK */
	Dataref::Type m_type = Dataref::Type::Unknown;	/* Represent the underlying data type of the dataref */
	Logger m_logger = Logger("XPLMServer.log", "Dataref", false);		/* The logger */
	std::string m_link = "";
	std::string m_conversionFactor = "1.0";
	int SetFloatArrayFromJson(int offset, const std::string& value);
	int SetIntArrayFromJson(int offset, const std::string& value);
};


static std::map<std::string, Dataref::Type> const string_to_type{
	{"Unknown", Dataref::Type::Unknown},
	{"INT", Dataref::Type::Int},
	{"FLOAT", Dataref::Type::Float},
	{"DOUBLE", Dataref::Type::Double},
	{"FLOAT_ARRAY", Dataref::Type::FloatArray},
	{"INT_ARRAY", Dataref::Type::IntArray},
	{"DATA", Dataref::Type::Data},
};
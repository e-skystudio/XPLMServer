#pragma once
#include <string>
#include "Logger.h"
#include <XPLMDataAccess.h>

class Dataref
/*
* \brief This class represent an X-Plane dataref in OOP format.
*  Allow to register, get, set (if not readonly) value to specific datarefs.
*/
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
	/* \brief Default Constructor */
	Dataref();
	/* \brief Copy Constructor */
	Dataref(const Dataref& rhs);
	~Dataref();
	/* \brief Load a dataref with a specific path
	* \param[in] path the path of the dataref
	* \return boolean True if the path return a valid dataref
	*/
	bool Load(std::string path);
	bool CanWrite();
	bool IsGood();
	Dataref::Type GetType();
	Dataref::Type LoadType();
	void SetType(Dataref::Type newType); 
	std::string GetValue();
	void SetValue(std::string value);
protected:
	XPLMDataRef m_dataref; /*!< Represent a void pointer locating the dataref as X-Plane SDK */
	Dataref::Type m_type; /*!< Represent the underlying data type of the dataref */
	Logger m_logger; /*!< The logger */
};


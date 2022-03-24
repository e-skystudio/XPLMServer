#pragma once
#include <string>

enum class DatarefType {
    Abstract = 0,
    XPLMDataref = 1,
    FFDataref = 2,
};

class AbstractDataref{
    public:
        DatarefType DatarefType = DatarefType::Abstract;
        virtual std::string GetValue() = 0;
        virtual void SetValue(std::string value) = 0;
        virtual void SetConversionFactor(std::string conversionFactor) = 0;
        virtual bool Load(std::string path) = 0;
    protected:
        std::string m_link;
        std::string m_conversionFactor;
};
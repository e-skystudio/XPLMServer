#pragma once
#include <string>


class AbstractDataref{
    public:
        enum DatarefType{
            Abstract = 0,
            XPLMDataref = 1,
            FFDataref = 2,
        };
        DatarefType DatarefType = DatarefType::Abstract;
        virtual std::string GetValue() = 0;
        virtual void SetValue(std::string value) = 0;
        void SetConversionFactor(std::string conversionFactor);
        virtual bool Load(std::string path) = 0;
    protected:
        std::string m_link;
        std::string m_conversionFactor;
};
#pragma once
#include <string>

class AbstractDataref{
    public:
        std::string DatarefType;
        virtual std::string GetValue() = 0;
        virtual void SetValue(std::string value) = 0;
        void SetConversionFactor(std::string conversionFactor);
        virtual bool Load(std::string path) = 0;
    protected:
        std::string m_link;
        std::string m_conversionFactor;
};
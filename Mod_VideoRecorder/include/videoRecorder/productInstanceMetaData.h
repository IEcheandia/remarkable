#pragma once

#include <string>
#include <vector>

namespace precitec
{

namespace vdr
{

struct ProductInstanceMetaData
{
    struct Nio
    {
        Nio(int type, int count)
            : type(type)
            , count(count)
            {
            }
        int type = 0;
        int count = 0;
    };
    struct SeamMetaData
    {
        std::string uuid;
        std::string linkTo;
        std::string seamSeries;
        std::uint32_t number = 0;
        std::vector<Nio> nios;
    };
    struct SeamSeriesMetaData
    {
        std::string uuid;
        std::uint32_t number = 0;
        std::vector<Nio> nios;
        bool nioSwitchedOff = false;
        std::vector<SeamMetaData> processedSeams;
    };
    std::string uuid;
    std::uint32_t serialNumber = 0;
    std::string extendedProductInfo;
    bool nioSwitchedOff = false;
    // ISODateWithMs
    std::string date;

    std::vector<Nio> nios;
    std::vector<SeamMetaData> processedSeams;
    std::vector<SeamSeriesMetaData> processedSeamSeries;

    std::string productUuid;
    std::string productName;
    int productType = 0;
};

}
}

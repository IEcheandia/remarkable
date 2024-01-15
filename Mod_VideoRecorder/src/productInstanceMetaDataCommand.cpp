#include "videoRecorder/productInstanceMetaDataCommand.h"
#include "json.hpp"
#include <fstream>

using json = nlohmann::json;

namespace precitec
{
namespace vdr
{

void to_json(json &j, const ProductInstanceMetaData::Nio &nio)
{
    j = json{
        {"type", nio.type},
        {"count", nio.count},
    };
}

void to_json(json &j, const ProductInstanceMetaData::SeamMetaData &seam)
{
    j = json{
        {"uuid", seam.uuid},
        {"number", seam.number},
        {"nio", seam.nios},
    };
    if (!seam.seamSeries.empty())
    {
        j["seamSeriesUuid"] = seam.seamSeries;
    }
    if (!seam.linkTo.empty())
    {
        j["linkTo"] = seam.linkTo;
    }
}

void to_json(json &j, const ProductInstanceMetaData::SeamSeriesMetaData &seamSeries)
{
    j = json{
        {"uuid", seamSeries.uuid},
        {"number", seamSeries.number},
        {"nio", seamSeries.nios},
    };
    if (!seamSeries.processedSeams.empty())
    {
        j["nioSwitchedOff"] = seamSeries.nioSwitchedOff;
        j["processedSeams"] = seamSeries.processedSeams;
    }
}

void to_json(json &j, const ProductInstanceMetaData &product)
{
    j = json{
        {"uuid", product.uuid},
        {"serialNumber", product.serialNumber},
        {"extendedProductInfo", product.extendedProductInfo},
        {"date", product.date},
        {"nioSwitchedOff", product.nioSwitchedOff},
        {"processedSeamSeries", product.processedSeamSeries},
        {"processedSeams", product.processedSeams},
        {"nio", product.nios},
        {"productUuid", product.productUuid},
        {"productName", product.productName},
        {"productType", product.productType},
    };
}

void ProductInstanceMetaDataCommand::execute()
{
    json js{};
    to_json(js, m_metaData);
    std::ofstream jsonOutputFile{m_oFile.path()};
    jsonOutputFile << std::setw(4) << js << std::endl;
    jsonOutputFile.close();
}

void SeamSeriesMetaDataCommand::execute()
{
    json js{};
    to_json(js, m_metaData);
    std::ofstream jsonOutputFile{m_oFile.path()};
    jsonOutputFile << std::setw(4) << js << std::endl;
    jsonOutputFile.close();
}

}
}

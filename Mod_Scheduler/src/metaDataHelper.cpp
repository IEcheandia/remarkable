#include "Scheduler/metaDataHelper.h"
#include "json.hpp"

#include <fstream>

using json = nlohmann::json;

namespace precitec
{

namespace vdr
{

void from_json(const json &j, vdr::ProductInstanceMetaData &product)
{
    j.at("uuid").get_to(product.uuid);
    j.at("serialNumber").get_to(product.serialNumber);
    j.at("extendedProductInfo").get_to(product.extendedProductInfo);
    j.at("date").get_to(product.date);
    j.at("nioSwitchedOff").get_to(product.nioSwitchedOff);
    j.at("productUuid").get_to(product.productUuid);
    j.at("productName").get_to(product.productName);
    j.at("productType").get_to(product.productType);
}
}

namespace scheduler
{

MetaDataHelper::MetaDataHelper(const std::string &filePath)
    : m_metaData{readMetaData(filePath)}
{
}

std::optional<vdr::ProductInstanceMetaData> MetaDataHelper::readMetaData(const std::string& filename)
{
    std::ifstream file;
    file.open(filename);
    if (!file.is_open())
    {
        precitec::wmLogTr(precitec::eWarning, "QnxMsg.Scheduler.FailedOpenMetaData", "Could not open %s for reading\n", filename);
        return {};
    }
    nlohmann::json j;
    vdr::ProductInstanceMetaData metaData;
    try
    {
        file >> j;
        metaData = j;
    }
    catch(nlohmann::json::exception & e)
    {
        file.close();
        precitec::wmLogTr(precitec::eWarning, "QnxMsg.Scheduler.ExceptionOpenMetaData", "Exception when reading %s: %s (id: %d)\n", filename, e.what(), e.id);
        return {};
    }
    catch (...)
    {
        file.close();
        precitec::system::logExcpetion("Read " + filename, std::current_exception());
        return {};
    }
    file.close();
    return {metaData};
}

void MetaDataHelper::updateString(std::string &path)
{
    if (!m_metaData)
    {
        return;
    }
    const std::map<std::string, std::string> s_keyValues{{
        {{"${PRODUCT_NAME}"}, m_metaData.value().productName},
        {{"${PRODUCT_UUID}"}, m_metaData.value().productUuid},
        {{"${PRODUCT_TYPE}"}, std::to_string(m_metaData.value().productType)},
        {{"${SERIALNUMBER}"}, std::to_string(m_metaData.value().serialNumber)},
        {{"${UUID}"}, m_metaData.value().uuid},
        {{"${DATE}"}, m_metaData.value().date},
        {{"${EXTENDED_PRODUCT_INFO}"}, m_metaData.value().extendedProductInfo},
    }};
    for (const auto &pair : s_keyValues)
    {
        const auto pos = path.find(pair.first);
        if (pos == std::string::npos)
        {
            continue;
        }
        path.replace(pos, pair.first.size(), pair.second);
    }
}

void MetaDataHelper::updateTargetPath(std::map<std::string, std::string> &settings)
{
    auto it = settings.find("TargetDirectoryPath");
    if (it == settings.end())
    {
        return;
    }
    updateString(it->second);
}


void MetaDataHelper::updateTargetFileName(std::map<std::string, std::string> &settings)
{
    auto it = settings.find("TargetFileName");
    if (it == settings.end())
    {
        return;
    }
    updateString(it->second);
}

}
}

#pragma once
#include "videoRecorder/fileCommand.h"
#include "videoRecorder/productInstanceMetaData.h"

namespace precitec
{
namespace vdr
{

class ProductInstanceMetaDataCommand : public BaseCommand, public FileCommand
{
public:
    ProductInstanceMetaDataCommand(const Poco::File& file, ProductInstanceMetaData &&metaData)
        : FileCommand(file)
        , m_metaData(std::move(metaData))
    {
    }

    void execute() override;

private:
    ProductInstanceMetaData m_metaData;
};

class SeamSeriesMetaDataCommand : public BaseCommand, public FileCommand
{
public:
    SeamSeriesMetaDataCommand(const Poco::File& file, ProductInstanceMetaData::SeamSeriesMetaData &&metaData)
        : FileCommand(file)
        , m_metaData(std::move(metaData))
    {
    }

    void execute() override;

private:
    ProductInstanceMetaData::SeamSeriesMetaData m_metaData;
};

}
}

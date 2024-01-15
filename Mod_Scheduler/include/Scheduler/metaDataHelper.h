#pragma once

#include "videoRecorder/productInstanceMetaDataCommand.h"

#include <optional>


namespace precitec
{

namespace scheduler
{

/**
 * Small helper class to parse the ProductInstance meta data and update settings.
 **/
class MetaDataHelper
{
public:
    MetaDataHelper(const std::string &filePath);

    /**
     * Updates the TargetDirectoryPath with placeholders replaced by data from MetaData.
     **/
    void updateTargetPath(std::map<std::string, std::string> &settings);

    /**
     * Updates the TargetFileName with placeholders replaced by data from MetaData.
     **/
    void updateTargetFileName(std::map<std::string, std::string> &settings);

    /**
     * Updates the @p value with placeholders replaced by data from MetaData.
     **/
    void updateString(std::string &value);

private:
    std::optional<vdr::ProductInstanceMetaData> readMetaData(const std::string& filename);

    std::optional<vdr::ProductInstanceMetaData> m_metaData;
};

}
}

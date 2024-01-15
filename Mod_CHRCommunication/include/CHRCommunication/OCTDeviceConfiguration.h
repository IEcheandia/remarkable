/**
 *     @file
 *     @copyright    Precitec Vision GmbH & Co. KG
 *     @author       LB
 *     @date         2019
 *     @brief        list of CHRocodile parameters
 */

#ifndef OCTDEVICECONFIGURATION_H
#define OCTDEVICECONFIGURATION_H

#include "message/device.h"
#include "Poco/Util/XMLConfiguration.h"

namespace precitec
{
namespace grabber
{

/* To add a new parameter:
 *  1) define a new Id in OCTDeviceConfiguration::keyId. The order is not important, but the ids need to be consecutive
 *  2) define the corresponding string in defineKeys
 *  3) add a new member to OCTDeviceConfiguration
 *  4) define the association to the key and the min,max,default values in OCTDeviceConfiguration::get
 *  5) update initFromFile
 */

struct OCTDeviceConfiguration
{
    enum keyId
    {
        eSampleFrequency = 0,
        eLampIntensity,
        eDetectionWindowLeft,
        eDetectionWindowRight,
        eQualityThreshold,
        eDataAveraging,
        eSpectralAveraging,
        eWeldingDepthSystemOffset,
        eNumProfilesToSend,
        eNumBufferLines,
        NUMKEYS
    };
    typedef std::array<std::string, keyId::NUMKEYS> t_keylist;

    static const t_keylist s_KeyList;

    int m_oSampleFrequency;
    int m_oLampIntensity;
    int m_oDetectionWindow_Left;
    int m_oDetectionWindow_Right;
    int m_oQualityThreshold;
    int m_oDataAveraging;
    int m_oSpectralAveraging;
    int m_oWeldingDepthSystemOffset;
    int m_oNumProfilesToSend;
    int m_oNumBuferLines;

    static t_keylist defineKeys();

    interface::SmpKeyValue get(const std::string& p_rKey) const;
    int getInt(Poco::Util::XMLConfiguration* pConfIn, keyId keyId) const;
    double getDouble(Poco::Util::XMLConfiguration* pConfIn, keyId keyId) const;
    bool initFromFile(const std::string& p_rFilePath);
    interface::Configuration makeConfiguration() const;
    bool writeToFile(const std::string& p_rFilePath) const;
};

}
}

#endif

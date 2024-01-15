/**
 * @file
 * @brief   Headerfile zum DeviceServer von App_CHRCommunication
 *
 * @author  EA
 * @date    23.10.2017
 * @version 1.0
 */

#pragma once

#include "message/device.h"
#include "message/device.interface.h"

#include "CHRCommunication/CHRCommunication.h"

namespace precitec
{
using namespace interface;

namespace grabber
{

class DeviceServer : public TDevice<AbstractInterface>
{
public:
    DeviceServer(CHRCommunication& p_rCHRCommunication);
    virtual ~DeviceServer();

    //TDeviceInterface

    /// das Einschalten
    int initialize(Configuration const& config, int subDevice = 0);
    /// die Reset-Taste
    void uninitialize();
    /// kann beliebig oft durchgefuehrt werden
    void reinitialize();

    /// spezifischen Parameter setzen
    KeyHandle set(SmpKeyValue keyValue, int subDevice = 0);
    /// mehrere Parameter setzen
    void set(Configuration config, int subDevice = 0);

    /// spezifischen Parameter auslesen
    SmpKeyValue get(Key key, int subDevice = 0);
    /// spezifischen Parameter auslesen
    SmpKeyValue get(KeyHandle handle, int subDevice = 0);
    /// alle Parameter auslesen
    Configuration get(int subDevice = 0);

private:
    CHRCommunication& m_rCHRCommunication;
};

} // namespace grabber

} // namespace precitec

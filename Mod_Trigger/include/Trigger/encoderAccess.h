/**
 *   @file
 *   @copyright   Precitec Vision GmbH & Co. KG
 *   @author      Alexander Egger (EA)
 *   @date        2019
 *   @brief       Accesses encoder board for generating position dependend image trigger
 */

#ifndef ENCODERACCESS_H_
#define ENCODERACCESS_H_

#include <stdint.h>
#include <pthread.h>
#include <functional>

#include "PreciEncoder.h"

namespace precitec
{

namespace trigger
{

class EncoderAccess;

struct DataToEncoderAccessThread
{
    EncoderAccess *m_pEncoderAccess;
    int *m_pPreciEncoderDriverFd;
};

typedef std::function<uint32_t(uint32_t)> IRQCallbackFunction;

class EncoderAccess
{
public:

    /**
        * @brief CTor.
        * @param
        */
    EncoderAccess();

    /**
    * @brief DTor.
    */
    virtual ~EncoderAccess();

    void initEncoder (void);
    uint32_t readEncoder (void);
    void setEncoder (uint32_t value);
    void writeEncCompare (uint32_t value);
    void writeEncDivider (uint32_t value);
    void startEncInterrupt (void);
    void start_encoder_interrupt(void);
    void stopEncInterrupt (void);
    void stop_encoder_interrupt(void);

    void test (void);
    void IsrCallback (uint32_t p_oEncoderCounter);
    void connectCallback(IRQCallbackFunction p_pIRQCallbackFunction);

    int GetIRQCounter(void) {return IRQCounter;};

private:

    bool m_oIsImageTriggerViaEncoderSignals;
    int m_oPreciEncoderDriverFd;
    int IRQCounter;

    uint16_t m_oVendorId;       // Vendor ID
    uint16_t m_oDeviceId;       // Device ID
    uint16_t m_oSubVendorId;    // Subsystem Vendor ID
    uint16_t m_oSubDeviceId;    // Subsystem Device ID
    EncoderDeviceType m_oEncoderDevice;

    pthread_t m_oThread;
    struct DataToEncoderAccessThread m_oDataToEncoderAccessThread;

    IRQCallbackFunction m_pIRQCallbackFunction;
}; // class EncoderAccess

} // namespace trigger

} // namespace precitec

#endif /* ENCODERACCESS_H_ */


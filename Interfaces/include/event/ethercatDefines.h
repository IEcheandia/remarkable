/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		EA
 * 	@date		2016
 * 	@brief		Controls the EtherCAT bus
 */

#ifndef ETHERCATDEFINES_H_
#define ETHERCATDEFINES_H_

///////////////////////////////////////////////////////////
// Vendor/Device Liste
///////////////////////////////////////////////////////////

#define VENDORID_BECKHOFF       0x00000002
#define VENDORID_KUNBUS         0x00000569
#define VENDORID_HMS            0x0000001b
#define VENDORID_COPLEY         0x000000ab
#define VENDORID_PARKER         0x02000089
#define VENDORID_HILSCHER       0x00000044
#define VENDORID_PRECITEC       0x000000fe
#define VENDORID_MAXON          0x000000fb

#define PRODUCTCODE_EK1100           0x044c2c52
#define PRODUCTCODE_EL1018           0x03fa3052
#define PRODUCTCODE_EL2008           0x07d83052
#define PRODUCTCODE_EL3102           0x0c1e3052
#define PRODUCTCODE_EL3162           0x0c5a3052
#define PRODUCTCODE_EL3702           0x0e763052
#define PRODUCTCODE_EL4102           0x10063052
#define PRODUCTCODE_EL4132           0x10243052
#define PRODUCTCODE_EL5101           0x13ed3052
#define PRODUCTCODE_KUNBUS_GW        0x000186E9
#define PRODUCTCODE_ANYBUS_GW        0x0000003d
#define PRODUCTCODE_COMMUNICATOR_GW  0x0000003e
#define PRODUCTCODE_ACCELNET         0x00000380
#define PRODUCTCODE_COMPAX           0x002e3bf1
#define PRODUCTCODE_FIELDBUS         0x0f0f0f0f
#define PRODUCTCODE_EK1310           0x051e2c52
#define PRODUCTCODE_FRONTEND         0x044c0c62
#define PRODUCTCODE_EPOS4            0x63500000
#define PRODUCTCODE_EL5151           0x141F3052

///////////////////////////////////////////////////////////
// enums for Ethercat Interfaces
///////////////////////////////////////////////////////////

enum EcatInstance {eInstance1 = 1, eInstance2 = 2, eInstance3 = 3, eInstance4 = 4, eInstance5 = 5, eInstance6 = 6, eInstance7 = 7, eInstance8 = 8};

enum EcatChannel {eChannel1 = 1, eChannel2 = 2, eChannel3 = 3, eChannel4 = 4, eChannel5 = 5, eChannel6 = 6, eChannel7 = 7, eChannel8 = 8};

enum EcatProductIndex { eProductIndex_EK1100 = 1,
                        eProductIndex_EL1018 = 2,
                        eProductIndex_EL2008 = 3,
                        eProductIndex_EL3102 = 4,
                        eProductIndex_EL3702 = 5,
                        eProductIndex_EL4102 = 6,
                        eProductIndex_EL4132 = 7,
                        eProductIndex_EL5101 = 8,
                        eProductIndex_Kunbus_GW = 9,
                        eProductIndex_Anybus_GW = 10,
                        eProductIndex_ACCELNET = 11,
                        eProductIndex_COMPAX = 12,
                        eProductIndex_Fieldbus = 13,
                        eProductIndex_EK1310 = 14,
                        eProductIndex_Frontend = 15,
                        eProductIndex_EPOS4 = 16,
                        eProductIndex_EL5151 = 17,
                      };

///////////////////////////////////////////////////////////
// structs for Ethercat Interfaces
///////////////////////////////////////////////////////////

struct EcatAxisInput
{
    uint16_t statusWord;
    uint8_t  modesOfOpDisp;
    uint8_t  errorReg;
    uint32_t manufacStatus;
    int32_t  actualPosition;
    int32_t  actualVelocity;
    int16_t  actualTorque;
    uint16_t m_oDigitalInputs;
    uint16_t m_oDigitalOutputs;
    int32_t  m_oFollowingError;
};

struct EcatAxisOutput
{
    uint16_t controlWord;
    uint8_t  modesOfOp;
    int32_t  profileTargetPosition;
    int32_t  profileVelocity;
    int32_t  profileAcceleration;
    int32_t  profileDeceleration;
    uint8_t  homingMethod;
    int32_t  homingOffset;
    int32_t  homingVelocityFast;
    int32_t  homingVelocitySlow;
};

struct EcatFRONTENDOutput
{
    uint16_t m_oAmpCH1;
    uint16_t m_oAmpCH2;
    uint16_t m_oAmpCH3;
    uint16_t m_oAmpCH4;
    uint16_t m_oOversampling;
};

namespace precitec
{
namespace EtherCAT
{

struct DigitalIn
{
    DigitalIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t value)
        : productIndex(productIndex)
        , instance(instance)
        , value(value)
    {
    }

    EcatProductIndex productIndex;
    EcatInstance instance;
    uint8_t value;
};

struct AnalogIn
{
    AnalogIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t statusCH1, uint16_t valueCH1, uint8_t statusCH2, uint16_t valueCH2)
        : productIndex(productIndex)
        , instance(instance)
        , statusCH1(statusCH1)
        , valueCH1(valueCH1)
        , statusCH2(statusCH2)
        , valueCH2(valueCH2)
    {
    }
    EcatProductIndex productIndex;
    EcatInstance instance;
    uint8_t statusCH1;
    uint16_t valueCH1;
    uint8_t statusCH2;
    uint16_t valueCH2;
};

struct AnalogOversamplingIn
{
    AnalogOversamplingIn(EcatProductIndex productIndex, EcatInstance instance)
        : productIndex(productIndex)
        , instance(instance)
    {
    }
    EcatProductIndex productIndex;
    EcatInstance instance;
    std::vector<int16_t> channel1;
    std::vector<int16_t> channel2;
};

struct Gateway
{
    Gateway(EcatProductIndex productIndex, EcatInstance instance)
        : productIndex(productIndex)
        , instance(instance)
    {
    }
    EcatProductIndex productIndex;
    EcatInstance instance;
    std::vector<uint8_t> data;
};

struct Encoder
{
    Encoder(EcatProductIndex productIndex, EcatInstance instance, uint16_t status, uint32_t counterValue, uint32_t latchValue)
        : productIndex(productIndex)
        , instance(instance)
        , status(status)
        , counterValue(counterValue)
        , latchValue(latchValue)
    {
    }
    EcatProductIndex productIndex;
    EcatInstance instance;
    uint16_t status;
    uint32_t counterValue;
    uint32_t latchValue;
};

struct Axis
{
    Axis(EcatProductIndex productIndex, EcatInstance instance)
        : productIndex(productIndex)
        , instance(instance)
    {
    }
    EcatProductIndex productIndex;
    EcatInstance instance;
    EcatAxisInput axis;
};

struct LWM
{
    LWM(EcatProductIndex productIndex, EcatInstance instance)
        : productIndex(productIndex)
        , instance(instance)
    {
    }
    EcatProductIndex productIndex;
    EcatInstance instance;
    std::vector<uint16_t> plasma;
    std::vector<uint16_t> temperature;
    std::vector<uint16_t> backReference;
    std::vector<uint16_t> analog;

};

struct EcatInData
{
    std::vector<DigitalIn> digitalIn;
    std::vector<AnalogIn> analogIn;
    std::vector<AnalogOversamplingIn> oversampling;
    std::vector<Gateway> gateway;
    std::vector<Encoder> encoder;
    std::vector<Axis> axis;
    std::vector<LWM> lwm;
};

}
}

#endif /* ETHERCATDEFINES_H_ */


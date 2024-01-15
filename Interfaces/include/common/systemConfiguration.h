#ifndef SYSTEMCONFIGURATION_H_
#define SYSTEMCONFIGURATION_H_

#include "Poco/AutoPtr.h"

namespace Poco
{
namespace Util
{
class XMLConfiguration;
}
}

namespace precitec
{

namespace interface
{

enum TypeOfSensor
{
    eCoax = 0,
    eScheimpflug = 1,
    eLED = 2
};

enum ZCollimatorType {eZColliAnalog = 1, eZColliDigitalV1 = 2};
enum FieldbusType {eFieldbusProfinet = 1, eFieldbusEthernetIP = 2, eFieldbusEtherCAT = 3};
enum class SOUVIS6000MachineType {eS6K_SouRing = 0, eS6K_SouSpeed = 1, eS6K_SouBlate = 2};
enum class ScannerModel
{
    ScanlabScanner = 0,
    SmartMoveScanner,
};
enum class ScannerGeneralMode
{
    eScanMasterMode = 0,
    eScantracker2DMode = 1,
};
enum class LensType
{
    F_Theta_340 = 1,
    F_Theta_460,
    F_Theta_255
};

enum class CorrectionFileMode
{
    Welding = 1,
    Pilot,
    HeightMeasurement,
};

class SystemConfiguration
{
public:
enum class BooleanKey : std::size_t
{
    ImageMirrorActive,
    ImageMirrorYActive,
    HasCamera,
    LineLaser1Enable,
    LineLaser1_OutputViaCamera,
    LineLaser2Enable,
    LineLaser2_OutputViaCamera,
    FieldLight1Enable,
    FieldLight1_OutputViaCamera,
    LED_IlluminationEnable,
    ImageTriggerViaEncoderSignals,
    ImageTriggerViaCameraEncoder,
    ZCollimatorEnable,
    ZCollAutoHomingEnable,
    LiquidLensControllerEnable,
    ScanTrackerEnable,
    ScanTrackerSerialViaOnboardPort,
    LaserControlEnable,
    LaserControlTwoChannel,
    Scanner2DEnable,
    LaserPowerInputEnable,
    EncoderInput1Enable,
    EncoderInput2Enable,
    RobotTrackSpeedEnable,
    AxisXEnable,
    AxisYEnable,
    AxisZEnable,
    ForceHomingOfAxis,
    IDM_Device1Enable,
    OCT_with_reference_arms,
    CLS2_Device1Enable,
    Newson_Scanner1Enable,
    HeadMonitorGatewayEnable,
    HeadMonitorSendsNotReady,
    LWM40_No1_Enable,
    Communication_To_LWM_Device_Enable,
    FastAnalogSignal1Enable,
    FastAnalogSignal2Enable,
    FastAnalogSignal3Enable,
    FastAnalogSignal4Enable,
    FastAnalogSignal5Enable,
    FastAnalogSignal6Enable,
    FastAnalogSignal7Enable,
    FastAnalogSignal8Enable,
    EmergencyStopSignalEnable,
    CabinetTemperatureOkEnable,
    GenPurposeAnalogIn1Enable,
    GenPurposeAnalogIn2Enable,
    GenPurposeAnalogIn3Enable,
    GenPurposeAnalogIn4Enable,
    GenPurposeAnalogIn5Enable,
    GenPurposeAnalogIn6Enable,
    GenPurposeAnalogIn7Enable,
    GenPurposeAnalogIn8Enable,
    GenPurposeAnalogOut1Enable,
    GenPurposeAnalogOut2Enable,
    GenPurposeAnalogOut3Enable,
    GenPurposeAnalogOut4Enable,
    GenPurposeAnalogOut5Enable,
    GenPurposeAnalogOut6Enable,
    GenPurposeAnalogOut7Enable,
    GenPurposeAnalogOut8Enable,
    GenPurposeDigOut1Enable,
    SeamEndDigOut1Enable,
    GenPurposeDigOutMultipleEnable,
    GenPurposeDigIn1Enable,
    GenPurposeDigInMultipleEnable,
    ProductNumberFromWMEnable,
    ContinuouslyModeActive,
    EtherCATMasterIsActive,
    FieldbusViaSeparateFieldbusBoard,
    FieldbusTwoSeparateFieldbusBoards,
    FieldbusInterfaceStandard,
    FieldbusInterfaceLight,
    FieldbusInterfaceFull,
    FieldbusExtendedProductInfo,
    ProductTypeOutOfInterfaceFull,
    ProductNumberOutOfInterfaceFull,
    QualityErrorFieldOnSeamSeries,
    NIOResultSwitchedOff,
    HardwareCameraEnabled,
    SOUVIS6000_Application,
    SOUVIS6000_Is_PreInspection,
    SOUVIS6000_Is_PostInspection_Top,
    SOUVIS6000_Is_PostInspection_Bottom,
    SOUVIS6000_TCPIP_Communication_On,
    SOUVIS6000_Automatic_Seam_No,
    SOUVIS6000_CrossSectionMeasurementEnable,
    SOUVIS6000_CrossSection_Leading_System,
    SCANMASTER_Application,
    SCANMASTER_ThreeStepInterface,
    SCANMASTER_GeneralApplication,
    KeyCount
};

enum class IntKey : std::size_t
{
    Type_of_Sensor,
    LineLaser_CameraCommandSet,
    LED_CONTROLLER_TYPE,
    LED_MaxCurrent_mA_Chan1,
    LED_MaxCurrent_mA_Chan2,
    LED_MaxCurrent_mA_Chan3,
    LED_MaxCurrent_mA_Chan4,
    LED_MaxCurrent_mA_Chan5,
    LED_MaxCurrent_mA_Chan6,
    LED_MaxCurrent_mA_Chan7,
    LED_MaxCurrent_mA_Chan8,
    CameraEncoderBurstFactor,
    ZCollimatorType,
    ScannerModel,
    ScannerGeneralMode,
    FieldbusBoard_TypeOfFieldbus,
    SOUVIS6000_Machine_Type,
    CameraInterfaceType,
    ScanlabScanner_Lens_Type,
    Maximum_Simulated_Laser_Power,
    Scanner2DController,
    LWM_Device_TCP_Port,
    KeyCount
};

enum class StringKey : std::size_t
{
    ZCollimator_IP_Address,
    LiquidLensController_IP_Address,
    Scanner2DController_IP_Address,
    LWM_Device_IP_Address,
    IDM_Device1_IP_Address,
    CLS2_Device1_IP_Address,
    SOUVIS6000_IP_Address_MachineControl,
    SOUVIS6000_IP_Address_CrossSection_Other,
    FieldbusBoard_IP_Address,
    FieldbusBoard_2_IP_Address,
    FieldbusBoard_Netmask,
    FieldbusBoard_2_Netmask,
    ScanTrackerSerialOnboardPort,
    ScanlabScanner_Correction_File,
    KeyCount
};

private:
    SystemConfiguration();
    Poco::AutoPtr<Poco::Util::XMLConfiguration> m_configuration;
    std::string m_saveFilePath;
    void save();

protected:
    SystemConfiguration(const std::string &loadFilePath, const std::string &saveFilePath = {});

public:
    virtual ~SystemConfiguration();
    static SystemConfiguration &instance();

    std::string getString(const std::string& key, const std::string& defaultValue, bool *found = nullptr);
    bool getBool(const std::string& key, bool defaultValue, bool *found = nullptr);
    int getInt(const std::string& key, int defaultValue, bool *found = nullptr);
    double getDouble(const std::string& key, double defaultValue, bool *found = nullptr);
    void setString(const std::string& key, std::string value);
    void setInt(const std::string& key, int value);
    void setBool(const std::string& key, bool value);
    void setDouble(const std::string& key, double value);

    std::string get(StringKey key);
    bool get(BooleanKey key);
    int get(IntKey key);
    void set(BooleanKey key, bool value);
    void set(IntKey key, int value);
    void set(StringKey key, const std::string& value);

    void toStdOut();

    static std::string keyToString(BooleanKey key);
    static std::string keyToString(IntKey key);
    static std::string keyToString(StringKey key);
    static BooleanKey stringToBooleanKey(const std::string& key);
    static IntKey stringToIntKey(const std::string& key);
    static StringKey stringToStringKey(const std::string& key);
    static bool getDefault(BooleanKey key);
    static int getDefault(IntKey key);
    static std::string getDefault(StringKey key);
};

} // namespace interface

} // namespace precitec

#endif /*SYSTEMCONFIGURATION_H_*/


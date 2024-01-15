/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		AL, EA, HS
 * 	@date		2010
 * 	@brief		Controls welding head hardware and extern sensors via ethercat bus.
 */


#ifndef WELDINGHEADCONTROL_H_
#define WELDINGHEADCONTROL_H_

#include <viWeldHead/StateMachine.h>
#include <viWeldHead/StateMachineV2.h>
#include <iostream>
#include <map>
#include <atomic>

#include "GlobalDefs.h"

#include "Poco/Version.h"
#include "Poco/DOM/Document.h"
#include "Poco/DOM/Element.h"
#include "Poco/DOM/Text.h"
#include "Poco/DOM/DOMWriter.h"
#include "Poco/XML/XMLWriter.h"
#include "Poco/FileStream.h"
#include "Poco/DOM/NodeList.h"
#include "Poco/DOM/NamedNodeMap.h"

#include "Poco/DOM/DOMParser.h"
#include "Poco/DOM/NodeIterator.h"
#include "Poco/DOM/NodeFilter.h"
#include "Poco/DOM/AutoPtr.h"
#include "Poco/SAX/InputSource.h"
#include "Poco/SAX/XMLReader.h"
#include "Poco/Exception.h"
#include "Poco/SAX/SAXParser.h"
#include "Poco/SAX/LexicalHandler.h"
#include "Poco/UUID.h"
#include "Poco/Util/Application.h"

#include "Poco/Exception.h"
#include "Poco/Mutex.h"
#include "viWeldHead/SAX_VIConfigParser.h"
#include "viWeldHead/outMotionDataServer.h"
#include "event/viWeldHeadSubscribe.h"
#include "event/viWeldHeadSubscribe.interface.h"
#include "event/viWeldHeadSubscribe.proxy.h"
#include "message/weldHead.interface.h"

#include "event/sensor.h"
#include "event/sensor.proxy.h"

#include "event/ethercatInputs.h"

#include "event/ethercatOutputs.proxy.h"
#include "geo/point.h"
#include "common/triggerContext.h"
#include "common/triggerInterval.h"

#include "viWeldHead/serialToTracker.h"
#include "viWeldHead/DataToLaserControl.h"

#include "viWeldHead/LEDControl/LEDI_ExportedDatatypes.hpp"

#ifndef LED_DRIVER_DISABLE_COMPLETE_CODE

#include "viWeldHead/LEDControl/LEDcomEth1.h"
//#include "viWeldHead/LEDControl/LEDcomSerial1.h"
#include "viWeldHead/LEDControl/LEDdriver.h"
//#include "viWeldHead/LEDControl/LEDdriverHW_PP860.h"
#include "viWeldHead/LEDControl/LEDdriverHW_PP420F.h"
#include "viWeldHead/LEDControl/LEDdriverHW_PP520.h"
#include "viWeldHead/LEDControl/LEDdriverHW_PP820.h"
#include "viWeldHead/LEDControl/lensDriverTRCL180.h"

#endif

#include "viWeldHead/Scanlab/Scanlab.h"

#include "viWeldHead/RingBuffer.h"

#include "viWeldHead/ZCollimatorV2/TmlCommunicator.h"

#include "viWeldHead/WeldHeadDefaults.h"

#include "common/systemConfiguration.h"

namespace precitec
{
	using viWeldHead::OutMotionDataServer;
	using namespace interface;
	using Poco::FastMutex;
	using namespace hardware;

namespace ethercat
{

#define LENGTH_OF_DEBUGFILE2  50000

struct TrackerCalibData
{
	int m_oFrequency; // in Hz
	double m_oMaxVolt;
    int m_oMaxWidth250; // in um (250mm)
    int m_oMaxWidth; // in um
    double m_oGradient; // in V/um
};

enum EncoderNumberType {eEncoderInputNo1 = 1, eEncoderInputNo2 = 2};
enum AnalogOutputNo {eAnalogOut1 = 0, eAnalogOut2 = 1, eAnalogOut3 = 2, eAnalogOut4 = 3, eAnalogOut5 = 4, eAnalogOut6 = 5, eAnalogOut7 = 6, eAnalogOut8 = 7, eLastAnalogOut = eAnalogOut8};
enum AnalogInputNo {eAnalogIn1 = 0, eAnalogIn2 = 1, eAnalogIn3 = 2, eAnalogIn4 = 3, eAnalogIn5 = 4, eAnalogIn6 = 5, eAnalogIn7 = 6, eAnalogIn8 = 7, eLastAnalogIn = eAnalogIn8};

class WeldingHeadControl;

struct DataToSendSensorDataThread
{
    WeldingHeadControl* m_pWeldingHeadControl;
    unsigned long m_oCycleTimeSendSensorData_ns;
};

struct DataToZCPositionDigV1ReadOutThread
{
    WeldingHeadControl* m_pWeldingHeadControl;
};

class WeldingHeadControl  : public TviWeldHeadSubscribe<AbstractInterface>,public TWeldHeadMsg<AbstractInterface>
{

public:
	WeldingHeadControl(OutMotionDataServer& outServer,
						TSensor<AbstractInterface> &sensorProxy,
						TEthercatOutputs<EventProxy>& p_rEthercatOutputsProxy);
	virtual ~WeldingHeadControl();

	//get from publisher
	virtual void SetTrackerScanWidthControlled(int value);      // Interface: viWeldHeadSubscribe (event)
	virtual void SetTrackerScanPosControlled(int value);        // Interface: viWeldHeadSubscribe (event)
	virtual void SetTrackerDriverOnOff(bool onOff);             // Interface: viWeldHeadSubscribe (event)
	virtual void doZCollDriving(DrivingType p_oDrivingType, int value); // Interface: viWeldHeadSubscribe (event)
	virtual void SetHeadMode(HeadAxisID axis, MotionMode mode, bool bHome); // Interface: viWeldHeadSubscribe (event)
	virtual void SetHeadValue(HeadAxisID axis, int value, MotionMode mode); // Interface: viWeldHeadSubscribe (event)
	virtual void RequestHeadInfo(HeadAxisID axis);              // Interface: viWeldHeadSubscribe (event)
	virtual void SetLCPowerOffset(int value);                   // Interface: viWeldHeadSubscribe (event)
	virtual void SetGenPurposeAnaOut(OutputID outputNo, int value); // Interface: viWeldHeadSubscribe (event)
	virtual void ScanmasterResult(ScanmasterResultType p_oResultType, ResultDoubleArray const& p_rResultDoubleArray); // Interface: viWeldHeadSubscribe (event)
	void ScanmasterHeight(double height) override;  // Interface: viWeldHeadSubscribe (event)
	virtual bool getLEDEnable(LEDPanelNo ledPanel);
	virtual bool setLEDEnable(LEDPanelNo ledPanel, bool enabled);
    bool reloadFiberSwitchCalibration(void) override;
    bool weldForScannerCalibration(const std::vector<geo2d::DPoint>& points, double laserPowerInPct, double durationInMs, double jumpSpeedInMmPerSec) override;

	//send to subscriber
	void HeadValueReached(HeadAxisID axis, MotionMode mode, int value);
	void HeadIsReady(HeadAxisID axis, MotionMode mode);
	void HeadError(HeadAxisID axis, ErrorCode errorCode, int value);
	void trackerInfo(TrackerInfo p_oTrackerInfo);

	bool setHeadPos(HeadAxisID axis, int value);           // Interface: weldHead (message)
	bool setHeadMode(HeadAxisID axis, MotionMode mode, bool bHome); // Interface: weldHead (message)
	int getHeadPosition(HeadAxisID axis);                  // Interface: weldHead (message)
	int getLowerLimit(HeadAxisID axis);                    // Interface: weldHead (message)
	int getUpperLimit(HeadAxisID axis);                    // Interface: weldHead (message)
	bool doZCollHoming(void);                              // Interface: weldHead (message)
	bool doZCollDrivingRelative(int value) override;       // Interface: weldHead (message)


	void SetFieldLight1Intensity(void);
	void SetLineLaser1Intensity(void);
	void SetLineLaser2Intensity(void);

	void SetLEDPanelIntensity(void);
	void SetLEDPanelPulseWidth(void);
	void LED_SaveIntensityPersistent(void);
    void sendLEDData();

    void StartSendSensorDataThread(unsigned long p_oCycleTimeSendSensorData_ns);
    void StopSendSensorDataThread(void);
    void SendSensorDataFunction(void);

	/**
	 * Passt das Senden der Sensordaten an die Bildfrequenz an.
	 * @param ids Sensor IDs
	 * @param context TriggerContext
	 * @param interval Interval
	 */
	void burst(const std::vector<int>& ids, TriggerContext const& context, TriggerInterval const& interval);
	void cancel(int id);

	void startAutomaticmode(uint32_t producttype, uint32_t productnumber); // Interface inspection
	void stopAutomaticmode(void);      // Interface inspection
	void start(int seamnumber);        // Interface inspection
	void end(int seamnumber);          // Interface inspection
	void info(int seamsequence);       // Interface inspection

	void ecatDigitalIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t value);           // Interface EthercatInputs
	void ecatAnalogIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t statusCH1, uint16_t valueCH1, uint8_t statusCH2, uint16_t valueCH2); // Interface EthercatInputs
	void ecatAnalogOversamplingInCH1(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecINT16 &data); // Interface EthercatInputs
	void ecatAnalogOversamplingInCH2(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecINT16 &data); // Interface EthercatInputs
	void ecatEncoderIn(EcatProductIndex productIndex, EcatInstance instance, uint16_t status, uint32_t counterValue, uint32_t latchValue); // Interface EthercatInputs
	void ecatLWMCh1PlasmaIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecUINT16 &data); // Interface EthercatInputs
	void ecatLWMCh2TempIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecUINT16 &data); // Interface EthercatInputs
	void ecatLWMCh3BackRefIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecUINT16 &data); // Interface EthercatInputs
	void ecatLWMCh4AnalogIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecUINT16 &data); // Interface EthercatInputs

	bool isLineLaser1Enabled(void) { return m_oLineLaser1Enable; }
	bool isLineLaser1OutputViaCamera(void) { return m_oLineLaser1_OutputViaCamera; }
	int getLineLaser1Intensity() { return m_oLineLaser1Intensity; }
	void setLineLaser1Intensity(int p_oIntensity) { m_oLineLaser1Intensity = p_oIntensity; }
	bool getLineLaser1OnOff() { return m_oLineLaser1OnOff; }
	void setLineLaser1OnOff(bool p_oOnOff) { m_oLineLaser1OnOff = p_oOnOff; }
	bool isLineLaser2Enabled(void) { return m_oLineLaser2Enable; }
	bool isLineLaser2OutputViaCamera(void) { return m_oLineLaser2_OutputViaCamera; }
	int getLineLaser2Intensity() { return m_oLineLaser2Intensity; }
	void setLineLaser2Intensity(int p_oIntensity) { m_oLineLaser2Intensity = p_oIntensity; }
	bool getLineLaser2OnOff() { return m_oLineLaser2OnOff; }
	void setLineLaser2OnOff(bool p_oOnOff) { m_oLineLaser2OnOff = p_oOnOff; }
	bool isFieldLight1Enabled(void) { return m_oFieldLight1Enable; }
	bool isFieldLight1OutputViaCamera(void) { return m_oFieldLight1_OutputViaCamera; }
	int getFieldLight1Intensity() { return m_oFieldLight1Intensity; }
	void setFieldLight1Intensity(int p_oIntensity) { m_oFieldLight1Intensity = p_oIntensity; }
	bool getFieldLight1OnOff() { return m_oFieldLight1OnOff; }
	void setFieldLight1OnOff(bool p_oOnOff) { m_oFieldLight1OnOff = p_oOnOff; }

	bool isGenPurposeAnaInEnabled(AnalogInputNo p_oAnalogInput) { return m_oGenPurposeAnaInEnable[p_oAnalogInput]; }
	bool isGenPurposeAnaOutEnabled(AnalogOutputNo p_oAnalogOutput) { return m_oGenPurposeAnaOutEnable[p_oAnalogOutput]; }

	int  getLEDPanelIntensity(LEDPanelNo p_oPanelNo) { return m_oLEDI_Parameters[p_oPanelNo].led_brightness ; }
	void setLEDPanelIntensity(LEDPanelNo p_oPanelNo, int p_oIntensity);
	bool getLEDPanelOnOff(LEDPanelNo p_oPanelNo) { return m_oLEDI_Parameters[p_oPanelNo].led_enable; }
	void setLEDPanelOnOff(LEDPanelNo p_oPanelNo, bool p_oOnOff);
	int  getLEDPanelPulseWidth(LEDPanelNo p_oPanelNo) { return m_oLEDI_Parameters[p_oPanelNo].led_pulse_width ; }
	void setLEDPanelPulseWidth(LEDPanelNo p_oPanelNo, int p_oPulseWidth);
    int getLEDControllerType() { return m_oLEDControllerType; }

	int getRequiredPositionX() { return m_oRequiredPositionX; }
	int getRequiredPositionY() { return m_oRequiredPositionY; }
	int getRequiredPositionZ() { return m_oRequiredPositionZ; }

	SerialToTracker* getSerialToTracker() { return m_pSerialToTracker; }
	DataToLaserControl* getDataToLaserControl() { return m_pDataToLaserControl; }

	void SetFocalLength(int p_oFocalLength);
	int GetFocalLength(void) { return m_oFocalLength; }

	void SetTrackerFrequencyStep(TrackerFrequencyStep p_oFreq);
	TrackerFrequencyStep GetTrackerFrequencyStep(void) { return m_oTrackerFrequencyStep; }
	void SetTrackerFrequencyCont(unsigned short p_oFreq);
	unsigned short GetTrackerFrequencyCont(void) { return m_oTrackerFrequencyCont; }

	bool GetTrackerDriverOnOff() { return m_oTrackerDriverOnOff; }
	void SetTrackerExpertMode(bool onOff);
	bool GetTrackerExpertMode() { return m_oTrackerExpertMode; }

	void SetScanWidthOutOfGapWidth(bool onOff);
	bool GetScanWidthOutOfGapWidth() { return m_oScanWidthOutOfGapWidth; }
	void SetTrackerScanWidth(bool p_oFailSafeRequest);
	void SetTrackerScanWidthFixed(int value);
	int GetScanWidthFixed() { return m_oScanWidthFixed; }
	void SetGapWidthToScanWidthOffset(int value) { m_oGapWidthToScanWidthOffset = value; }
	int GetGapWidthToScanWidthOffset() { return m_oGapWidthToScanWidthOffset; }
	void SetGapWidthToScanWidthGradient(int value) { m_oGapWidthToScanWidthGradient = value; }
	int GetGapWidthToScanWidthGradient() { return m_oGapWidthToScanWidthGradient; }

	void SetScanPosOutOfGapPos(bool onOff);
	bool GetScanPosOutOfGapPos() { return m_oScanPosOutOfGapPos; }
	void SetTrackerScanPos(bool p_oFailSafeRequest);
	void SetTrackerScanPosFixed(int value);
	int GetScanPosFixed() { return m_oScanPosFixed; }
	void SetTrackerMaxAmplitude(int p_oTrackerMaxAmplitude);
	int GetTrackerMaxAmplitude() { return m_oTrackerMaxAmplitude; }

    Scanlab* getScanlab() { return m_pScanlab; }

	StateMachineInterface* getStatesHeadX() { return m_pStatesHeadX; }
	StateMachineInterface* getStatesHeadY() { return m_pStatesHeadY; }
	StateMachineInterface* getStatesHeadZ() { return m_pStatesHeadZ; }
	int getInstanceAxisX() { return m_oConfigParser.getInstanceAxisX(); }
	int getInstanceAxisY() { return m_oConfigParser.getInstanceAxisY(); }
	int getInstanceAxisZ() { return m_oConfigParser.getInstanceAxisZ(); }
    bool getDebugInfo_AxisController(void) { return m_oDebugInfo_AxisController; };
    void setDebugInfo_AxisController(bool p_oDebugInfo_AxisController);

	void ClearEncoderCounter(EncoderNumberType p_oEncoderNo);

	bool isLED_IlluminationEnabled(void) { return m_oLED_IlluminationEnable; }

	bool isScanTrackerEnabled(void) { return m_oScanTrackerEnable; }
	bool isLaserControlEnabled(void) { return m_oLaserControlEnable; }
	bool isZCollimatorEnabled(void) { return m_oZCollimatorEnable; }
	precitec::interface::ZCollimatorType getZCollimatorType(void) { return m_oZCollimatorType; }

    bool isLiquidLensControllerEnabled(void)
    {
        return m_liquidLensControllerEnable;
    }

	bool isLaserPowerInputEnabled(void) { return m_oLaserPowerInputEnable; }
	bool isEncoderInput1Enabled(void) { return m_oEncoderInput1Enable; }
	bool isEncoderInput2Enabled(void) { return m_oEncoderInput2Enable; }
	bool isRobotTrackSpeedEnabled(void) { return m_oRobotTrackSpeedEnable; }

	void SetZCRefTravel(bool p_oOnOff);
	void SetZCAutomatic(bool p_oOnOff);
	void SetZCAnalogInValue(int p_oValue);
	int GetZCActPosition() { return m_oZCActPosition; }
	void SetZCSystemOffset(double p_oZCSystemOffset);
	double GetZCSystemOffset() { return m_oZCSystemOffset; }
	void SetZCLensToFocusRatio(double p_oZCLensToFocusRatio);
	double GetZCLensToFocusRatio() { return m_oZCLensToFocusRatio; }

    void setLiquidLensPosition(double value);
    double getLiquidLensPosition(void)
    {
        return m_liquidLensPosition;
    }

    void ZCWaitForEndOfDriving(void);
    int GetZCActualPositionV2();

    void StartZCPositionDigV1ReadOutThread(void);
    void StopZCPositionDigV1ReadOutThread(void);
    void ZCPositionDigV1ReadOutFunction(void);

	void SetLCStartSignal(bool p_oOnOff);
	bool GetLCStartSignal() { return m_oLCStartSignal; }

	bool isAxisXEnabled(void) { return m_oAxisXEnable; }
	bool isAxisYEnabled(void) { return m_oAxisYEnable; }
	bool isAxisZEnabled(void) { return m_oAxisZEnable; }

	bool isFastAnalogSignal1Enabled(void) { return m_oFastAnalogSignal1Enable; }
	bool isFastAnalogSignal2Enabled(void) { return m_oFastAnalogSignal2Enable; }
	bool isFastAnalogSignal3Enabled(void) { return m_oFastAnalogSignal3Enable; }
	bool isFastAnalogSignal4Enabled(void) { return m_oFastAnalogSignal4Enable; }
	bool isFastAnalogSignal5Enabled(void) { return m_oFastAnalogSignal5Enable; }
	bool isFastAnalogSignal6Enabled(void) { return m_oFastAnalogSignal6Enable; }
	bool isFastAnalogSignal7Enabled(void) { return m_oFastAnalogSignal7Enable; }
	bool isFastAnalogSignal8Enabled(void) { return m_oFastAnalogSignal8Enable; }

	bool isLWM40_No1_Enabled(void) { return m_oLWM40_No1_Enable; }

    int GetLWM40_No1_AmpPlasma(void);
    int GetLWM40_No1_AmpTemperature(void);
    int GetLWM40_No1_AmpBackReflection(void);
    int GetLWM40_No1_AmpAnalogInput(void);
    void SetLWM40_No1_AmpPlasma(int p_oValue);
    void SetLWM40_No1_AmpTemperature(int p_oValue);
    void SetLWM40_No1_AmpBackReflection(int p_oValue);
    void SetLWM40_No1_AmpAnalogInput(int p_oValue);

    bool isScanlabScannerEnabled(void) { return m_oScanlabScannerEnable; }
    interface::ScannerGeneralMode getScannerGeneralMode(void)
    {
        return m_scannerGeneralMode;
    }
    interface::ScannerModel getScannerModel(void)
    {
        return m_scannerModel;
    }

    bool isSOUVIS6000_Application(void) { return m_oIsSOUVIS6000_Application; }

    void saveKeyValueToWeldHeadDefaults(const std::string& key, int value);

private:
	OutMotionDataServer& m_outMotionDataServer;
	TEthercatOutputs<EventProxy>& m_rEthercatOutputsProxy;

	StateMachineInterface* m_pStatesHeadX;
	StateMachineInterface* m_pStatesHeadY;
	StateMachineInterface* m_pStatesHeadZ;
    bool m_oDebugInfo_AxisController;

	VIMainCallback<WeldingHeadControl> *m_cbHeadValueReached;
	VIMainCallback<WeldingHeadControl> *m_cbHeadIsReady;
	VIMainCallback<WeldingHeadControl> *m_cbHeadError;

    // analog inputs
	SLAVE_PROXY_INFORMATION m_oGenPurposeAnaInProxyInfo[eLastAnalogIn + 1];
    std::atomic<int16_t> m_oGenPurposeAnaIn[eLastAnalogIn + 1];
	SLAVE_PROXY_INFORMATION m_oLaserPowerSignalProxyInfo;
    std::atomic<int16_t> m_oLaserPowerSignal;
	SLAVE_PROXY_INFORMATION m_oRobotTrackSpeedProxyInfo;
    std::atomic<int16_t> m_oRobotTrackSpeed;
    // analog inputs oversampling
	SLAVE_PROXY_INFORMATION m_oOversamplingSignal1ProxyInfo;
    Poco::FastMutex m_oOversamplingSignal1RingBufferMutex;
    RingBuffer<int16_t> m_oOversamplingSignal1RingBuffer;
	SLAVE_PROXY_INFORMATION m_oOversamplingSignal2ProxyInfo;
    Poco::FastMutex m_oOversamplingSignal2RingBufferMutex;
    RingBuffer<int16_t> m_oOversamplingSignal2RingBuffer;
	SLAVE_PROXY_INFORMATION m_oOversamplingSignal3ProxyInfo;
    Poco::FastMutex m_oOversamplingSignal3RingBufferMutex;
    RingBuffer<int16_t> m_oOversamplingSignal3RingBuffer;
	SLAVE_PROXY_INFORMATION m_oOversamplingSignal4ProxyInfo;
    Poco::FastMutex m_oOversamplingSignal4RingBufferMutex;
    RingBuffer<int16_t> m_oOversamplingSignal4RingBuffer;
	SLAVE_PROXY_INFORMATION m_oOversamplingSignal5ProxyInfo;
    Poco::FastMutex m_oOversamplingSignal5RingBufferMutex;
    RingBuffer<int16_t> m_oOversamplingSignal5RingBuffer;
	SLAVE_PROXY_INFORMATION m_oOversamplingSignal6ProxyInfo;
    Poco::FastMutex m_oOversamplingSignal6RingBufferMutex;
    RingBuffer<int16_t> m_oOversamplingSignal6RingBuffer;
	SLAVE_PROXY_INFORMATION m_oOversamplingSignal7ProxyInfo;
    Poco::FastMutex m_oOversamplingSignal7RingBufferMutex;
    RingBuffer<int16_t> m_oOversamplingSignal7RingBuffer;
	SLAVE_PROXY_INFORMATION m_oOversamplingSignal8ProxyInfo;
    Poco::FastMutex m_oOversamplingSignal8RingBufferMutex;
    RingBuffer<int16_t> m_oOversamplingSignal8RingBuffer;
    // inputs LWM40
	SLAVE_PROXY_INFORMATION m_oLWM40_1_PlasmaProxyInfo;
    Poco::FastMutex m_oLWM40_1_PlasmaRingBufferMutex;
    RingBuffer<uint16_t> m_oLWM40_1_PlasmaRingBuffer;
	SLAVE_PROXY_INFORMATION m_oLWM40_1_TemperatureProxyInfo;
    Poco::FastMutex m_oLWM40_1_TemperatureRingBufferMutex;
    RingBuffer<uint16_t> m_oLWM40_1_TemperatureRingBuffer;
	SLAVE_PROXY_INFORMATION m_oLWM40_1_BackReflectionProxyInfo;
    Poco::FastMutex m_oLWM40_1_BackReflectionRingBufferMutex;
    RingBuffer<uint16_t> m_oLWM40_1_BackReflectionRingBuffer;
	SLAVE_PROXY_INFORMATION m_oLWM40_1_AnalogInputProxyInfo;
    Poco::FastMutex m_oLWM40_1_AnalogInputRingBufferMutex;
    RingBuffer<uint16_t> m_oLWM40_1_AnalogInputRingBuffer;
    // analog outputs
	SLAVE_PROXY_INFORMATION m_oLineLaser1IntensityProxyInfo;
	SLAVE_PROXY_INFORMATION m_oLineLaser2IntensityProxyInfo;
	SLAVE_PROXY_INFORMATION m_oFieldLight1IntensityProxyInfo;
	SLAVE_PROXY_INFORMATION m_oGenPurposeAnaOutProxyInfo[eLastAnalogOut + 1];
	SLAVE_PROXY_INFORMATION m_oTrackerScanWidthProxyInfo;
	SLAVE_PROXY_INFORMATION m_oTrackerScanPosProxyInfo;
	SLAVE_PROXY_INFORMATION m_oZCAnalogInProxyInfo;
	SLAVE_PROXY_INFORMATION m_oLCPowerOffsetProxyInfo;
    // digital inputs
	SLAVE_PROXY_INFORMATION m_oSTScannerOkProxyInfo;
	SLAVE_PROXY_INFORMATION m_oSTScannerLimitsProxyInfo;
	SLAVE_PROXY_INFORMATION m_oZCErrorProxyInfo;
	SLAVE_PROXY_INFORMATION m_oZCPosReachedProxyInfo;
	SLAVE_PROXY_INFORMATION m_oLCErrorSignalProxyInfo;
	SLAVE_PROXY_INFORMATION m_oLCReadySignalProxyInfo;
	SLAVE_PROXY_INFORMATION m_oLCLimitWarningProxyInfo;
	SLAVE_PROXY_INFORMATION m_oLEDTemperatureHighProxyInfo;
    // digital outputs
	SLAVE_PROXY_INFORMATION m_oSTEnableDriverProxyInfo;
	SLAVE_PROXY_INFORMATION m_oZCRefTravelProxyInfo;
	SLAVE_PROXY_INFORMATION m_oZCAutomaticProxyInfo;
	SLAVE_PROXY_INFORMATION m_oLCStartSignalProxyInfo;
    // encoder inputs
	SLAVE_PROXY_INFORMATION m_oEncoderInput1ProxyInfo;
    std::atomic<uint32_t> m_oEncoderInput1;
	SLAVE_PROXY_INFORMATION m_oEncoderInput2ProxyInfo;
    std::atomic<uint32_t> m_oEncoderInput2;
    // encoder outputs (reset)
	SLAVE_PROXY_INFORMATION m_oEncoderReset1ProxyInfo;
	SLAVE_PROXY_INFORMATION m_oEncoderReset2ProxyInfo;

	void activateAnalogOutput(SLAVE_PROXY_INFORMATION &p_rProxyInfo, const std::string &p_rInfoText);
    void activateAnalogInput(SLAVE_PROXY_INFORMATION &p_rProxyInfo, const std::string &p_rInfoText);
    void activateAnalogOversamplingInput(SLAVE_PROXY_INFORMATION &p_rProxyInfo, const std::string &p_rInfoText);
    void activateLWM40Input(SLAVE_PROXY_INFORMATION &p_rProxyInfo, const std::string &p_rInfoText);
    void activateDigitalOutput(SLAVE_PROXY_INFORMATION &p_rProxyInfo, const std::string &p_rInfoText);
    void activateDigitalInput(SLAVE_PROXY_INFORMATION &p_rProxyInfo, const std::string &p_rInfoText);
    void activateEncoderInput(SLAVE_PROXY_INFORMATION &p_rProxyInfo, const std::string &p_rInfoText);
    void activateEncoderOutput(SLAVE_PROXY_INFORMATION &p_rProxyInfo, const std::string &p_rInfoText);

    void IncomingGenPurposeAnaIn_V2(AnalogInputNo p_oInput);
    void IncomingLaserPowerSignal_V2(void);
    void IncomingRobotTrackSpeed_V2(void);
    void IncomingScannerXPosition_V2(void);
    void IncomingScannerYPosition_V2(void);
    void IncomingFiberSwitchPosition_V2(void);
    void IncomingZCPositionDigV1_V2(void);
    void IncomingScannerWeldingFinished_V2(void);
    void IncomingContourPreparedFinished(void);

    void IncomingOversamplingSignal1_V2(void);
    void IncomingOversamplingSignal2_V2(void);
    void IncomingOversamplingSignal3_V2(void);
    void IncomingOversamplingSignal4_V2(void);
    void IncomingOversamplingSignal5_V2(void);
    void IncomingOversamplingSignal6_V2(void);
    void IncomingOversamplingSignal7_V2(void);
    void IncomingOversamplingSignal8_V2(void);

    void IncomingLWM40_1_Plasma_V2(void);
    void IncomingLWM40_1_Temperature_V2(void);
    void IncomingLWM40_1_BackReflection_V2(void);
    void IncomingLWM40_1_AnalogInput_V2(void);

	void IncomingScannerOK(unsigned char p_oValue);
	void IncomingScannerLimits(unsigned char p_oValue);
	void IncomingZCError(unsigned char p_oValue);
	void IncomingZCPosReached(unsigned char p_oValue);
	void IncomingLCErrorSignal(unsigned char p_oValue);
	void IncomingLCReadySignal(unsigned char p_oValue);
	void IncomingLCLimitWarning(unsigned char p_oValue);

	void IncomingLEDTemperatureHigh(unsigned char p_oValue);

	void IncomingEncoderInput1_V2(void);
	void IncomingEncoderInput2_V2(void);

    int parseTrackerCalibrationFile(std::string& CalibFilename);
	void interpretTrackerCalibLine(std::string& p_rFullLine, std::string& p_rPart1, std::string& p_rPart2, std::string& p_rPart3);
	void CalculateFocalLengthDependencies(void);

	void cleanBuffer();

	int m_oStartBitScannerOK;
	int m_oStartBitScannerLimits;

	int m_oEncoderRevolutions;
	int m_oStartBitZCError;
	int m_oStartBitZCPosReached;

	bool m_oZCRefTravelIsActive;
	bool m_oZCAutomaticIsActive;
	bool m_oZCError;
	bool m_oZCPosReached;
	Poco::Event m_oZCRefTravelEvent;
	int m_oZCStateVarAutomatic;
	int m_oZCStateVarRefTravel;
	int m_oZCNewPositionValue;
	int m_oZCActPosition;
    std::atomic<double> m_oZCSystemOffset;
    std::atomic<double> m_oZCLensToFocusRatio;

    tml::TmlCommunicator m_oTmlCommunicator;
    double m_oNextAbsolutePositionV2Um;
    std::atomic<bool> m_oZCDrivingV2IsActive;
    std::atomic<bool> m_oZCHomingV2IsActive;

    pthread_t m_oZCPositionDigV1ReadOutThread_ID;
    struct DataToZCPositionDigV1ReadOutThread m_oDataToZCPositionDigV1ReadOutThread;
    std::atomic<int32_t> m_oZCPositionDigV1ReadOut;

	int m_oStartBitLCErrorSignal;
	int m_oStartBitLCReadySignal;
	int m_oStartBitLCLimitWarning;

	int m_oStartBitLEDTemperatureHigh;

	SAX_VIConfigParser m_oConfigParser;

	bool m_bGlasNotPresent;
	bool m_bGlasDirty;
	bool m_bTempGlasFail;
	bool m_bTempHeadFail;

	TSensor<AbstractInterface>    &m_oSensorProxy;

    pthread_t m_oSendSensorDataThread_ID;
    struct DataToSendSensorDataThread m_oDataToSendSensorDataThread;

	TriggerContext m_context;
	std::mutex m_mutex;
	TriggerInterval m_interval;
	long m_triggerDistanceNanoSecs;

	int m_imageNrGPAI[eLastAnalogIn + 1];
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesGPAI1;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesGPAI2;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesGPAI3;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesGPAI4;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesGPAI5;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesGPAI6;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesGPAI7;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesGPAI8;
	int m_imageNrLP;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesLP;
	int m_imageNrRobSpeed;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesRobSpeed;
	int m_imageNrENC1;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesENC1;
	int m_imageNrENC2;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesENC2;

	int m_imageNrOverSmp1;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesOverSmp1;
	int m_imageNrOverSmp2;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesOverSmp2;
	int m_imageNrOverSmp3;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesOverSmp3;
	int m_imageNrOverSmp4;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesOverSmp4;
	int m_imageNrOverSmp5;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesOverSmp5;
	int m_imageNrOverSmp6;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesOverSmp6;
	int m_imageNrOverSmp7;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesOverSmp7;
	int m_imageNrOverSmp8;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesOverSmp8;

	int m_imageNrScannerXPos;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesScannerXPos;
	int m_imageNrScannerYPos;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesScannerYPos;
	int m_imageNrFiberSwitchPos;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesFiberSwitchPos;
	int m_imageNrZCPositionDigV1;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesZCPositionDigV1;
	int m_imageNrScannerWeldingFinished;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesScannerWeldingFinished;
    int m_imageNrContourPreparedFinished;
    TSmartArrayPtr<int>::ShArrayPtr* m_pValuesContourPreparedFinished;

	int m_imageNrLWM40_1_Plasma;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesLWM40_1_Plasma;
	int m_imageNrLWM40_1_Temperature;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesLWM40_1_Temperature;
	int m_imageNrLWM40_1_BackReflection;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesLWM40_1_BackReflection;
	int m_imageNrLWM40_1_AnalogInput;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesLWM40_1_AnalogInput;

	int m_oLineLaser1Intensity;
	bool m_oLineLaser1OnOff;
	bool m_oLineLaser1Enable;
	bool m_oLineLaser1_OutputViaCamera;
	bool m_oLineLaser2Enable;
	bool m_oLineLaser2_OutputViaCamera;
	int m_oLineLaser2Intensity;
	bool m_oLineLaser2OnOff;
	bool m_oFieldLight1Enable;
	bool m_oFieldLight1_OutputViaCamera;
	int m_oFieldLight1Intensity;
	bool m_oFieldLight1OnOff;

	int m_oLineLas1IntensToWin;
	bool m_oLineLas1OnOffToWin;
	int m_oLineLas2IntensToWin;
	bool m_oLineLas2OnOffToWin;
	int m_oFieldL1IntensToWin;
	bool m_oFieldL1OnOffToWin;

	bool m_oGenPurposeAnaInEnable[eLastAnalogIn + 1];
	bool m_oGenPurposeAnaOutEnable[eLastAnalogOut + 1];

	int m_oRequiredPositionX;
	int m_oRequiredPositionY;
	int m_oRequiredPositionZ;

	SerialToTracker* m_pSerialToTracker;
	DataToLaserControl* m_pDataToLaserControl;
	int m_oFocalLength;
	TrackerFrequencyStep m_oTrackerFrequencyStep;
	unsigned short m_oTrackerFrequencyCont;
	unsigned short m_oTrackerFrequencyBoth;
	bool m_oScannerOK;
	bool m_oScannerLimits;
	bool m_oTrackerDriverOnOff;
	bool m_oTrackerExpertMode;
	bool m_oLCStartSignal;

	bool m_oScanWidthOutOfGapWidth;
	int m_oScanWidthFixed;
	bool m_oScanWidthFixedWasSet;
	int m_oScanWidthControlled;
	int m_oGapWidthToScanWidthOffset;
	int m_oGapWidthToScanWidthGradient;
	unsigned short m_oScanWidthUMSent;
	int m_oScanWidthVoltSent;
	bool m_oScanWidthLimitedUM;
	bool m_oScanWidthLimitedVolt;

	bool m_oScanPosOutOfGapPos;
	int m_oScanPosFixed;
	bool m_oScanPosFixedWasSet;
	int m_oScanPosControlled;
	int m_oMaxScanPosUM;
	double m_oMaxScanPosMM;
	int m_oMinScanPosUM;
	double m_oMinScanPosMM;
	short m_oScanPosUMSent;
	int m_oScanPosVoltSent;
	bool m_oScanPosLimitedUM;
	bool m_oScanPosLimitedVolt;
	int m_oTrackerMaxAmplitude;

	static const int MAX_TRACKER_CALIB_DATA = 17;
	struct TrackerCalibData m_oTrackerCalibArray[MAX_TRACKER_CALIB_DATA];

    Scanlab* m_pScanlab;

#ifndef LED_DRIVER_DISABLE_COMPLETE_CODE
	LEDcomEthernet1T m_oLEDcomEthernet1;
	LEDdriverHW_PP420T m_oLEDdriverHW_PP420;
	LEDdriverHW_PP520T m_oLEDdriverHW_PP520;
	LEDdriverHW_PP820T m_oLEDdriverHW_PP820;
	LEDdriverT m_oLEDdriver;
	LEDdriverParameterT m_oLEDdriverParameter;
#endif
	LEDI_ParametersT m_oLEDI_Parameters[ANZ_LEDI_PARAMETERS];
    LEDControllerType m_oLEDControllerType;
	bool m_oLED_IlluminationEnable;
	int m_oLED_MaxCurrent_mA_Chan1;
	int m_oLED_MaxCurrent_mA_Chan2;
	int m_oLED_MaxCurrent_mA_Chan3;
	int m_oLED_MaxCurrent_mA_Chan4;
	int m_oLED_MaxCurrent_mA_Chan5;
	int m_oLED_MaxCurrent_mA_Chan6;
	int m_oLED_MaxCurrent_mA_Chan7;
	int m_oLED_MaxCurrent_mA_Chan8;
	int m_oLEDchannelMaxCurrent[ANZ_LEDI_PARAMETERS];
	bool m_oLED_ParameterChanged;
	bool m_oLED_DriverIsOperational;
	bool m_oScanTrackerEnable;
	bool m_oLaserControlEnable;
	bool m_oZCollimatorEnable;
	ZCollimatorType m_oZCollimatorType;
	bool m_oLaserPowerInputEnable;
	bool m_oEncoderInput1Enable;
	bool m_oEncoderInput2Enable;
	bool m_oRobotTrackSpeedEnable;

    bool m_liquidLensControllerEnable{false};
    LEDcomEthernet1T m_lensComEthernet;
    LensDriverTRCL180 m_lensDriverTRCL180;
    LEDdriverT m_lensDriver;
    bool m_lensDriverIsOperational{true};
    double m_liquidLensPosition{0.0};

	bool m_oAxisXEnable;
	bool m_oAxisYEnable;
	bool m_oAxisZEnable;

	bool m_oFastAnalogSignal1Enable;
	bool m_oFastAnalogSignal2Enable;
	bool m_oFastAnalogSignal3Enable;
	bool m_oFastAnalogSignal4Enable;
	bool m_oFastAnalogSignal5Enable;
	bool m_oFastAnalogSignal6Enable;
	bool m_oFastAnalogSignal7Enable;
	bool m_oFastAnalogSignal8Enable;

	bool m_oLWM40_No1_Enable;

    int m_oLWM40_1_AmpPlasma;
    int m_oLWM40_1_AmpTemperature;
    int m_oLWM40_1_AmpBackReflection;
    int m_oLWM40_1_AmpAnalogInput;

    bool m_oScanlabScannerEnable;
    interface::ScannerModel m_scannerModel;
    interface::ScannerGeneralMode m_scannerGeneralMode;

    bool m_oIsSOUVIS6000_Application;

	bool m_oCycleIsOn;
	bool m_oSeamIsOn;
    std::atomic<bool> m_liveModeIsOnAtBurst{false};

   	std::map<Sensor, bool>	m_oSensorIdsEnabled;

   	short m_bGlasNotPresentCounter;
   	short m_bGlasDirtyCounter;
   	short m_bTempGlasFailCounter;
   	short m_bTempHeadFailCounter;

    bool m_oPrintDebug1;

    bool m_oDebugInfoDebugFile1;
    FILE *m_pDebugFile1;
    int m_oArrayDebugFile1[10][1200];

    bool m_oDebugInfoDebugFile2;
    int m_oWriteIndexDebugFile2;
    FILE *m_pDebugFile2;
    int m_oArrayDebugFile2[LENGTH_OF_DEBUGFILE2];
};

} // namespace ethercat
} // namespace precitec

#endif /* WELDINGHEADCONTROL_H_ */

/*
 * SAX_VIConfigParser.cpp
 *
 *  Created on: 16.04.2010
 *      Author: f.agrawal
 */

#include <cstdio>
#include <module/moduleLogger.h>
#include "viWeldHead/SAX_VIConfigParser.h"

using namespace precitec;

SAX_VIConfigParser::SAX_VIConfigParser() :
			_pLocator(0) {

	m_parseXAxis = false;
	m_parseYAxis = false;
	m_parseZAxis = false;

	ClearTempValues();

	m_oProductCode_x = 0;
	m_oProductCode_y = 0;
	m_oProductCode_z = 0;

	m_oInstance_x = 0;
	m_oInstance_y = 0;
	m_oInstance_z = 0;

	m_oVendorID_x = 0;
	m_oVendorID_y = 0;
	m_oVendorID_z = 0;

	m_oHomeable_x = false;
	m_oHomeable_y = false;
	m_oHomeable_z = false;

	m_oHomingDirPosX = false;
	m_oHomingDirPosY = false;
	m_oHomingDirPosZ = false;

	m_oMountingRightTopX = false;
	m_oMountingRightTopY = false;
	m_oMountingRightTopZ = false;

	//init length 100 000

	m_oSoftLimitsAxisXActive = false;
	m_oSoftLimitsAxisYActive = false;
	m_oSoftLimitsAxisZActive = false;

	m_oAxisLengthX = 50000;
	m_oAxisLengthY = 50000;
	m_oAxisLengthZ = 50000;

	m_oLowerSoftEndAxisX = -1;
	m_oLowerSoftEndAxisY = -1;
	m_oLowerSoftEndAxisZ = -1;

	m_oUpperSoftEndAxisX = -1;
	m_oUpperSoftEndAxisY = -1;
	m_oUpperSoftEndAxisZ = -1;

	//Linienlaser
	m_parseSetLineLaser1Intensity = false;
	m_parseSetLineLaser2Intensity = false;
	m_parseSetFieldLight1Intensity = false;
	// Laser-Leistungs-Ueberwachung
	m_parseLaserPowerSignal = false;

	// Oversampling inputs
	m_parseOversamplingIn1 = false;
	m_parseOversamplingIn2 = false;
	m_parseOversamplingIn3 = false;
	m_parseOversamplingIn4 = false;
	m_parseOversamplingIn5 = false;
	m_parseOversamplingIn6 = false;
	m_parseOversamplingIn7 = false;
	m_parseOversamplingIn8 = false;

	// LWM40 inputs
	m_parseLWM40_1_Plasma = false;
	m_parseLWM40_1_Temperature = false;
	m_parseLWM40_1_BackReflection = false;
	m_parseLWM40_1_AnalogInput = false;

	// Encoder Inputs
	m_parseEncoderInput1 = false;
	m_parseEncoderInput2 = false;
	m_parseRobotTrackSpeed = false;

	// ScanTracker Interface
	m_oParseScanTracker = false;
	m_oParseSetSerialComm = false;
	m_oParseSetScanWidth = false;
	m_oParseSetScanPos = false;
	m_oParseSetEnableDriver = false;
	m_oParseGetScannerOK = false;
	m_oParseGetScannerLimits = false;

	m_oFocalLength = 250;
	m_oTrackerMaxAmplitude = 5000;
	m_oSerialCommType = 1;
	m_oSerialCommPort = 1;

	// Interface fuer motorisierter Z-Kollimator
	m_oParseMotZColl = false;
	m_oParseZCRefTravel = false;
	m_oParseZCAutomatic = false;
	m_oParseZCError = false;
	m_oParseZCPosReached = false;
	m_oParseZCAnalogIn = false;

	// Interface fuer LaserControl
	m_oParseLaserControl = false;
	m_oParseLCStartSignal = false;
	m_oParseLCErrorSignal = false;
	m_oParseLCReadySignal = false;
	m_oParseLCLimitWarning = false;
	m_oParseLCPowerOffset = false;

	m_oParseGenPurposeAnaIn1 = false;
	m_oParseGenPurposeAnaIn2 = false;
	m_oParseGenPurposeAnaIn3 = false;
	m_oParseGenPurposeAnaIn4 = false;
	m_oParseGenPurposeAnaIn5 = false;
	m_oParseGenPurposeAnaIn6 = false;
	m_oParseGenPurposeAnaIn7 = false;
	m_oParseGenPurposeAnaIn8 = false;
	m_oParseGenPurposeAnaOut1 = false;
	m_oParseGenPurposeAnaOut2 = false;
	m_oParseGenPurposeAnaOut3 = false;
	m_oParseGenPurposeAnaOut4 = false;
	m_oParseGenPurposeAnaOut5 = false;
	m_oParseGenPurposeAnaOut6 = false;
	m_oParseGenPurposeAnaOut7 = false;
	m_oParseGenPurposeAnaOut8 = false;

	m_oParseGetLEDTemperatureHigh = false;
}

SAX_VIConfigParser::~SAX_VIConfigParser() {

}

void SAX_VIConfigParser::ClearTempValues() {

	m_parseHomeable = false;
	m_oParseHomingDirPos = false;
	m_oParseMountingRightTop = false;

	m_oTempParseString.clear();

	m_tempProductCode = 0;
	m_tempVendorID = 0;
	m_tempSlaveType = 0;
	m_tempInstance = 0;
	m_tempStartBit = 0;
	m_tempTriggerLevel = 0;
	m_tempLength = 0;

	m_getSlaveType = false;
	m_getTriggerLevel = false;
	m_getProductCode = false;
	m_getVendorID = false;
	m_getInstance = false;
	m_getStartBit = false;
	m_getLength = false;
}

void SAX_VIConfigParser::insertInCommandList(const CommandType command, const std::string& p_rCommandName)
{
    if ((m_tempSlaveType == 0) ||
        (m_tempProductCode == 0) ||
        (m_tempVendorID == 0) ||
        (m_tempInstance == 0))
    {
        wmLogTr(eError, "QnxMsg.VI.VIConfErr1", "Invalid signal data of <%s> in VI_Config.xml, please correct !\n", p_rCommandName.c_str());
        wmLogTr(eError, "QnxMsg.VI.VIConfErr2", "Signal <%s> is not activated!\n", p_rCommandName.c_str());
        ClearTempValues();
        return;
    }

	SLAVE_PROXY_INFORMATION info;
	info.nSlaveType = m_tempSlaveType;
	info.nProductCode = m_tempProductCode;
	info.nVendorID = m_tempVendorID;
	info.nInstance = m_tempInstance;
	info.nStartBit = m_tempStartBit;
	info.nTriggerLevel = m_tempTriggerLevel;
	info.nLength = 1;

	COMMAND_INFORMATION commandInfo;
	commandInfo.proxyInfo = info;
	commandInfo.commandType = command;

	m_inCommandsList.push_back(commandInfo);
	ClearTempValues();
}

// ContentHandler
void SAX_VIConfigParser::setDocumentLocator(const Locator* loc) {
	_pLocator = loc;
}

void SAX_VIConfigParser::startDocument() {
}

void SAX_VIConfigParser::endDocument() {

	//check softEnds
	if(m_oSoftLimitsAxisXActive){
		if(m_oLowerSoftEndAxisX > m_oUpperSoftEndAxisX )
		{
			std::swap(m_oLowerSoftEndAxisX, m_oUpperSoftEndAxisX);
		}
	}
	if(m_oSoftLimitsAxisYActive){
		if(m_oLowerSoftEndAxisY > m_oUpperSoftEndAxisY )
		{
			std::swap(m_oLowerSoftEndAxisY, m_oUpperSoftEndAxisY);
		}
	}
	if(m_oSoftLimitsAxisZActive){
		if(m_oLowerSoftEndAxisZ > m_oUpperSoftEndAxisZ )
		{
			std::swap(m_oLowerSoftEndAxisZ, m_oUpperSoftEndAxisZ);
		}
	}

// nur aktivieren, wenn Achse X vollstaendig integriert ist
#if 0
    if ((m_oProductCode_x == 0) ||
        (m_oVendorID_x == 0) ||
        (m_oInstance_x == 0))
    {
        wmLogTr(eError, "QnxMsg.VI.VIConfErr1", "Invalid signal data of <%s> in VI_Config.xml, please correct !\n", "Axis_X");
        wmLogTr(eError, "QnxMsg.VI.VIConfErr2", "Signal <%s> is not activated!\n", "Axis_X");
        m_oProductCode_x = 0;
        m_oVendorID_x = 0;
        m_oInstance_x = 0;
    }
#endif

	wmLog(eDebug, "----------------AXIS X--------------------\n");
	wmLog(eDebug, "ProductCode          : 0x%x\n", m_oProductCode_x);
	wmLog(eDebug, "VendorID             : 0x%x\n", m_oVendorID_x);
	wmLog(eDebug, "Instance             : 0x%x\n", m_oInstance_x);
	wmLog(eDebug, "Homeable             : %d\n", m_oHomeable_x);
	wmLog(eDebug, "HomingDirPositive    : %d\n", m_oHomingDirPosX);
	wmLog(eDebug, "MountingRightTop     : %d\n", m_oMountingRightTopX);
	wmLog(eDebug, "Axis Length          : %d\n", m_oAxisLengthX);
	wmLog(eDebug, "Axis SoftLimitsActive: %d\n", m_oSoftLimitsAxisXActive);
	wmLog(eDebug, "Axis lowerSoftEnd    : %d\n", m_oLowerSoftEndAxisX);
	wmLog(eDebug, "Axis upperSoftEnd    : %d\n", m_oUpperSoftEndAxisX);

    if ((m_oProductCode_y == 0) ||
        (m_oVendorID_y == 0) ||
        (m_oInstance_y == 0))
    {
        wmLogTr(eError, "QnxMsg.VI.VIConfErr1", "Invalid signal data of <%s> in VI_Config.xml, please correct !\n", "Axis_Y");
        wmLogTr(eError, "QnxMsg.VI.VIConfErr2", "Signal <%s> is not activated!\n", "Axis_Y");
        m_oProductCode_y = 0;
        m_oVendorID_y = 0;
        m_oInstance_y = 0;
    }

	wmLog(eDebug, "----------------AXIS Y--------------------\n");
	wmLog(eDebug, "ProductCode          : 0x%x\n", m_oProductCode_y);
	wmLog(eDebug, "VendorID             : 0x%x\n", m_oVendorID_y);
	wmLog(eDebug, "Instance             : 0x%x\n", m_oInstance_y);
	wmLog(eDebug, "Homeable             : %d\n", m_oHomeable_y);
	wmLog(eDebug, "HomingDirPositive    : %d\n", m_oHomingDirPosY);
	wmLog(eDebug, "MountingRightTop     : %d\n", m_oMountingRightTopY);
	wmLog(eDebug, "Axis Length          : %d\n", m_oAxisLengthY);
	wmLog(eDebug, "Axis SoftLimitsActive: %d\n", m_oSoftLimitsAxisYActive);
	wmLog(eDebug, "Axis lowerSoftEnd    : %d\n", m_oLowerSoftEndAxisY);
	wmLog(eDebug, "Axis upperSoftEnd    : %d\n", m_oUpperSoftEndAxisY);

// nur aktivieren, wenn Achse Z vollstaendig integriert ist
#if 0
    if ((m_oProductCode_z == 0) ||
        (m_oVendorID_z == 0) ||
        (m_oInstance_z == 0))
    {
        wmLogTr(eError, "QnxMsg.VI.VIConfErr1", "Invalid signal data of <%s> in VI_Config.xml, please correct !\n", "Axis_Z");
        wmLogTr(eError, "QnxMsg.VI.VIConfErr2", "Signal <%s> is not activated!\n", "Axis_Z");
        m_oProductCode_z = 0;
        m_oVendorID_z = 0;
        m_oInstance_z = 0;
    }
#endif

	wmLog(eDebug, "----------------AXIS Z--------------------\n");
	wmLog(eDebug, "ProductCode          : 0x%x\n", m_oProductCode_z);
	wmLog(eDebug, "VendorID             : 0x%x\n", m_oVendorID_z);
	wmLog(eDebug, "Instance             : 0x%x\n", m_oInstance_z);
	wmLog(eDebug, "Homeable             : %d\n", m_oHomeable_z);
	wmLog(eDebug, "HomingDirPositive    : %d\n", m_oHomingDirPosZ);
	wmLog(eDebug, "MountingRightTop     : %d\n", m_oMountingRightTopZ);
	wmLog(eDebug, "Axis Length          : %d\n", m_oAxisLengthZ);
	wmLog(eDebug, "Axis SoftLimitsActive: %d\n", m_oSoftLimitsAxisZActive);
	wmLog(eDebug, "Axis lowerSoftEnd    : %d\n", m_oLowerSoftEndAxisZ);
	wmLog(eDebug, "Axis upperSoftEnd    : %d\n", m_oUpperSoftEndAxisZ);
}

void SAX_VIConfigParser::startElement(const XMLString& uri,
		const XMLString& localName, const XMLString& qname,
		const Attributes& attributes) {

	if (strcmp("Axis_X", localName.c_str()) == 0) {
		m_parseXAxis = true;

		for (int i = 0; i < attributes.getLength(); ++i)
		{
			if ((strcmp("AxisLength", attributes.getLocalName(i).c_str()) == 0)){
				m_oAxisLengthX = Tools::StringToInt(attributes.getValue(i));
			}
			if ((strcmp("SoftLimitsActive", attributes.getLocalName(i).c_str()) == 0)
					&& (strcmp("1", attributes.getValue(i).c_str()) == 0)) {
				m_oSoftLimitsAxisXActive= true;
			}
			if ((strcmp("LowerSoftEnd", attributes.getLocalName(i).c_str()) == 0)){
				m_oLowerSoftEndAxisX = Tools::StringToInt(attributes.getValue(i));
			}
			if ((strcmp("UpperSoftEnd", attributes.getLocalName(i).c_str()) == 0)){
				m_oUpperSoftEndAxisX = Tools::StringToInt(attributes.getValue(i));
			}
		}
	}
	if (strcmp("Axis_Y", localName.c_str()) == 0) {
		m_parseYAxis = true;

		for (int i = 0; i < attributes.getLength(); ++i)
		{
			if ((strcmp("AxisLength", attributes.getLocalName(i).c_str()) == 0)){
				m_oAxisLengthY = Tools::StringToInt(attributes.getValue(i));
			}
			if ((strcmp("SoftLimitsActive", attributes.getLocalName(i).c_str()) == 0)
					&& (strcmp("1", attributes.getValue(i).c_str()) == 0)) {
				m_oSoftLimitsAxisYActive = true;
			}
			if ((strcmp("LowerSoftEnd", attributes.getLocalName(i).c_str()) == 0)){
				m_oLowerSoftEndAxisY = Tools::StringToInt(attributes.getValue(i));
			}
			if ((strcmp("UpperSoftEnd", attributes.getLocalName(i).c_str()) == 0)){
				m_oUpperSoftEndAxisY = Tools::StringToInt(attributes.getValue(i));
			}
		}
	}
	if (strcmp("Axis_Z", localName.c_str()) == 0) {
		m_parseZAxis = true;

		for (int i = 0; i < attributes.getLength(); ++i)
		{
			if ((strcmp("AxisLength", attributes.getLocalName(i).c_str()) == 0)){
				m_oAxisLengthZ = Tools::StringToInt(attributes.getValue(i));
			}
			if ((strcmp("SoftLimitsActive", attributes.getLocalName(i).c_str()) == 0)
					&& (strcmp("1", attributes.getValue(i).c_str()) == 0)) {
				m_oSoftLimitsAxisZActive = true;
			}
			if ((strcmp("LowerSoftEnd", attributes.getLocalName(i).c_str()) == 0)){
				m_oLowerSoftEndAxisZ = Tools::StringToInt(attributes.getValue(i));
			}
			if ((strcmp("UpperSoftEnd", attributes.getLocalName(i).c_str()) == 0)){
				m_oUpperSoftEndAxisZ = Tools::StringToInt(attributes.getValue(i));
			}
		}
	}
	// ScanTracker Interface
	if (strcmp("ScanTracker", localName.c_str()) == 0)
	{
		for (int i = 0; i < attributes.getLength(); ++i)
		{
			if ((strcmp("FocalLength", attributes.getLocalName(i).c_str()) == 0)){
				m_oFocalLength = Tools::StringToInt(attributes.getValue(i));
			}
			if ((strcmp("MaxAmplitude", attributes.getLocalName(i).c_str()) == 0)){
				m_oTrackerMaxAmplitude = Tools::StringToInt(attributes.getValue(i));
			}
		}
		m_oParseScanTracker = true;
	}
	// SetSerialComm
	if (strcmp("SetSerialComm", localName.c_str()) == 0)
	{
		for (int i = 0; i < attributes.getLength(); ++i)
		{
			if ((strcmp("Type", attributes.getLocalName(i).c_str()) == 0))
			{
				m_oSerialCommType = Tools::StringToInt(attributes.getValue(i));
				if (m_oSerialCommType < 1) m_oSerialCommType = 1; // COM Interfaces onBoard
				if (m_oSerialCommType > 2) m_oSerialCommType = 2; // RS422 Board Delock89318
			}
			if ((strcmp("Port", attributes.getLocalName(i).c_str()) == 0))
			{
				m_oSerialCommPort = Tools::StringToInt(attributes.getValue(i));
				if (m_oSerialCommPort < 1) m_oSerialCommPort = 1; // Port 1 (COM1, RS422-1)
				if (m_oSerialCommPort > 2) m_oSerialCommPort = 2; // Port 2 (COM2, RS422-2)
			}
		}
		m_oParseSetSerialComm = true;
	}
	// SetScanWidth
	if (strcmp("SetScanWidth", localName.c_str()) == 0)
	{
		m_oParseSetScanWidth = true;
	}
	// SetScanPos
	if (strcmp("SetScanPos", localName.c_str()) == 0)
	{
		m_oParseSetScanPos = true;
	}
	// SetEnableDriver
	if (strcmp("SetEnableDriver", localName.c_str()) == 0)
	{
		m_oParseSetEnableDriver = true;
	}
	// GetScannerOK
	if (strcmp("GetScannerOK", localName.c_str()) == 0)
	{
		m_oParseGetScannerOK = true;
	}
	// GetScannerLimits
	if (strcmp("GetScannerLimits", localName.c_str()) == 0)
	{
		m_oParseGetScannerLimits = true;
	}
	// Interface fuer motorisierter Z-Kollimator
	if (strcmp("MotZCollimator", localName.c_str()) == 0)
	{
		m_oParseMotZColl = true;
	}
	// Ref_Travel
	if (strcmp("Ref_Travel", localName.c_str()) == 0)
	{
		m_oParseZCRefTravel = true;
	}
	// Automatic
	if (strcmp("Automatic", localName.c_str()) == 0)
	{
		m_oParseZCAutomatic = true;
	}
	// Error
	if (strcmp("Error", localName.c_str()) == 0)
	{
		m_oParseZCError = true;
	}
	// Pos_Reached
	if (strcmp("Pos_Reached", localName.c_str()) == 0)
	{
		m_oParseZCPosReached = true;
	}
	// Analog_In
	if (strcmp("Analog_In", localName.c_str()) == 0)
	{
		m_oParseZCAnalogIn = true;
	}
	// Interface fuer LaserControl
	if (strcmp("LaserControl", localName.c_str()) == 0)
	{
		m_oParseLaserControl = true;
	}
	// LCStartSignal
	if (strcmp("LCStartSignal", localName.c_str()) == 0)
	{
		m_oParseLCStartSignal = true;
	}
	// LCErrorSignal
	if (strcmp("LCErrorSignal", localName.c_str()) == 0)
	{
		m_oParseLCErrorSignal = true;
	}
	// LCReadySignal
	if (strcmp("LCReadySignal", localName.c_str()) == 0)
	{
		m_oParseLCReadySignal = true;
	}
	// LCLimitWarning
	if (strcmp("LCLimitWarning", localName.c_str()) == 0)
	{
		m_oParseLCLimitWarning = true;
	}
	// LCPowerOffset
	if (strcmp("LCPowerOffset", localName.c_str()) == 0)
	{
		m_oParseLCPowerOffset = true;
	}
	// GenPurposeAnaIn1
	if (strcmp("GeneralPurpose_AnalogIn1", localName.c_str()) == 0)
	{
		m_oParseGenPurposeAnaIn1 = true;
	}
	// GenPurposeAnaIn2
	if (strcmp("GeneralPurpose_AnalogIn2", localName.c_str()) == 0)
	{
		m_oParseGenPurposeAnaIn2 = true;
	}
	// GenPurposeAnaIn3
	if (strcmp("GeneralPurpose_AnalogIn3", localName.c_str()) == 0)
	{
		m_oParseGenPurposeAnaIn3 = true;
	}
	// GenPurposeAnaIn4
	if (strcmp("GeneralPurpose_AnalogIn4", localName.c_str()) == 0)
	{
		m_oParseGenPurposeAnaIn4 = true;
	}
	// GenPurposeAnaIn5
	if (strcmp("GeneralPurpose_AnalogIn5", localName.c_str()) == 0)
	{
		m_oParseGenPurposeAnaIn5 = true;
	}
	// GenPurposeAnaIn6
	if (strcmp("GeneralPurpose_AnalogIn6", localName.c_str()) == 0)
	{
		m_oParseGenPurposeAnaIn6 = true;
	}
	// GenPurposeAnaIn7
	if (strcmp("GeneralPurpose_AnalogIn7", localName.c_str()) == 0)
	{
		m_oParseGenPurposeAnaIn7 = true;
	}
	// GenPurposeAnaIn8
	if (strcmp("GeneralPurpose_AnalogIn8", localName.c_str()) == 0)
	{
		m_oParseGenPurposeAnaIn8 = true;
	}
	// GenPurposeAnaOut1
	if (strcmp("GeneralPurpose_AnalogOut1", localName.c_str()) == 0)
	{
		m_oParseGenPurposeAnaOut1 = true;
	}
	// GenPurposeAnaOut2
	if (strcmp("GeneralPurpose_AnalogOut2", localName.c_str()) == 0)
	{
		m_oParseGenPurposeAnaOut2 = true;
	}
	// GenPurposeAnaOut3
	if (strcmp("GeneralPurpose_AnalogOut3", localName.c_str()) == 0)
	{
		m_oParseGenPurposeAnaOut3 = true;
	}
	// GenPurposeAnaOut4
	if (strcmp("GeneralPurpose_AnalogOut4", localName.c_str()) == 0)
	{
		m_oParseGenPurposeAnaOut4 = true;
	}
	// GenPurposeAnaOut5
	if (strcmp("GeneralPurpose_AnalogOut5", localName.c_str()) == 0)
	{
		m_oParseGenPurposeAnaOut5 = true;
	}
	// GenPurposeAnaOut6
	if (strcmp("GeneralPurpose_AnalogOut6", localName.c_str()) == 0)
	{
		m_oParseGenPurposeAnaOut6 = true;
	}
	// GenPurposeAnaOut7
	if (strcmp("GeneralPurpose_AnalogOut7", localName.c_str()) == 0)
	{
		m_oParseGenPurposeAnaOut7 = true;
	}
	// GenPurposeAnaOut8
	if (strcmp("GeneralPurpose_AnalogOut8", localName.c_str()) == 0)
	{
		m_oParseGenPurposeAnaOut8 = true;
	}
	// GetLEDTemperatureHigh
	if (strcmp("LED_Controller_Temperature_Too_High", localName.c_str()) == 0)
	{
		m_oParseGetLEDTemperatureHigh = true;
	}
	// SetLineLaser1Intensity
	if (strcmp("SetLineLaser1Intensity", localName.c_str()) == 0) {
		m_parseSetLineLaser1Intensity = true;
	}
	// SetFieldLight1Intensity
	if (strcmp("SetFieldLight1Intensity", localName.c_str()) == 0) {
		m_parseSetFieldLight1Intensity = true;
	}
	// SetLineLaser2Intensity
	if (strcmp("SetLineLaser2Intensity", localName.c_str()) == 0) {
		m_parseSetLineLaser2Intensity = true;
	}
	// LaserPowerSignal
	if (strcmp("LaserPowerSignal", localName.c_str()) == 0) {
		m_parseLaserPowerSignal = true;
	}
	// OversamplingSignal1
	if (strcmp("OversamplingSignal1", localName.c_str()) == 0) {
		m_parseOversamplingIn1 = true;
	}
	// OversamplingSignal2
	if (strcmp("OversamplingSignal2", localName.c_str()) == 0) {
		m_parseOversamplingIn2 = true;
	}
	// OversamplingSignal3
	if (strcmp("OversamplingSignal3", localName.c_str()) == 0) {
		m_parseOversamplingIn3 = true;
	}
	// OversamplingSignal4
	if (strcmp("OversamplingSignal4", localName.c_str()) == 0) {
		m_parseOversamplingIn4 = true;
	}
	// OversamplingSignal5
	if (strcmp("OversamplingSignal5", localName.c_str()) == 0) {
		m_parseOversamplingIn5 = true;
	}
	// OversamplingSignal6
	if (strcmp("OversamplingSignal6", localName.c_str()) == 0) {
		m_parseOversamplingIn6 = true;
	}
	// OversamplingSignal7
	if (strcmp("OversamplingSignal7", localName.c_str()) == 0) {
		m_parseOversamplingIn7 = true;
	}
	// OversamplingSignal8
	if (strcmp("OversamplingSignal8", localName.c_str()) == 0) {
		m_parseOversamplingIn8 = true;
	}
	// LWM40_1_Plasma
	if (strcmp("LWM40_1_Plasma", localName.c_str()) == 0) {
		m_parseLWM40_1_Plasma = true;
	}
	// LWM40_1_Temperature
	if (strcmp("LWM40_1_Temperature", localName.c_str()) == 0) {
		m_parseLWM40_1_Temperature = true;
	}
	// LWM40_1_BackReflection
	if (strcmp("LWM40_1_BackReflection", localName.c_str()) == 0) {
		m_parseLWM40_1_BackReflection = true;
	}
	// LWM40_1_AnalogInput
	if (strcmp("LWM40_1_AnalogInput", localName.c_str()) == 0) {
		m_parseLWM40_1_AnalogInput = true;
	}
	// Encoder Inputs
	if (strcmp("EncoderInput1", localName.c_str()) == 0) {
		m_parseEncoderInput1 = true;
	}
	if (strcmp("EncoderInput2", localName.c_str()) == 0) {
		m_parseEncoderInput2 = true;
	}
	if (strcmp("RobotTrackSpeed", localName.c_str()) == 0) {
		m_parseRobotTrackSpeed = true;
	}

	if ((strcmp("ProductCode", localName.c_str()) == 0)) {
		m_getProductCode = true;
	}
	if ((strcmp("VendorID", localName.c_str()) == 0)) {
		m_getVendorID = true;
	}
	if ((strcmp("SlaveType", localName.c_str()) == 0)) {
		m_getSlaveType = true;
	}
	if ((strcmp("Instance", localName.c_str()) == 0)) {
		m_getInstance = true;
	}
	if ((strcmp("StartBit", localName.c_str()) == 0)) {
		m_getStartBit = true;
	}
	if ((strcmp("TriggerLevel", localName.c_str()) == 0)) {
		m_getTriggerLevel = true;
	}
	if ((strcmp("Length", localName.c_str()) == 0)) {
		m_getLength = true;
	}
	if ((strcmp("HomeAble", localName.c_str()) == 0)) {
		m_parseHomeable = true;
	}
	if ((strcmp("HomingDirPositive", localName.c_str()) == 0)) {
		m_oParseHomingDirPos = true;
	}
	if ((strcmp("MountingRightTop", localName.c_str()) == 0)) {
		m_oParseMountingRightTop = true;
	}
}

void SAX_VIConfigParser::endElement(const XMLString& uri,
		const XMLString& localName, const XMLString& qname) {

	if (m_parseXAxis && (strcmp("Axis_X", localName.c_str()) == 0)) {
		m_parseXAxis = false;
		ClearTempValues();
	}
	if (m_parseYAxis && (strcmp("Axis_Y", localName.c_str()) == 0)) {
		m_parseYAxis = false;
		ClearTempValues();
	}
	if (m_parseZAxis && (strcmp("Axis_Z", localName.c_str()) == 0)) {
		m_parseZAxis = false;
		ClearTempValues();
	}
	if (strcmp("ProductCode", localName.c_str()) == 0) {
		if (m_parseXAxis) {
			m_oProductCode_x = Tools::StringToInt(std::string(m_oTempParseString));
		}
		else if (m_parseYAxis) {
			m_oProductCode_y = Tools::StringToInt(std::string(m_oTempParseString));
		}
		else if (m_parseZAxis) {
			m_oProductCode_z = Tools::StringToInt(std::string(m_oTempParseString));
		}
		else
		{
			m_tempProductCode = Tools::StringToInt(m_oTempParseString);
		}
		m_oTempParseString.clear();
		m_getProductCode = false;
	}
	if (strcmp("VendorID", localName.c_str()) == 0) {
		if (m_parseXAxis) {
			m_oVendorID_x = Tools::StringToInt(m_oTempParseString);
		}
		else if (m_parseYAxis) {
			m_oVendorID_y = Tools::StringToInt(m_oTempParseString);
		}
		else if (m_parseZAxis) {
			m_oVendorID_z = Tools::StringToInt(m_oTempParseString);
		}
		else
		{
			m_tempVendorID = Tools::StringToInt(m_oTempParseString);
		}
		m_oTempParseString.clear();
		m_getVendorID = false;
	}
	if (strcmp("SlaveType", localName.c_str()) == 0) {
		m_tempSlaveType = Tools::StringToInt(m_oTempParseString);
		m_oTempParseString.clear();
		m_getSlaveType = false;
	}
	if (strcmp("Instance", localName.c_str()) == 0) {
		if (m_parseXAxis) {
			m_oInstance_x = Tools::StringToInt(m_oTempParseString);
		}
		else if (m_parseYAxis) {
			m_oInstance_y = Tools::StringToInt(m_oTempParseString);
		}
		else if (m_parseZAxis) {
			m_oInstance_z = Tools::StringToInt(m_oTempParseString);
		}
		else
		{
			m_tempInstance = Tools::StringToInt(m_oTempParseString);
		}
		m_oTempParseString.clear();
		m_getInstance = false;
	}
	if (strcmp("StartBit", localName.c_str()) == 0) {
		m_tempStartBit = Tools::StringToInt(m_oTempParseString);
		m_oTempParseString.clear();
		m_getStartBit = false;
	}
	if (strcmp("TriggerLevel", localName.c_str()) == 0) {
		m_tempTriggerLevel = Tools::StringToInt(m_oTempParseString);
		m_oTempParseString.clear();
		m_getTriggerLevel = false;
	}
	if (strcmp("Length", localName.c_str()) == 0) {
		m_tempLength = Tools::StringToInt(m_oTempParseString);
		m_oTempParseString.clear();
		m_getLength = false;
	}
	if (strcmp("HomeAble", localName.c_str()) == 0) {
		if (m_parseXAxis) {
			m_oHomeable_x = Tools::StringToInt(m_oTempParseString);
		}
		else if (m_parseYAxis) {
			m_oHomeable_y = Tools::StringToInt(m_oTempParseString);
		}
		else if (m_parseZAxis) {
			m_oHomeable_z = Tools::StringToInt(m_oTempParseString);
		}
		m_oTempParseString.clear();
		m_parseHomeable = false;
	}
	if (strcmp("HomingDirPositive", localName.c_str()) == 0) {
		if (m_parseXAxis) {
			m_oHomingDirPosX = Tools::StringToInt(m_oTempParseString);
		}
		else if (m_parseYAxis) {
			m_oHomingDirPosY = Tools::StringToInt(m_oTempParseString);
		}
		else if (m_parseZAxis) {
			m_oHomingDirPosZ = Tools::StringToInt(m_oTempParseString);
		}
		m_oTempParseString.clear();
		m_oParseHomingDirPos = false;
	}
	if (strcmp("MountingRightTop", localName.c_str()) == 0) {
		if (m_parseXAxis) {
			m_oMountingRightTopX = Tools::StringToInt(m_oTempParseString);
		}
		else if (m_parseYAxis) {
			m_oMountingRightTopY = Tools::StringToInt(m_oTempParseString);
		}
		else if (m_parseZAxis) {
			m_oMountingRightTopZ = Tools::StringToInt(m_oTempParseString);
		}
		m_oTempParseString.clear();
		m_oParseMountingRightTop = false;
	}
	if ((strcmp("SetLineLaser1Intensity", localName.c_str()) == 0)) {
		insertInCommandList(ALineLaser1Intens, "SetLineLaser1Intensity");
		m_parseSetLineLaser1Intensity = false;
	}
	if ((strcmp("SetFieldLight1Intensity", localName.c_str()) == 0)) {
		insertInCommandList(AFieldLight1Intens, "SetFieldLight1Intensity");
		m_parseSetFieldLight1Intensity = false;
	}
	if ((strcmp("SetLineLaser2Intensity", localName.c_str()) == 0)) {
		insertInCommandList(ALineLaser2Intens, "SetLineLaser2Intensity");
		m_parseSetLineLaser2Intensity = false;
	}
	if ((strcmp("LaserPowerSignal", localName.c_str()) == 0)) {
		insertInCommandList(ELaserPowerSignal, "LaserPowerSignal");
		m_parseLaserPowerSignal = false;
	}

	if ((strcmp("OversamplingSignal1", localName.c_str()) == 0)) {
		insertInCommandList(EOversamplingSignal1, "OversamplingSignal1");
		m_parseOversamplingIn1 = false;
	}
	if ((strcmp("OversamplingSignal2", localName.c_str()) == 0)) {
		insertInCommandList(EOversamplingSignal2, "OversamplingSignal2");
		m_parseOversamplingIn2 = false;
	}
	if ((strcmp("OversamplingSignal3", localName.c_str()) == 0)) {
		insertInCommandList(EOversamplingSignal3, "OversamplingSignal3");
		m_parseOversamplingIn3 = false;
	}
	if ((strcmp("OversamplingSignal4", localName.c_str()) == 0)) {
		insertInCommandList(EOversamplingSignal4, "OversamplingSignal4");
		m_parseOversamplingIn4 = false;
	}
	if ((strcmp("OversamplingSignal5", localName.c_str()) == 0)) {
		insertInCommandList(EOversamplingSignal5, "OversamplingSignal5");
		m_parseOversamplingIn5 = false;
	}
	if ((strcmp("OversamplingSignal6", localName.c_str()) == 0)) {
		insertInCommandList(EOversamplingSignal6, "OversamplingSignal6");
		m_parseOversamplingIn6 = false;
	}
	if ((strcmp("OversamplingSignal7", localName.c_str()) == 0)) {
		insertInCommandList(EOversamplingSignal7, "OversamplingSignal7");
		m_parseOversamplingIn7 = false;
	}
	if ((strcmp("OversamplingSignal8", localName.c_str()) == 0)) {
		insertInCommandList(EOversamplingSignal8, "OversamplingSignal8");
		m_parseOversamplingIn8 = false;
	}
	if ((strcmp("LWM40_1_Plasma", localName.c_str()) == 0)) {
		insertInCommandList(ELWM40_1_Plasma, "LWM40_1_Plasma");
		m_parseLWM40_1_Plasma = false;
	}
	if ((strcmp("LWM40_1_Temperature", localName.c_str()) == 0)) {
		insertInCommandList(ELWM40_1_Temperature, "LWM40_1_Temperature");
		m_parseLWM40_1_Temperature = false;
	}
	if ((strcmp("LWM40_1_BackReflection", localName.c_str()) == 0)) {
		insertInCommandList(ELWM40_1_BackReflection, "LWM40_1_BackReflection");
		m_parseLWM40_1_BackReflection = false;
	}
	if ((strcmp("LWM40_1_AnalogInput", localName.c_str()) == 0)) {
		insertInCommandList(ELWM40_1_AnalogInput, "LWM40_1_AnalogInput");
		m_parseLWM40_1_AnalogInput = false;
	}

	if ((strcmp("EncoderInput1", localName.c_str()) == 0)) {
		insertInCommandList(EEncoderInput1, "EncoderInput1");
		m_parseEncoderInput1 = false;
	}
	if ((strcmp("EncoderInput2", localName.c_str()) == 0)) {
		insertInCommandList(EEncoderInput2, "EncoderInput2");
		m_parseEncoderInput2 = false;
	}
	if ((strcmp("RobotTrackSpeed", localName.c_str()) == 0)) {
		insertInCommandList(ERobotTrackSpeed, "RobotTrackSpeed");
		m_parseRobotTrackSpeed = false;
	}
	if (m_oParseScanTracker)
	{
		if ((strcmp("SetScanWidth", localName.c_str()) == 0))
		{
			insertInCommandList(ATrackerScanWidth, "SetScanWidth");
			m_oParseSetScanWidth = false;
		}
		if ((strcmp("SetScanPos", localName.c_str()) == 0))
		{
			insertInCommandList(ATrackerScanPos, "SetScanPos");
			m_oParseSetScanPos = false;
		}
		if ((strcmp("SetEnableDriver", localName.c_str()) == 0))
		{
			insertInCommandList(ATrackerEnableDriver, "SetEnableDriver");
			m_oParseSetEnableDriver = false;
		}
		if ((strcmp("GetScannerOK", localName.c_str()) == 0))
		{
			insertInCommandList(ETrackerScannerOK, "GetScannerOK");
			m_oParseGetScannerOK = false;
		}
		if ((strcmp("GetScannerLimits", localName.c_str()) == 0))
		{
			insertInCommandList(ETrackerScannerLimits, "GetScannerLimits");
			m_oParseGetScannerLimits = false;
		}
	}
	if (m_oParseMotZColl)
	{
		if ((strcmp("Ref_Travel", localName.c_str()) == 0))
		{
			insertInCommandList(AZCRefTravel, "Ref_Travel");
			m_oParseZCRefTravel = false;
		}
		if ((strcmp("Automatic", localName.c_str()) == 0))
		{
			insertInCommandList(AZCAutomatic, "Automatic");
			m_oParseZCAutomatic = false;
		}
		if ((strcmp("Error", localName.c_str()) == 0))
		{
			insertInCommandList(EZCError, "Error");
			m_oParseZCError = false;
		}
		if ((strcmp("Pos_Reached", localName.c_str()) == 0))
		{
			insertInCommandList(EZCPosReached, "Pos_Reached");
			m_oParseZCPosReached = false;
		}
		if ((strcmp("Analog_In", localName.c_str()) == 0))
		{
			insertInCommandList(AZCAnalogIn, "Analog_In");
			m_oParseZCAnalogIn = false;
		}
	}
	// Interface fuer LaserControl
	if (m_oParseLaserControl)
	{
		if ((strcmp("LCStartSignal", localName.c_str()) == 0))
		{
			insertInCommandList(ALCStartSignal, "LCStartSignal");
			m_oParseLCStartSignal = false;
		}
		if ((strcmp("LCErrorSignal", localName.c_str()) == 0))
		{
			insertInCommandList(ELCErrorSignal, "LCErrorSignal");
			m_oParseLCErrorSignal = false;
		}
		if ((strcmp("LCReadySignal", localName.c_str()) == 0))
		{
			insertInCommandList(ELCReadySignal, "LCReadySignal");
			m_oParseLCReadySignal = false;
		}
		if ((strcmp("LCLimitWarning", localName.c_str()) == 0))
		{
			insertInCommandList(ELCLimitWarning, "LCLimitWarning");
			m_oParseLCLimitWarning = false;
		}
		if ((strcmp("LCPowerOffset", localName.c_str()) == 0))
		{
			insertInCommandList(ALCPowerOffset, "LCPowerOffset");
			m_oParseLCPowerOffset = false;
		}
	}
	if ((strcmp("GeneralPurpose_AnalogIn1", localName.c_str()) == 0))
	{
		insertInCommandList(EGenPurposeAnaIn1, "GeneralPurpose_AnalogIn1");
		m_oParseGenPurposeAnaIn1 = false;
	}
	if ((strcmp("GeneralPurpose_AnalogIn2", localName.c_str()) == 0))
	{
		insertInCommandList(EGenPurposeAnaIn2, "GeneralPurpose_AnalogIn2");
		m_oParseGenPurposeAnaIn2 = false;
	}
	if ((strcmp("GeneralPurpose_AnalogIn3", localName.c_str()) == 0))
	{
		insertInCommandList(EGenPurposeAnaIn3, "GeneralPurpose_AnalogIn3");
		m_oParseGenPurposeAnaIn3 = false;
	}
	if ((strcmp("GeneralPurpose_AnalogIn4", localName.c_str()) == 0))
	{
		insertInCommandList(EGenPurposeAnaIn4, "GeneralPurpose_AnalogIn4");
		m_oParseGenPurposeAnaIn4 = false;
	}
	if ((strcmp("GeneralPurpose_AnalogIn5", localName.c_str()) == 0))
	{
		insertInCommandList(EGenPurposeAnaIn5, "GeneralPurpose_AnalogIn5");
		m_oParseGenPurposeAnaIn5 = false;
	}
	if ((strcmp("GeneralPurpose_AnalogIn6", localName.c_str()) == 0))
	{
		insertInCommandList(EGenPurposeAnaIn6, "GeneralPurpose_AnalogIn6");
		m_oParseGenPurposeAnaIn6 = false;
	}
	if ((strcmp("GeneralPurpose_AnalogIn7", localName.c_str()) == 0))
	{
		insertInCommandList(EGenPurposeAnaIn7, "GeneralPurpose_AnalogIn7");
		m_oParseGenPurposeAnaIn7 = false;
	}
	if ((strcmp("GeneralPurpose_AnalogIn8", localName.c_str()) == 0))
	{
		insertInCommandList(EGenPurposeAnaIn8, "GeneralPurpose_AnalogIn8");
		m_oParseGenPurposeAnaIn8 = false;
	}
	if ((strcmp("GeneralPurpose_AnalogOut1", localName.c_str()) == 0))
	{
		insertInCommandList(AGenPurposeAnaOut1, "GeneralPurpose_AnalogOut1");
		m_oParseGenPurposeAnaOut1 = false;
	}
	if ((strcmp("GeneralPurpose_AnalogOut2", localName.c_str()) == 0))
	{
		insertInCommandList(AGenPurposeAnaOut2, "GeneralPurpose_AnalogOut2");
		m_oParseGenPurposeAnaOut2 = false;
	}
	if ((strcmp("GeneralPurpose_AnalogOut3", localName.c_str()) == 0))
	{
		insertInCommandList(AGenPurposeAnaOut3, "GeneralPurpose_AnalogOut3");
		m_oParseGenPurposeAnaOut3 = false;
	}
	if ((strcmp("GeneralPurpose_AnalogOut4", localName.c_str()) == 0))
	{
		insertInCommandList(AGenPurposeAnaOut4, "GeneralPurpose_AnalogOut4");
		m_oParseGenPurposeAnaOut4 = false;
	}
	if ((strcmp("GeneralPurpose_AnalogOut5", localName.c_str()) == 0))
	{
		insertInCommandList(AGenPurposeAnaOut5, "GeneralPurpose_AnalogOut5");
		m_oParseGenPurposeAnaOut5 = false;
	}
	if ((strcmp("GeneralPurpose_AnalogOut6", localName.c_str()) == 0))
	{
		insertInCommandList(AGenPurposeAnaOut6, "GeneralPurpose_AnalogOut6");
		m_oParseGenPurposeAnaOut6 = false;
	}
	if ((strcmp("GeneralPurpose_AnalogOut7", localName.c_str()) == 0))
	{
		insertInCommandList(AGenPurposeAnaOut7, "GeneralPurpose_AnalogOut7");
		m_oParseGenPurposeAnaOut7 = false;
	}
	if ((strcmp("GeneralPurpose_AnalogOut8", localName.c_str()) == 0))
	{
		insertInCommandList(AGenPurposeAnaOut8, "GeneralPurpose_AnalogOut8");
		m_oParseGenPurposeAnaOut8 = false;
	}
	if ((strcmp("LED_Controller_Temperature_Too_High", localName.c_str()) == 0))
	{
		insertInCommandList(ELEDTemperatureHigh, "LED_Controller_Temperature_Too_High");
		m_oParseGetLEDTemperatureHigh = false;
	}
	// ScanTracker Interface
	if((strcmp("ScanTracker", localName.c_str()) == 0))
	{
		m_oParseScanTracker = false;
	}
	// SetSerialComm
	if (strcmp("SetSerialComm", localName.c_str()) == 0)
	{
		m_oParseSetSerialComm = false;
	}
	// Interface fuer motorisierter Z-Kollimator
	if((strcmp("MotZCollimator", localName.c_str()) == 0))
	{
		m_oParseMotZColl = false;
	}
	// Interface fuer LaserControl
	if((strcmp("LaserControl", localName.c_str()) == 0))
	{
		m_oParseLaserControl = false;
	}
}

void SAX_VIConfigParser::characters(const XMLChar ch[], int start, int length) {
	if (m_parseHomeable) {
		m_oTempParseString += std::string(ch + start, length);
	}
	if (m_oParseHomingDirPos) {
		m_oTempParseString += std::string(ch + start, length);
	}
	if (m_oParseMountingRightTop) {
		m_oTempParseString += std::string(ch + start, length);
	}
	if (m_getProductCode) {
		m_oTempParseString += std::string(ch + start, length);
	}
	if (m_getVendorID) {
		m_oTempParseString += std::string(ch + start, length);
	}
	if (m_getSlaveType) {
		m_oTempParseString += std::string(ch + start, length);
	}
	if (m_getInstance) {
		m_oTempParseString += std::string(ch + start, length);
	}
	if (m_getStartBit) {
		m_oTempParseString += std::string(ch + start, length);
	}
	if (m_getTriggerLevel) {
		m_oTempParseString += std::string(ch + start, length);
	}
	if (m_getLength) {
		m_oTempParseString += std::string(ch + start, length);
	}
}

//NOT USED

void SAX_VIConfigParser::skippedEntity(const XMLString& name) {

}

void SAX_VIConfigParser::endPrefixMapping(const XMLString& prefix) {

}

void SAX_VIConfigParser::startPrefixMapping(const XMLString& prefix,
		const XMLString& uri) {

}

void SAX_VIConfigParser::processingInstruction(const XMLString& target,
		const XMLString& data) {

}

void SAX_VIConfigParser::ignorableWhitespace(const XMLChar ch[], int start,
		int length) {

}

void SAX_VIConfigParser::startDTD(const XMLString& name,
		const XMLString& publicId, const XMLString& systemId) {

}

void SAX_VIConfigParser::endDTD() {

}

void SAX_VIConfigParser::startEntity(const XMLString& name) {

}

void SAX_VIConfigParser::endEntity(const XMLString& name) {

}

void SAX_VIConfigParser::startCDATA() {

}

void SAX_VIConfigParser::endCDATA() {

}

void SAX_VIConfigParser::comment(const XMLChar ch[], int start, int length) {

}

//END NOT USED

int SAX_VIConfigParser::getFocalLength(void) { return m_oFocalLength; }
int SAX_VIConfigParser::getMaxAmplitude(void) { return m_oTrackerMaxAmplitude; }
int SAX_VIConfigParser::getSerialCommType(void) { return m_oSerialCommType; }
int SAX_VIConfigParser::getSerialCommPort(void) { return m_oSerialCommPort; }

int SAX_VIConfigParser::getProductCodeAxisX(void) { return m_oProductCode_x; }
int SAX_VIConfigParser::getProductCodeAxisY(void) { return m_oProductCode_y; }
int SAX_VIConfigParser::getProductCodeAxisZ(void) { return m_oProductCode_z; }

int SAX_VIConfigParser::getInstanceAxisX(void) { return m_oInstance_x; }
int SAX_VIConfigParser::getInstanceAxisY(void) { return m_oInstance_y; }
int SAX_VIConfigParser::getInstanceAxisZ(void) { return m_oInstance_z; }

int SAX_VIConfigParser::getVendorIDAxisX(void) { return m_oVendorID_x; }
int SAX_VIConfigParser::getVendorIDAxisY(void) { return m_oVendorID_y; }
int SAX_VIConfigParser::getVendorIDAxisZ(void) { return m_oVendorID_z; }

bool SAX_VIConfigParser::isAxisXHomeable(void) { return m_oHomeable_x; }
bool SAX_VIConfigParser::isAxisYHomeable(void) { return m_oHomeable_y; }
bool SAX_VIConfigParser::isAxisZHomeable(void) { return m_oHomeable_z; }

bool SAX_VIConfigParser::areSoftLimitsAxisXActive(void) { return m_oSoftLimitsAxisXActive; }
bool SAX_VIConfigParser::areSoftLimitsAxisYActive(void) { return m_oSoftLimitsAxisYActive; }
bool SAX_VIConfigParser::areSoftLimitsAxisZActive(void) { return m_oSoftLimitsAxisZActive; }

int SAX_VIConfigParser::getAxisLengthX(void) { return m_oAxisLengthX; }
int SAX_VIConfigParser::getAxisLengthY(void) { return m_oAxisLengthY; }
int SAX_VIConfigParser::getAxisLengthZ(void) { return m_oAxisLengthZ; }

int SAX_VIConfigParser::getLowerSoftEndAxisX(void) { return m_oLowerSoftEndAxisX; }
int SAX_VIConfigParser::getLowerSoftEndAxisY(void) { return m_oLowerSoftEndAxisY; }
int SAX_VIConfigParser::getLowerSoftEndAxisZ(void) { return m_oLowerSoftEndAxisZ; }

int SAX_VIConfigParser::getUpperSoftEndAxisX(void) { return m_oUpperSoftEndAxisX; }
int SAX_VIConfigParser::getUpperSoftEndAxisY(void) { return m_oUpperSoftEndAxisY; }
int SAX_VIConfigParser::getUpperSoftEndAxisZ(void) { return m_oUpperSoftEndAxisZ; }

bool SAX_VIConfigParser::getHomingDirPosAxisX(void) { return m_oHomingDirPosX; }
bool SAX_VIConfigParser::getHomingDirPosAxisY(void) { return m_oHomingDirPosY; }
bool SAX_VIConfigParser::getHomingDirPosAxisZ(void) { return m_oHomingDirPosZ; }

bool SAX_VIConfigParser::getMountingRightTopAxisX(void) { return m_oMountingRightTopX; }
bool SAX_VIConfigParser::getMountingRightTopAxisY(void) { return m_oMountingRightTopY; }
bool SAX_VIConfigParser::getMountingRightTopAxisZ(void) { return m_oMountingRightTopZ; }


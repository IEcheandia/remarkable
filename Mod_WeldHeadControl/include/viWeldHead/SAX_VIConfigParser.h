/*
 * SAX_VIConfigParser.h
 *
 *  Created on: 16.04.2010
 *      Author: f.agrawal
 */

#ifndef SAX_VICONFIGPARSER_H_
#define SAX_VICONFIGPARSER_H_

#include "Poco/SAX/SAXParser.h"
#include "Poco/SAX/ContentHandler.h"
#include "Poco/SAX/LexicalHandler.h"
#include "Poco/SAX/Attributes.h"
#include "Poco/SAX/Locator.h"
#include "Poco/Exception.h"
#include <iostream>
#include <unistd.h>
#include <list>

#include "event/ethercatOutputs.h"
#include "Tools.h"
#include "VIDefs.h"

using Poco::XML::XMLString;
using Poco::XML::XMLChar;
using Poco::XML::SAXParser;
using Poco::XML::XMLReader;
using Poco::XML::Attributes;
using Poco::XML::Locator;
using Poco::XML::ContentHandler;
using Poco::XML::LexicalHandler;

class SAX_VIConfigParser : public ContentHandler, public LexicalHandler{
public:
	SAX_VIConfigParser();
	virtual ~SAX_VIConfigParser();

private:
	virtual void characters(const XMLChar ch[], int start, int length);
	virtual void endElement(const XMLString& uri, const XMLString& localName, const XMLString& qname);
	virtual void startElement(const XMLString& uri, const XMLString& localName, const XMLString& qname,
				const Attributes& attributes);
	virtual void endDocument();
	virtual void startDocument();
	virtual void setDocumentLocator(const Locator* loc);

	//Not used
	virtual void skippedEntity(const XMLString& name);
	virtual void endPrefixMapping(const XMLString& prefix);
	virtual void startPrefixMapping(const XMLString& prefix, const XMLString& uri);
	virtual void processingInstruction(const XMLString& target, const XMLString& data);
	virtual void ignorableWhitespace(const XMLChar ch[], int start, int length);
	virtual void startDTD(const XMLString& name, const XMLString& publicId, const XMLString& systemId);
	virtual void endDTD();
	virtual void startEntity(const XMLString& name);
	virtual void endEntity(const XMLString& name);
	virtual void startCDATA();
	virtual void endCDATA();
	virtual void comment(const XMLChar ch[], int start, int length);
	//END Not used

    void ClearTempValues();
    void insertInCommandList(const CommandType command, const std::string& p_rCommandName);

	const Locator* _pLocator;

	bool m_parseXAxis;
	bool m_parseYAxis;
	bool m_parseZAxis;

	bool m_oSoftLimitsAxisXActive;
	bool m_oSoftLimitsAxisYActive;
	bool m_oSoftLimitsAxisZActive;

	int m_oProductCode_x;
	int m_oProductCode_y;
	int m_oProductCode_z;

	int m_oInstance_x;
	int m_oInstance_y;
	int m_oInstance_z;

	int m_oVendorID_x;
	int m_oVendorID_y;
	int m_oVendorID_z;

	bool m_oHomeable_x;
	bool m_oHomeable_y;
	bool m_oHomeable_z;

	int m_oAxisLengthX;
	int m_oAxisLengthY;
	int m_oAxisLengthZ;

	int m_oLowerSoftEndAxisX;
	int m_oLowerSoftEndAxisY;
	int m_oLowerSoftEndAxisZ;

	int m_oUpperSoftEndAxisX;
	int m_oUpperSoftEndAxisY;
	int m_oUpperSoftEndAxisZ;

    bool m_oHomingDirPosX;
    bool m_oHomingDirPosY;
    bool m_oHomingDirPosZ;

    bool m_oMountingRightTopX;
    bool m_oMountingRightTopY;
    bool m_oMountingRightTopZ;

	bool m_parseHomeable;
	bool m_oParseHomingDirPos;
	bool m_oParseMountingRightTop;
	bool m_parseSetLineLaser1Intensity;
	bool m_parseSetLineLaser2Intensity;
	bool m_parseSetFieldLight1Intensity;
	bool m_parseLaserPowerSignal;

	bool m_parseOversamplingIn1;
	bool m_parseOversamplingIn2;
	bool m_parseOversamplingIn3;
	bool m_parseOversamplingIn4;
	bool m_parseOversamplingIn5;
	bool m_parseOversamplingIn6;
	bool m_parseOversamplingIn7;
	bool m_parseOversamplingIn8;

	bool m_parseLWM40_1_Plasma;
	bool m_parseLWM40_1_Temperature;
	bool m_parseLWM40_1_BackReflection;
	bool m_parseLWM40_1_AnalogInput;

	bool m_parseEncoderInput1;
	bool m_parseEncoderInput2;
	bool m_parseRobotTrackSpeed;

	std::string m_oTempParseString;

	int m_tempProductCode;
	int m_tempVendorID;
	int m_tempSlaveType;
	int m_tempInstance;
	int m_tempStartBit;
	int m_tempTriggerLevel;
	int m_tempLength;

	bool m_getProductCode;
	bool m_getVendorID;
	bool m_getSlaveType;
	bool m_getInstance;
	bool m_getStartBit;
	bool m_getTriggerLevel;
	bool m_getLength;

	bool m_oParseScanTracker;
	bool m_oParseSetSerialComm;
	bool m_oParseSetScanWidth;
	bool m_oParseSetScanPos;
	bool m_oParseSetEnableDriver;
	bool m_oParseGetPosOutput;
	bool m_oParseGetOSCOutput;
	bool m_oParseGetScannerOK;
	bool m_oParseGetScannerLimits;

	int m_oFocalLength;
	int m_oTrackerMaxAmplitude;
	int m_oSerialCommType;
	int m_oSerialCommPort;

	bool m_oParseMotZColl;
	bool m_oParseZCRefTravel;
	bool m_oParseZCAutomatic;
	bool m_oParseZCError;
	bool m_oParseZCPosReached;
	bool m_oParseZCAnalogIn;

	bool m_oParseLaserControl;
	bool m_oParseLCStartSignal;
	bool m_oParseLCErrorSignal;
	bool m_oParseLCReadySignal;
	bool m_oParseLCLimitWarning;
	bool m_oParseLCPowerOffset;

	bool m_oParseGenPurposeAnaIn1;
	bool m_oParseGenPurposeAnaIn2;
	bool m_oParseGenPurposeAnaIn3;
	bool m_oParseGenPurposeAnaIn4;
	bool m_oParseGenPurposeAnaIn5;
	bool m_oParseGenPurposeAnaIn6;
	bool m_oParseGenPurposeAnaIn7;
	bool m_oParseGenPurposeAnaIn8;
	bool m_oParseGenPurposeAnaOut1;
	bool m_oParseGenPurposeAnaOut2;
	bool m_oParseGenPurposeAnaOut3;
	bool m_oParseGenPurposeAnaOut4;
	bool m_oParseGenPurposeAnaOut5;
	bool m_oParseGenPurposeAnaOut6;
	bool m_oParseGenPurposeAnaOut7;
	bool m_oParseGenPurposeAnaOut8;

	bool m_oParseGetLEDTemperatureHigh;

public:

	int getFocalLength(void);
	int getMaxAmplitude(void);
	int getSerialCommType(void);
	int getSerialCommPort(void);

	int getProductCodeAxisX(void);
	int getProductCodeAxisY(void);
	int getProductCodeAxisZ(void);

	int getInstanceAxisX(void);
	int getInstanceAxisY(void);
	int getInstanceAxisZ(void);

	int getVendorIDAxisX(void);
	int getVendorIDAxisY(void);
	int getVendorIDAxisZ(void);

	bool isAxisXHomeable(void);
	bool isAxisYHomeable(void);
	bool isAxisZHomeable(void);

	bool areSoftLimitsAxisXActive(void);
	bool areSoftLimitsAxisYActive(void);
	bool areSoftLimitsAxisZActive(void);

	int getAxisLengthX(void);
	int getAxisLengthY(void);
	int getAxisLengthZ(void);

	int getLowerSoftEndAxisX(void);
	int getLowerSoftEndAxisY(void);
	int getLowerSoftEndAxisZ(void);

	int getUpperSoftEndAxisX(void);
	int getUpperSoftEndAxisY(void);
	int getUpperSoftEndAxisZ(void);

	bool getHomingDirPosAxisX(void);
	bool getHomingDirPosAxisY(void);
	bool getHomingDirPosAxisZ(void);

	bool getMountingRightTopAxisX(void);
	bool getMountingRightTopAxisY(void);
	bool getMountingRightTopAxisZ(void);

	std::list<COMMAND_INFORMATION> m_inCommandsList;
};

#endif /* SAX_VICONFIGPARSER_H_ */


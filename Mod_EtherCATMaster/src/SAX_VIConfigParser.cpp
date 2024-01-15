/*
 * SAX_VIConfigParser.cpp
 *
 *  Created on: 16.04.2010
 *      Author: f.agrawal
 */
#include <cstdio>


#include "EtherCATMaster/SAX_VIConfigParser.h"

SAX_VIConfigParser::SAX_VIConfigParser() :
	_pLocator(0)
{
	ClearTempValues();

	m_oInfoSystemReady.m_oPresent = false;
	m_oInfoSystemErrorField.m_oPresent = false;
	m_oInfoSystemReadyFull.m_oPresent = false;
	m_oInfoSystemErrorFieldFull.m_oPresent = false;
}

SAX_VIConfigParser::~SAX_VIConfigParser()
{
}

void SAX_VIConfigParser::getSystemReadyInfo(struct InfoStruct &p_rSystemReadyInfo)
{
	p_rSystemReadyInfo.m_oPresent = m_oInfoSystemReady.m_oPresent;
	p_rSystemReadyInfo.m_oProductCode = m_oInfoSystemReady.m_oProductCode;
	p_rSystemReadyInfo.m_oVendorID = m_oInfoSystemReady.m_oVendorID;
	p_rSystemReadyInfo.m_oSlaveType = m_oInfoSystemReady.m_oSlaveType;
	p_rSystemReadyInfo.m_oInstance = m_oInfoSystemReady.m_oInstance;
	p_rSystemReadyInfo.m_oStartBit = m_oInfoSystemReady.m_oStartBit;
	p_rSystemReadyInfo.m_oLength = m_oInfoSystemReady.m_oLength;
}

void SAX_VIConfigParser::getSystemErrorFieldInfo(struct InfoStruct &p_rSystemErroFieldInfo)
{
	p_rSystemErroFieldInfo.m_oPresent = m_oInfoSystemErrorField.m_oPresent;
	p_rSystemErroFieldInfo.m_oProductCode = m_oInfoSystemErrorField.m_oProductCode;
	p_rSystemErroFieldInfo.m_oVendorID = m_oInfoSystemErrorField.m_oVendorID;
	p_rSystemErroFieldInfo.m_oSlaveType = m_oInfoSystemErrorField.m_oSlaveType;
	p_rSystemErroFieldInfo.m_oInstance = m_oInfoSystemErrorField.m_oInstance;
	p_rSystemErroFieldInfo.m_oStartBit = m_oInfoSystemErrorField.m_oStartBit;
	p_rSystemErroFieldInfo.m_oLength = m_oInfoSystemErrorField.m_oLength;
}

void SAX_VIConfigParser::getSystemReadyInfoFull(struct InfoStruct &p_rSystemReadyInfoFull)
{
	p_rSystemReadyInfoFull.m_oPresent = m_oInfoSystemReadyFull.m_oPresent;
	p_rSystemReadyInfoFull.m_oProductCode = m_oInfoSystemReadyFull.m_oProductCode;
	p_rSystemReadyInfoFull.m_oVendorID = m_oInfoSystemReadyFull.m_oVendorID;
	p_rSystemReadyInfoFull.m_oSlaveType = m_oInfoSystemReadyFull.m_oSlaveType;
	p_rSystemReadyInfoFull.m_oInstance = m_oInfoSystemReadyFull.m_oInstance;
	p_rSystemReadyInfoFull.m_oStartBit = m_oInfoSystemReadyFull.m_oStartBit;
	p_rSystemReadyInfoFull.m_oLength = m_oInfoSystemReadyFull.m_oLength;
}

void SAX_VIConfigParser::getSystemErrorFieldInfoFull(struct InfoStruct &p_rSystemErroFieldInfoFull)
{
	p_rSystemErroFieldInfoFull.m_oPresent = m_oInfoSystemErrorFieldFull.m_oPresent;
	p_rSystemErroFieldInfoFull.m_oProductCode = m_oInfoSystemErrorFieldFull.m_oProductCode;
	p_rSystemErroFieldInfoFull.m_oVendorID = m_oInfoSystemErrorFieldFull.m_oVendorID;
	p_rSystemErroFieldInfoFull.m_oSlaveType = m_oInfoSystemErrorFieldFull.m_oSlaveType;
	p_rSystemErroFieldInfoFull.m_oInstance = m_oInfoSystemErrorFieldFull.m_oInstance;
	p_rSystemErroFieldInfoFull.m_oStartBit = m_oInfoSystemErrorFieldFull.m_oStartBit;
	p_rSystemErroFieldInfoFull.m_oLength = m_oInfoSystemErrorFieldFull.m_oLength;
}

void SAX_VIConfigParser::ClearTempValues()
{
	m_parseSetSystemReadyStatus = false;
	m_parseSetSystemErrorField = false;
	m_parseSetSystemReadyStatusFull = false;
	m_parseSetSystemErrorFieldFull = false;

	m_oTempParseString.clear();

	m_getProductCode = false;
	m_getVendorID = false;
	m_getSlaveType = false;
	m_getInstance = false;
	m_getStartBit = false;
	m_getLength = false;

	m_getBitMapping = false;
	m_cntBits = 0;

	m_tempProductCode = 0;
	m_tempVendorID = 0;
	m_tempSlaveType = 0;
	m_tempInstance = 0;
	m_tempStartBit = 0;
	m_tempLength = 0;
}

// ContentHandler
void SAX_VIConfigParser::setDocumentLocator(const Locator* loc) {
	_pLocator = loc;
}

void SAX_VIConfigParser::startDocument() {
}

void SAX_VIConfigParser::endDocument() {

}

void SAX_VIConfigParser::startElement(const XMLString& uri,
		const XMLString& localName, const XMLString& qname,
		const Attributes& attributes) {

	//########## Outputs ######

	// SystemReadyStatus
	if (strcmp("SystemReadyStatus", localName.c_str()) == 0) {
		m_parseSetSystemReadyStatus = true;
	}
	// SystemErrorField
	if (strcmp("SystemErrorField", localName.c_str()) == 0) {
		m_parseSetSystemErrorField = true;
	}
	// SystemReadyStatusFull
	if (strcmp("SetSystemReadyStatusFull", localName.c_str()) == 0) {
		m_parseSetSystemReadyStatusFull = true;
	}
	// SystemErrorFieldFull
	if (strcmp("SetSystemErrorFieldFull", localName.c_str()) == 0) {
		m_parseSetSystemErrorFieldFull = true;
	}

	//########## General ######

	// ProductCode
	else if ((strcmp("ProductCode", localName.c_str()) == 0)) {
		m_getProductCode = true;
	}
	// VendorID
	else if (strcmp("VendorID", localName.c_str()) == 0) {
		m_getVendorID = true;
	}
	// SlaveType
	else if (strcmp("SlaveType", localName.c_str()) == 0) {
		m_getSlaveType = true;
	}
	// Instance
	else if ((strcmp("Instance", localName.c_str()) == 0)) {
		m_getInstance = true;
	}
	// StartBit
	else if ((strcmp("StartBit", localName.c_str()) == 0)) {
		m_getStartBit = true;
	}
	// Length
	else if ((strcmp("Length", localName.c_str()) == 0)) {
		m_getLength = true;
	}
	// Bits
	else if (strcmp("Bits", localName.c_str()) == 0) {
		m_cntBits = Tools::StringToInt(attributes.getValue(0));
	}
	// Output
	else if (strcmp("Output", localName.c_str()) == 0) {
		//safety check...
		if (m_cntBits-- > 0) {
			m_getBitMapping = true;
		} else {
			m_getBitMapping = false;
		}
	} // Input
	else if (strcmp("Input", localName.c_str()) == 0) {
	}

	//#############################################################
}

void SAX_VIConfigParser::endElement(const XMLString& uri,
		const XMLString& localName, const XMLString& qname) {

	//########## Outputs ######

	if ((strcmp("SystemReadyStatus", localName.c_str()) == 0)) {
		m_parseSetSystemReadyStatus = false;
		ClearTempValues();
	}
	if ((strcmp("SystemErrorField", localName.c_str()) == 0)) {
		m_parseSetSystemErrorField = false;
		ClearTempValues();
	}
	if ((strcmp("SetSystemReadyStatusFull", localName.c_str()) == 0)) {
		m_parseSetSystemReadyStatusFull = false;
		ClearTempValues();
	}
	if ((strcmp("SetSystemErrorFieldFull", localName.c_str()) == 0)) {
		m_parseSetSystemErrorFieldFull = false;
		ClearTempValues();
	}

	//########## General ######

	else if (strcmp("ProductCode", localName.c_str()) == 0) {
		m_tempProductCode = Tools::StringToInt(m_oTempParseString);
		m_oTempParseString.clear();
		m_getProductCode = false;
	}
	else if (strcmp("VendorID", localName.c_str()) == 0) {
		m_tempVendorID = Tools::StringToInt(m_oTempParseString);
		m_oTempParseString.clear();
		m_getVendorID = false;
	}
	else if (strcmp("SlaveType", localName.c_str()) == 0) {
		m_tempSlaveType = Tools::StringToInt(m_oTempParseString);
		m_oTempParseString.clear();
		m_getSlaveType = false;
	}
	else if (strcmp("Instance", localName.c_str()) == 0) {
		m_tempInstance = Tools::StringToInt(m_oTempParseString);
		m_oTempParseString.clear();
		m_getInstance = false;
	}
	else if (strcmp("StartBit", localName.c_str()) == 0) {
		m_tempStartBit = Tools::StringToInt(m_oTempParseString);
		m_oTempParseString.clear();
		m_getStartBit = false;
	}
	else if (strcmp("Length", localName.c_str()) == 0) {
		m_tempLength = Tools::StringToInt(m_oTempParseString);
		m_oTempParseString.clear();
		m_getLength = false;
	}
//	else if (strcmp("Bit", localName.c_str()) == 0) {
//		m_getBit = false;
//	}

    else if (strcmp("Output", localName.c_str()) == 0)
    {
        if(m_parseSetSystemReadyStatus)
        {
            if ((m_tempSlaveType == 0) ||
                (m_tempProductCode == 0) ||
                (m_tempVendorID == 0) ||
                (m_tempInstance == 0))
            {
                wmLogTr(eError, "QnxMsg.VI.VIConfErr1", "Invalid signal data of <%s> in VI_Config.xml, please correct !\n", "SystemReadyStatus");
                wmLogTr(eError, "QnxMsg.VI.VIConfErr2", "Signal <%s> is not activated!\n", "SystemReadyStatus");
                m_oInfoSystemReady.m_oPresent = false;
                m_oInfoSystemReady.m_oProductCode = 0;
                m_oInfoSystemReady.m_oVendorID = 0;
                m_oInfoSystemReady.m_oSlaveType = 0;
                m_oInfoSystemReady.m_oInstance = 0;
                m_oInfoSystemReady.m_oStartBit = 0;
                m_oInfoSystemReady.m_oLength = 0;
            }
            else
            {
                m_oInfoSystemReady.m_oPresent = true;
                m_oInfoSystemReady.m_oProductCode = m_tempProductCode;
                m_oInfoSystemReady.m_oVendorID = m_tempVendorID;
                m_oInfoSystemReady.m_oSlaveType = m_tempSlaveType;
                m_oInfoSystemReady.m_oInstance = m_tempInstance;
                m_oInfoSystemReady.m_oStartBit = m_tempStartBit;
                m_oInfoSystemReady.m_oLength = m_tempLength;
            }
        }
        if(m_parseSetSystemErrorField)
        {
            if ((m_tempSlaveType == 0) ||
                (m_tempProductCode == 0) ||
                (m_tempVendorID == 0) ||
                (m_tempInstance == 0))
            {
                wmLogTr(eError, "QnxMsg.VI.VIConfErr1", "Invalid signal data of <%s> in VI_Config.xml, please correct !\n", "SystemErrorField");
                wmLogTr(eError, "QnxMsg.VI.VIConfErr2", "Signal <%s> is not activated!\n", "SystemErrorField");
                m_oInfoSystemErrorField.m_oPresent = false;
                m_oInfoSystemErrorField.m_oProductCode = 0;
                m_oInfoSystemErrorField.m_oVendorID = 0;
                m_oInfoSystemErrorField.m_oSlaveType = 0;
                m_oInfoSystemErrorField.m_oInstance = 0;
                m_oInfoSystemErrorField.m_oStartBit = 0;
                m_oInfoSystemErrorField.m_oLength = 0;
            }
            else
            {
                m_oInfoSystemErrorField.m_oPresent = true;
                m_oInfoSystemErrorField.m_oProductCode = m_tempProductCode;
                m_oInfoSystemErrorField.m_oVendorID = m_tempVendorID;
                m_oInfoSystemErrorField.m_oSlaveType = m_tempSlaveType;
                m_oInfoSystemErrorField.m_oInstance = m_tempInstance;
                m_oInfoSystemErrorField.m_oStartBit = m_tempStartBit;
                m_oInfoSystemErrorField.m_oLength = m_tempLength;
            }
        }
        if(m_parseSetSystemReadyStatusFull)
        {
            if ((m_tempSlaveType == 0) ||
                (m_tempProductCode == 0) ||
                (m_tempVendorID == 0) ||
                (m_tempInstance == 0))
            {
                wmLogTr(eError, "QnxMsg.VI.VIConfErr1", "Invalid signal data of <%s> in VI_Config.xml, please correct !\n", "SetSystemReadyStatusFull");
                wmLogTr(eError, "QnxMsg.VI.VIConfErr2", "Signal <%s> is not activated!\n", "SetSystemReadyStatusFull");
                m_oInfoSystemReadyFull.m_oPresent = false;
                m_oInfoSystemReadyFull.m_oProductCode = 0;
                m_oInfoSystemReadyFull.m_oVendorID = 0;
                m_oInfoSystemReadyFull.m_oSlaveType = 0;
                m_oInfoSystemReadyFull.m_oInstance = 0;
                m_oInfoSystemReadyFull.m_oStartBit = 0;
                m_oInfoSystemReadyFull.m_oLength = 0;
            }
            else
            {
                m_oInfoSystemReadyFull.m_oPresent = true;
                m_oInfoSystemReadyFull.m_oProductCode = m_tempProductCode;
                m_oInfoSystemReadyFull.m_oVendorID = m_tempVendorID;
                m_oInfoSystemReadyFull.m_oSlaveType = m_tempSlaveType;
                m_oInfoSystemReadyFull.m_oInstance = m_tempInstance;
                m_oInfoSystemReadyFull.m_oStartBit = m_tempStartBit;
                m_oInfoSystemReadyFull.m_oLength = m_tempLength;
            }

        }
        if(m_parseSetSystemErrorFieldFull)
        {
            if ((m_tempSlaveType == 0) ||
                (m_tempProductCode == 0) ||
                (m_tempVendorID == 0) ||
                (m_tempInstance == 0))
            {
                wmLogTr(eError, "QnxMsg.VI.VIConfErr1", "Invalid signal data of <%s> in VI_Config.xml, please correct !\n", "SetSystemErrorFieldFull");
                wmLogTr(eError, "QnxMsg.VI.VIConfErr2", "Signal <%s> is not activated!\n", "SetSystemErrorFieldFull");
                m_oInfoSystemErrorFieldFull.m_oPresent = false;
                m_oInfoSystemErrorFieldFull.m_oProductCode = 0;
                m_oInfoSystemErrorFieldFull.m_oVendorID = 0;
                m_oInfoSystemErrorFieldFull.m_oSlaveType = 0;
                m_oInfoSystemErrorFieldFull.m_oInstance = 0;
                m_oInfoSystemErrorFieldFull.m_oStartBit = 0;
                m_oInfoSystemErrorFieldFull.m_oLength = 0;
            }
            else
            {
                m_oInfoSystemErrorFieldFull.m_oPresent = true;
                m_oInfoSystemErrorFieldFull.m_oProductCode = m_tempProductCode;
                m_oInfoSystemErrorFieldFull.m_oVendorID = m_tempVendorID;
                m_oInfoSystemErrorFieldFull.m_oSlaveType = m_tempSlaveType;
                m_oInfoSystemErrorFieldFull.m_oInstance = m_tempInstance;
                m_oInfoSystemErrorFieldFull.m_oStartBit = m_tempStartBit;
                m_oInfoSystemErrorFieldFull.m_oLength = m_tempLength;
            }
        }
    }
}

void SAX_VIConfigParser::characters(const XMLChar ch[], int start, int length) {
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
	if (m_getLength) {
		m_oTempParseString += std::string(ch + start, length);
	}
}

//NOT USED

void SAX_VIConfigParser::skippedEntity(const XMLString& name) {
}

void SAX_VIConfigParser::endPrefixMapping(const XMLString& prefix) {
}

void SAX_VIConfigParser::startPrefixMapping(const XMLString& prefix, const XMLString& uri) {
}

void SAX_VIConfigParser::processingInstruction(const XMLString& target, const XMLString& data) {
}

void SAX_VIConfigParser::ignorableWhitespace(const XMLChar ch[], int start, int length) {
}

void SAX_VIConfigParser::startDTD(const XMLString& name, const XMLString& publicId, const XMLString& systemId) {
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

//END OF NOT USED


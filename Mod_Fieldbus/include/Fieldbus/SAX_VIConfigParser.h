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
#include "Poco/Path.h"
#include <iostream>
#include <unistd.h>
#include <Tools.h>

#include "module/moduleLogger.h"

using Poco::XML::XMLString;
using Poco::XML::XMLChar;
using Poco::XML::SAXParser;
using Poco::XML::XMLReader;
using Poco::XML::XMLChar;
using Poco::XML::Attributes;
using Poco::XML::Locator;
using Poco::XML::ContentHandler;
using Poco::XML::LexicalHandler;
using namespace precitec;

struct InfoStruct
{
    bool m_oPresent;
    unsigned int m_oProductCode;
    unsigned int m_oVendorID;
    int m_oSlaveType;
    int m_oInstance;
    int m_oStartBit;
    int m_oLength;
};

/**
 * SAX_VIConfigParser, parst VI_Config.xml
 **/
class SAX_VIConfigParser: public ContentHandler, public LexicalHandler{

public:
    SAX_VIConfigParser();
    virtual ~SAX_VIConfigParser();

    void getSystemReadyInfo(struct InfoStruct &p_rSystemReadyInfo);
    void getSystemErrorFieldInfo(struct InfoStruct &p_rSystemErrorFieldInfo);
    void getSystemReadyInfoFull(struct InfoStruct &p_rSystemReadyInfoFull);
    void getSystemErrorFieldInfoFull(struct InfoStruct &p_rSystemErrorFieldInfoFull);
    void getS6KSystemFaultInfo(struct InfoStruct &p_rSystemFaultInfo);
    void getS6KSystemReadyInfo(struct InfoStruct &p_rSystemReadyInfo);

private:
    virtual void characters(const XMLChar ch[], int start, int length);
    virtual void endElement(const XMLString& uri, const XMLString& localName, const XMLString& qname);
    virtual void startElement(const XMLString& uri, const XMLString& localName, const XMLString& qname,
                              const Attributes& attributes);
    virtual void endDocument();
    virtual void startDocument();
    virtual void setDocumentLocator(const Locator* loc);

    // Not used
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
    //END of Not used

    void ClearTempValues();

    const Locator* _pLocator;

    // Outputs
    bool m_parseSetSystemReadyStatus; //Enable/Disable SystemReady
    bool m_parseSetSystemErrorField; //Enable/Disable SystemErrorField
    bool m_parseSetSystemReadyStatusFull; //Enable/Disable SystemReady ueber Interface Full
    bool m_parseSetSystemErrorFieldFull; //Enable/Disable SystemErrorField ueber Interface Full
    bool m_parseSetS6KSystemFaultStatus; //Enable/Disable S6KSystemFault
    bool m_parseSetS6KSystemReadyStatus; //Enable/Disable S6KSystemReady

    std::string m_oTempParseString;

    bool m_getProductCode;
    bool m_getVendorID;
    bool m_getSlaveType;
    bool m_getInstance;
    bool m_getStartBit;
    bool m_getLength;

    bool m_getBitMapping;
    int m_cntBits;

    int m_tempProductCode;
    int m_tempVendorID;
    int m_tempSlaveType;
    int m_tempInstance;
    int m_tempStartBit;
    int m_tempLength;

    struct InfoStruct m_oInfoSystemReady;
    struct InfoStruct m_oInfoSystemErrorField;
    struct InfoStruct m_oInfoSystemReadyFull;
    struct InfoStruct m_oInfoSystemErrorFieldFull;
    struct InfoStruct m_oInfoS6KSystemFault;
    struct InfoStruct m_oInfoS6KSystemReady;
};

#endif /* SAX_VICONFIGPARSER_H_ */


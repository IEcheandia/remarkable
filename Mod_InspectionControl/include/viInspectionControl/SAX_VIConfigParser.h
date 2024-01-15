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
#include <iostream>
#include <Tools.h>
#include <list>

#include "event/ethercatOutputs.h"
#include "VIDefs.h"

namespace precitec
{

namespace ethercat
{

using namespace Poco::XML;

struct HM_GATEWAY_INFORMATION
{
    bool isValid;
    int nSlaveType;
    unsigned int nProductCode;
    unsigned int nVendorID;
    unsigned int nInstance;

    unsigned int bitGlasNotPresent;
    unsigned int bitGlasDirty;
    unsigned int bitTempGlasFail;
    unsigned int bitTempHeadFail;
};

/**
 * SAX_VIConfigParser, parst VI_Config.xml
 **/
class SAX_VIConfigParser: public ContentHandler, public LexicalHandler{

public:
    SAX_VIConfigParser();
    virtual ~SAX_VIConfigParser();

    bool getProductNumberExtern() { return m_oProductNumberExtern; }
    bool IsStartCycleAnalogInput() { return m_oIsStartCycleAnalogInput; }
    bool IsStartSeamAnalogInput() { return m_oIsStartSeamAnalogInput; }

    /**
    * Vector mit den Kommandoinformationen (Mapping)
    **/
    std::vector<COMMAND_INFORMATION> m_inCommandList; ///< von der HW (Kunde)

    /**
    * Vector mit den Kommandoinformationen (Mapping)
    **/
    std::vector<COMMAND_INFORMATION> m_outCommandList; ///< an die HW (Kunde)

    //HeadmonitorGateway
    HM_GATEWAY_INFORMATION m_HMinGatewayInfo;
    HM_GATEWAY_INFORMATION m_HMoutGatewayInfo;

private:
    virtual void startDocument();
    virtual void endDocument();
    virtual void startElement(const XMLString& uri, const XMLString& localName, const XMLString& qname,
                              const Attributes& attributes);
    virtual void endElement(const XMLString& uri, const XMLString& localName, const XMLString& qname);
    virtual void characters(const XMLChar ch[], int start, int length);
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
    // End of Not used

    void ClearTempValues();
    void insertInOutCommandList(const CommandType pCommand, const std::string& p_rCommandName, const int pLength);
    void insertInInCommandList(const CommandType pCommand, const std::string& p_rCommandName, const int pLength);

    const Locator* _pLocator;

    std::string m_oTempParseString;

    bool m_oGetProductCode;
    bool m_oGetVendorID;
    bool m_oGetSlaveType;
    bool m_oGetInstance;
    bool m_oGetStartBit;
    bool m_oGetTriggerLevel;
    bool m_oGetLength;

    int m_oTempProductCode;
    int m_oTempVendorID;
    int m_oTempSlaveType;
    int m_oTempInstance;
    int m_oTempStartBit;
    int m_oTempTriggerLevel;
    int m_oTempLength;

    int m_oBitGlasNotPresent;
    int m_oBitGlasDirty;
    int m_oBitTempGlasFail;
    int m_oBitTempHeadFail;

    bool m_oProductNumberExtern; ///< Seriennummer Bauteil wird von extern eingelesen
    bool m_oIsStartCycleAnalogInput; ///< start cycle signal is read via analog input
    bool m_oIsStartSeamAnalogInput; ///< start seam signal is read via analog input

    bool m_oIsInvertedEmergencyStopActivated;

    // Inputs
    bool m_oParseGetTriggerContinuouslyMode; //Endlosbetrieb Start/Stop
    bool m_oParseGetTriggerAutomaticMode; //Automatikbetrieb Start/Stop
    bool m_oParseGetTriggerInspectionInfo; //Nahtfolge uebernehmen
    bool m_oParseGetTriggerInspectionPreStart; // Seam PreStart
    bool m_oParseGetTriggerInspectionStartEnd; //Inspektion Naht aktiv
    bool m_oParseGetChangeToStandardMode; //next cycle is started in standard mode
    bool m_oParseGetTriggerUnblockLineLaser; // Der Linienlaser darf ein- oder muss ausgeschaltet werden
    bool m_oParseGetSeamSeries; //Nahtfolgenummer
    bool m_oParseGetSeamNr; //Nahtnummer
    bool m_oParseGetTriggerQuitSystemFault; // Quittierung eines anstehenden Systemfehlers
    bool m_oParseGetCalibrationType; // Art der gewuenschten Kalibration
    bool m_oParseGetTriggerCalibrationMode; //Kalibrationsbetrieb Start/Stop
    bool m_oParseGetTriggerHomingYAxis; // Start Referenzieren der Y-Achse
    bool m_oParseGetProductType; //Bauteil-Typ
    bool m_oParseGetProductNumber; //Bauteil-Nummer, Seriennummer
    bool m_oParseGetExtendedProductInfo; //Bauteil-Nummer, Seriennummer
    bool m_oParseGenPurposeDigIn1; // General Purpose 16 Bit Input via Fieldbus for Processing in Filter-Graph
    bool m_oParseGenPurposeDigInAddress;  // General Purpose 16 Bit Input Address via Fieldbus
    bool m_oParseGenPurposeDigInTakeOver; // General Purpose 16 Bit Input Takeover via Fieldbus
    bool m_oParseGenPurposeDigOutTakeOver; // General Purpose 16 Bit Output Takeover via Fieldbus

    bool m_oParseGetInvertedTriggerEmergencyStop; //emergency stop

    bool m_oParseHeadmonitorGateway;

    bool m_oParseTriggerProductNumberFull; // Uebernahme Bauteil-Nummer,Seriennummer ueber Interface Full
    bool m_oParseAcknResultsReadyFull; // Quittierung Resultate ueber Interface Full
    bool m_oParseTriggerQuitSystemFaultFull; // Quittierung eines anstehenden Systemfehlers
    bool m_oParseGetProductTypeFull; // Bauteil-Typ ueber Interface Full
    bool m_oParseGetProductNumberFull; // Bauteil-Nummer,Seriennummer ueber Interface Full

    bool m_oParse_S6K_SouvisActive;
    bool m_oParse_S6K_SouvisInspection;
    bool m_oParse_S6K_QuitSystemFault;
    bool m_oParse_S6K_MachineReady;
    bool m_oParse_S6K_ProductNumber;
    bool m_oParse_S6K_AcknQualityData;
    bool m_oParse_S6K_CycleDataValid;
    bool m_oParse_S6K_CycleData;
    bool m_oParse_S6K_SeamNo;
    bool m_oParse_S6K_SeamNoValid;
    bool m_oParse_S6K_AcknResultData;
    bool m_oParse_S6K_MakePictures;

    bool m_oParse_SM_AcknowledgeStep;

    // Outputs
    bool m_oParseSetInspectCycleAckn; //Set/Reset InspectCycleAckn
    bool m_oParseSetInspectionPreStartAckn; // Set/Reset m_oParseSetInspectionPreStartAckn
    bool m_oParseSetInspectionStartEndAckn; // Set/Reset m_oParseSetInspectionStartEndAckn
    bool m_oParseSetInspectionOK; //Set/Reset InspectionOK
    bool m_oParseSetSumErrorLatched; //Set/Reset SumErrorLatched
    bool m_oParseSetInspectionIncomplete; //Set/Reset InspectionIncomplete
    bool m_oParseSetQualityErrorField; //Enable/Disable QualityErrorField
    bool m_oParseSetSumErrorSeam; //Set/Reset SetSumErrorSeam
    bool m_oParseSetSumErrorSeamSeries; //Set/Reset SetSumErrorSeamSeries
    bool m_oParseSetSystemReadyStatus; //Enable/Disable SystemReady
    bool m_oParseSetSystemErrorField; //Enable/Disable SystemErrorField
    bool m_oParseSetCalibResultsField; //Enable/Disable Field (3 Bits) for Calibration Results
    bool m_oParseSetPositionResultsField; //Enable/Disable Field for Position Output
    bool m_oParseSetProductType; //Enable/Disable Bauteil-Typ Spiegelung
    bool m_oParseSetProductNumber; //Enable/Disable Bauteil-Nummer Spiegelung
    bool m_oParseSetExtendedProductInfo; //Enable/Disable Bauteil-Nummer Spiegelung
    bool m_oParseSetGenPurposeDigInAckn; //Set/Reset GenPurposeDigInAckn

    bool m_oParseCabinetTemperatureOk; //parseCabinetTemperature

    bool m_oParseAcknProductNumberFull; // Quittierung Bauteil-Nummer,Seriennummer ueber Interface Full
    bool m_oParseTriggerResultsReadyFull; // Resultate ueber Interface Full
    bool m_oParseSetInspectionOKFull; //Set/Reset InspectionOK ueber Interface Full
    bool m_oParseSetSumErrorLatchedFull; //Set/Reset SumErrorLatched ueber Interface Full
    bool m_oParseSetQualityErrorFieldFull; //Enable/Disable QualityErrorField ueber Interface Full
    bool m_oParseSetSystemReadyStatusFull; //Enable/Disable SystemReady ueber Interface Full
    bool m_oParseSetCabinetTemperatureOkFull; //Enable/Disable CabinetTemperatureOk ueber Interface Full
    bool m_oParseSetHMSignalsFieldFull; //Enable/Disable HMSignalsField ueber Interface Full
    bool m_oParseSetSystemErrorFieldFull; //Enable/Disable SystemErrorField ueber Interface Full
    bool m_oParseSetProductTypeFull; //Enable/Disable Bauteil-Typ Spiegelung ueber Interface Full
    bool m_oParseSetProductNumberFull; // Bauteil-Nummer,Seriennummer ueber Interface Full

    bool m_oParse_S6K_SystemFault;
    bool m_oParse_S6K_SystemReady;
    bool m_oParse_S6K_FastStop_DoubleBlank;
    bool m_oParse_S6K_SeamErrorCat1;
    bool m_oParse_S6K_SeamErrorCat2;
    bool m_oParse_S6K_QualityDataValid;
    bool m_oParse_S6K_AcknCycleData;
    bool m_oParse_S6K_CycleDataMirror;
    bool m_oParse_S6K_SeamNoMirror;
    bool m_oParse_S6K_AcknSeamNo;
    bool m_oParse_S6K_ResultDataValid;
    bool m_oParse_S6K_ResultDataCount;
    bool m_oParse_S6K_ResultsImage1;
    bool m_oParse_S6K_ResultsImage2;
    bool m_oParse_S6K_ResultsImage3;
    bool m_oParse_S6K_ResultsImage4;
    bool m_oParse_S6K_ResultsImage5;
    bool m_oParse_S6K_ResultsImage6;
    bool m_oParse_S6K_ResultsImage7;
    bool m_oParse_S6K_ResultsImage8;
    bool m_oParse_S6K_ResultsImage9;
    bool m_oParse_S6K_ResultsImage10;
    bool m_oParse_S6K_ResultsImage11;
    bool m_oParse_S6K_ResultsImage12;
    bool m_oParse_S6K_ResultsImage13;
    bool m_oParse_S6K_ResultsImage14;
    bool m_oParse_S6K_ResultsImage15;
    bool m_oParse_S6K_ResultsImage16;
    bool m_oParse_S6K_ResultsImage17;
    bool m_oParse_S6K_ResultsImage18;

    bool m_oParse_SM_ProcessingActive;
    bool m_oParse_SM_TakeoverStep;
    bool m_oParse_SM_StepField;
};

} // namespace ethercat
} // namespace precitec

#endif /* SAX_VICONFIGPARSER_H_ */


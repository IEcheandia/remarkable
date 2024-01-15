/*
 * SAX_VIConfigParser.cpp
 *
 *  Created on: 16.04.2010
 *      Author: f.agrawal
 */

#include "viInspectionControl/SAX_VIConfigParser.h"

#include "event/ethercatDefines.h"
#include "common/systemConfiguration.h"

#include "module/moduleLogger.h"

namespace precitec
{

    using namespace interface;

namespace ethercat
{

SAX_VIConfigParser::SAX_VIConfigParser():
    _pLocator(0),
    m_oProductNumberExtern(false),
    m_oIsStartCycleAnalogInput(false),
    m_oIsStartSeamAnalogInput(false)
{
    m_HMinGatewayInfo.isValid = false;
    m_HMoutGatewayInfo.isValid = false;

    ClearTempValues();

    m_oIsInvertedEmergencyStopActivated = SystemConfiguration::instance().getBool("EmergencyStopSignalEnable", false);
    wmLog(eDebug, "m_oIsInvertedEmergencyStopActivated (bool): %d\n", m_oIsInvertedEmergencyStopActivated);
}

SAX_VIConfigParser::~SAX_VIConfigParser()
{
}

void SAX_VIConfigParser::ClearTempValues()
{
    m_oTempParseString.clear();

    m_oGetProductCode = false;
    m_oGetVendorID = false;
    m_oGetSlaveType = false;
    m_oGetInstance = false;
    m_oGetStartBit = false;
    m_oGetTriggerLevel = false;
    m_oGetLength = false;

    m_oTempProductCode = 0;
    m_oTempVendorID = 0;
    m_oTempSlaveType = 0;
    m_oTempInstance = 0;
    m_oTempStartBit = 0;
    m_oTempTriggerLevel = 0;
    m_oTempLength = 0;

    m_oBitGlasNotPresent = 0;
    m_oBitGlasDirty = 0;
    m_oBitTempGlasFail = 0;
    m_oBitTempHeadFail = 0;

    // Inputs
    m_oParseGetTriggerContinuouslyMode = false;
    m_oParseGetTriggerAutomaticMode = false;
    m_oParseGetTriggerInspectionInfo = false;
    m_oParseGetTriggerInspectionPreStart = false;
    m_oParseGetTriggerInspectionStartEnd = false;
    m_oParseGetChangeToStandardMode = false;
    m_oParseGetTriggerUnblockLineLaser = false;
    m_oParseGetSeamSeries = false;
    m_oParseGetSeamNr = false;
    m_oParseGetTriggerQuitSystemFault = false;
    m_oParseGetCalibrationType = false;
    m_oParseGetTriggerCalibrationMode = false;
    m_oParseGetTriggerHomingYAxis = false;
    m_oParseGetProductType = false;
    m_oParseGetProductNumber = false;
    m_oParseGetExtendedProductInfo = false;
    m_oParseGenPurposeDigIn1 = false;
    m_oParseGenPurposeDigInAddress = false;
    m_oParseGenPurposeDigInTakeOver = false;
    m_oParseGenPurposeDigOutTakeOver = false;

    m_oParseGetInvertedTriggerEmergencyStop = false;

    //HeadMonitor
    m_oParseHeadmonitorGateway = false;

    m_oParseTriggerProductNumberFull = false;
    m_oParseAcknResultsReadyFull = false;
    m_oParseTriggerQuitSystemFaultFull = false;
    m_oParseGetProductTypeFull = false;
    m_oParseGetProductNumberFull = false;

    m_oParse_S6K_SouvisActive = false;
    m_oParse_S6K_SouvisInspection = false;
    m_oParse_S6K_QuitSystemFault = false;
    m_oParse_S6K_MachineReady = false;
    m_oParse_S6K_ProductNumber = false;
    m_oParse_S6K_AcknQualityData = false;
    m_oParse_S6K_CycleDataValid = false;
    m_oParse_S6K_CycleData = false;
    m_oParse_S6K_SeamNo = false;
    m_oParse_S6K_SeamNoValid = false;
    m_oParse_S6K_AcknResultData = false;
    m_oParse_S6K_MakePictures = false;

    m_oParse_SM_AcknowledgeStep = false;

    // Outputs
    m_oParseSetInspectCycleAckn = false;
    m_oParseSetInspectionPreStartAckn = false;
    m_oParseSetInspectionStartEndAckn = false;
    m_oParseSetInspectionOK = false;
    m_oParseSetSumErrorLatched = false;
    m_oParseSetInspectionIncomplete = false;
    m_oParseSetQualityErrorField = false;
    m_oParseSetSumErrorSeam = false;
    m_oParseSetSumErrorSeamSeries = false;
    m_oParseSetSystemReadyStatus = false;
    m_oParseSetSystemErrorField = false;
    m_oParseSetCalibResultsField = false;
    m_oParseSetPositionResultsField = false;
    m_oParseSetProductType = false;
    m_oParseSetProductNumber = false;
    m_oParseSetExtendedProductInfo = false;
    m_oParseSetGenPurposeDigInAckn = false;

    m_oParseCabinetTemperatureOk = false;

    m_oParseAcknProductNumberFull = false;
    m_oParseTriggerResultsReadyFull = false;
    m_oParseSetInspectionOKFull = false;
    m_oParseSetSumErrorLatchedFull = false;
    m_oParseSetQualityErrorFieldFull = false;
    m_oParseSetSystemReadyStatusFull = false;
    m_oParseSetCabinetTemperatureOkFull = false;
    m_oParseSetHMSignalsFieldFull = false;
    m_oParseSetSystemErrorFieldFull = false;
    m_oParseSetProductTypeFull = false;
    m_oParseSetProductNumberFull = false;

    m_oParse_S6K_SystemFault = false;
    m_oParse_S6K_SystemReady = false;
    m_oParse_S6K_FastStop_DoubleBlank = false;
    m_oParse_S6K_SeamErrorCat1 = false;
    m_oParse_S6K_SeamErrorCat2 = false;
    m_oParse_S6K_QualityDataValid = false;
    m_oParse_S6K_AcknCycleData = false;
    m_oParse_S6K_CycleDataMirror = false;
    m_oParse_S6K_SeamNoMirror = false;
    m_oParse_S6K_AcknSeamNo = false;
    m_oParse_S6K_ResultDataValid = false;
    m_oParse_S6K_ResultDataCount = false;
    m_oParse_S6K_ResultsImage1 = false;
    m_oParse_S6K_ResultsImage2 = false;
    m_oParse_S6K_ResultsImage3 = false;
    m_oParse_S6K_ResultsImage4 = false;
    m_oParse_S6K_ResultsImage5 = false;
    m_oParse_S6K_ResultsImage6 = false;
    m_oParse_S6K_ResultsImage7 = false;
    m_oParse_S6K_ResultsImage8 = false;
    m_oParse_S6K_ResultsImage9 = false;
    m_oParse_S6K_ResultsImage10 = false;
    m_oParse_S6K_ResultsImage11 = false;
    m_oParse_S6K_ResultsImage12 = false;
    m_oParse_S6K_ResultsImage13 = false;
    m_oParse_S6K_ResultsImage14 = false;
    m_oParse_S6K_ResultsImage15 = false;
    m_oParse_S6K_ResultsImage16 = false;
    m_oParse_S6K_ResultsImage17 = false;
    m_oParse_S6K_ResultsImage18 = false;

    m_oParse_SM_ProcessingActive = false;
    m_oParse_SM_TakeoverStep = false;
    m_oParse_SM_StepField = false;
}

void SAX_VIConfigParser::insertInOutCommandList(const CommandType pCommand, const std::string& p_rCommandName, const int pLength)
{
    if ((m_oTempSlaveType == 0) ||
        (m_oTempProductCode == 0) ||
        (m_oTempVendorID == 0) ||
        (m_oTempInstance == 0))
    {
        wmLogTr(eError, "QnxMsg.VI.VIConfErr1", "Invalid signal data of <%s> in VI_Config.xml, please correct !\n", p_rCommandName.c_str());
        wmLogTr(eError, "QnxMsg.VI.VIConfErr2", "Signal <%s> is not activated!\n", p_rCommandName.c_str());
        ClearTempValues();
        return;
    }

    SLAVE_PROXY_INFORMATION info;
    info.nSlaveType = m_oTempSlaveType;
    info.nProductCode = m_oTempProductCode;
    info.nVendorID = m_oTempVendorID;
    info.nInstance = m_oTempInstance;
    info.nStartBit = m_oTempStartBit;
    info.nTriggerLevel = m_oTempTriggerLevel;
    info.nLength = pLength;

    COMMAND_INFORMATION commandInfo;
    commandInfo.proxyInfo = info;
    commandInfo.commandType = pCommand;

    m_outCommandList.push_back(commandInfo);
    ClearTempValues();
}

void SAX_VIConfigParser::insertInInCommandList(const CommandType pCommand, const std::string& p_rCommandName, const int pLength)
{
    if ((m_oTempSlaveType == 0) ||
        (m_oTempProductCode == 0) ||
        (m_oTempVendorID == 0) ||
        (m_oTempInstance == 0))
    {
        wmLogTr(eError, "QnxMsg.VI.VIConfErr1", "Invalid signal data of <%s> in VI_Config.xml, please correct !\n", p_rCommandName.c_str());
        wmLogTr(eError, "QnxMsg.VI.VIConfErr2", "Signal <%s> is not activated!\n", p_rCommandName.c_str());
        ClearTempValues();
        return;
    }

    SLAVE_PROXY_INFORMATION info;
    info.nSlaveType = m_oTempSlaveType;
    info.nProductCode = m_oTempProductCode;
    info.nVendorID = m_oTempVendorID;
    info.nInstance = m_oTempInstance;
    info.nStartBit = m_oTempStartBit;
    info.nTriggerLevel = m_oTempTriggerLevel;
    info.nLength = pLength;
    if (m_oTempSlaveType == -1)
    {
        info.m_oChannel = eChannel1;
    }
    else if (m_oTempSlaveType == -2)
    {
        info.m_oChannel = eChannel2;
    }

    COMMAND_INFORMATION commandInfo;
    commandInfo.proxyInfo = info;
    commandInfo.commandType = pCommand;

    m_inCommandList.push_back(commandInfo);
    ClearTempValues();
}

void SAX_VIConfigParser::startDocument()
{
    m_oProductNumberExtern = false;
    m_oIsStartCycleAnalogInput = false;
    m_oIsStartSeamAnalogInput = false;
}

void SAX_VIConfigParser::endDocument()
{
}

void SAX_VIConfigParser::startElement(const XMLString& uri, const XMLString& localName, const XMLString& qname,
                                      const Attributes& attributes)
{
    bool printFlag = true;

    //########## Inputs ######

    // TriggerContinuouslyMode
    if (strcmp("TriggerContinuouslyMode", localName.c_str()) == 0)
    {
        m_oParseGetTriggerContinuouslyMode = true;
    }
    // TriggerAutomaticMode
    else if (strcmp("TriggerAutomaticMode", localName.c_str()) == 0)
    {
        m_oParseGetTriggerAutomaticMode = true;
    }
    // TriggerInspectionInfo
    else if (strcmp("TriggerInspectionInfo", localName.c_str()) == 0)
    {
        m_oParseGetTriggerInspectionInfo = true;
    }
    // TriggerInspectionPreStart
    else if (strcmp("TriggerInspectionPreStart", localName.c_str()) == 0)
    {
        m_oParseGetTriggerInspectionPreStart = true;
    }
    // TriggerInspectionStartEnd
    else if (strcmp("TriggerInspectionStartEnd", localName.c_str()) == 0)
    {
        m_oParseGetTriggerInspectionStartEnd = true;
    }
    // ChangeToStandardMode
    else if (strcmp("ChangeToStandardMode", localName.c_str()) == 0)
    {
        m_oParseGetChangeToStandardMode = true;
    }
    // TriggerUnblockLineLaser
    else if (strcmp("TriggerUnblockLineLaser", localName.c_str()) == 0)
    {
        m_oParseGetTriggerUnblockLineLaser = true;
    }
    // Seamseries
    else if (strcmp("Seamseries", localName.c_str()) == 0)
    {
        m_oParseGetSeamSeries = true;
    }
    // SeamNr
    else if (strcmp("SeamNr", localName.c_str()) == 0)
    {
        m_oParseGetSeamNr = true;
    }
    // TriggerQuitSystemFault
    else if (strcmp("TriggerQuitSystemFault", localName.c_str()) == 0)
    {
        m_oParseGetTriggerQuitSystemFault = true;
    }
    // CalibrationType
    else if (strcmp("CalibrationType", localName.c_str()) == 0)
    {
        m_oParseGetCalibrationType = true;
    }
    // TriggerCalibrationMode
    else if (strcmp("TriggerCalibrationMode", localName.c_str()) == 0)
    {
        m_oParseGetTriggerCalibrationMode = true;
    }
    // TriggerHomingYAxis
    else if (strcmp("TriggerHomingYAxis", localName.c_str()) == 0)
    {
        m_oParseGetTriggerHomingYAxis = true;
    }
    // ProductType
    else if (strcmp("ProductType", localName.c_str()) == 0)
    {
        m_oParseGetProductType = true;
    }
    // ProductNumber
    else if (strcmp("ProductNumber", localName.c_str()) == 0)
    {
        m_oParseGetProductNumber = true;
    }
    // ExtendedProductInfo
    else if (strcmp("ExtendedProductInfo_UTF8", localName.c_str()) == 0)
    {
        m_oParseGetExtendedProductInfo = true;
    }
    // GeneralPurposeDigitalInput1
    else if (strcmp("GeneralPurposeDigitalInput1", localName.c_str()) == 0)
    {
        m_oParseGenPurposeDigIn1 = true;
    }
    // GeneralPurposeDigitalInputAddress
    else if (strcmp("GeneralPurposeDigitalInputAddress", localName.c_str()) == 0)
    {
        m_oParseGenPurposeDigInAddress = true;
    }
    // GeneralPurposeDigitalInputTakeOver
    else if (strcmp("GeneralPurposeDigitalInputTakeOver", localName.c_str()) == 0)
    {
        m_oParseGenPurposeDigInTakeOver = true;
    }
    // GeneralPurposeDigitalOutputTakeOver
    else if (strcmp("GeneralPurposeDigitalOutputTakeOver", localName.c_str()) == 0)
    {
        m_oParseGenPurposeDigOutTakeOver = true;
    }
    // InvertedTriggermEmergencyStop
    else if (m_oIsInvertedEmergencyStopActivated && strcmp("InvertedTriggerEmergencyStop", localName.c_str()) == 0)
    {
        m_oParseGetInvertedTriggerEmergencyStop = true;
    }
    // TriggerProductNumberFull
    else if (strcmp("TriggerProductNumberFull", localName.c_str()) == 0)
    {
        m_oParseTriggerProductNumberFull = true;
    }
    // AcknResultsReadyFull
    else if (strcmp("AcknResultsReadyFull", localName.c_str()) == 0)
    {
        m_oParseAcknResultsReadyFull = true;
    }
    // TriggerQuitSystemFaultFull
    else if (strcmp("TriggerQuitSystemFaultFull", localName.c_str()) == 0)
    {
        m_oParseTriggerQuitSystemFaultFull = true;
    }
    // GetProductTypeFull
    else if (strcmp("GetProductTypeFull", localName.c_str()) == 0)
    {
        m_oParseGetProductTypeFull = true;
    }
    // GetProductNumberFull
    else if (strcmp("GetProductNumberFull", localName.c_str()) == 0)
    {
        m_oParseGetProductNumberFull = true;
    }

    // S6K_SouvisActive
    else if (strcmp("S6K_SouvisActive", localName.c_str()) == 0)
    {
        m_oParse_S6K_SouvisActive = true;
    }
    // S6K_SouvisInspection
    else if (strcmp("S6K_SouvisInspection", localName.c_str()) == 0)
    {
        m_oParse_S6K_SouvisInspection = true;
    }
    // S6K_QuitSystemFault
    else if (strcmp("S6K_QuitSystemFault", localName.c_str()) == 0)
    {
        m_oParse_S6K_QuitSystemFault = true;
    }
    // S6K_MachineReady
    else if (strcmp("S6K_MachineReady", localName.c_str()) == 0)
    {
        m_oParse_S6K_MachineReady = true;
    }
    // S6K_ProductNumber
    else if (strcmp("S6K_ProductNumber", localName.c_str()) == 0)
    {
        m_oParse_S6K_ProductNumber = true;
    }
    // S6K_AcknQualityData
    else if (strcmp("S6K_AcknQualityData", localName.c_str()) == 0)
    {
        m_oParse_S6K_AcknQualityData = true;
    }
    // S6K_CycleDataValid
    else if (strcmp("S6K_CycleDataValid", localName.c_str()) == 0)
    {
        m_oParse_S6K_CycleDataValid = true;
    }
    // S6K_CycleData
    else if (strcmp("S6K_CycleData", localName.c_str()) == 0)
    {
        m_oParse_S6K_CycleData = true;
    }
    // S6K_SeamNo
    else if (strcmp("S6K_SeamNo", localName.c_str()) == 0)
    {
        m_oParse_S6K_SeamNo = true;
    }
    // S6K_SeamNoValid
    else if (strcmp("S6K_SeamNoValid", localName.c_str()) == 0)
    {
        m_oParse_S6K_SeamNoValid = true;
    }
    // S6K_AcknResultData
    else if (strcmp("S6K_AcknResultData", localName.c_str()) == 0)
    {
        m_oParse_S6K_AcknResultData = true;
    }
    // S6K_MakePictures
    else if (strcmp("S6K_MakePictures", localName.c_str()) == 0)
    {
        m_oParse_S6K_MakePictures = true;
    }
    // SM_AcknowledgeStep
    else if (strcmp("SM_AcknowledgeStep", localName.c_str()) == 0)
    {
        m_oParse_SM_AcknowledgeStep = true;
    }

    //########## Outputs ######

    // InspectCycleAckn
    else if (strcmp("InspectCycleAckn", localName.c_str()) == 0)
    {
        m_oParseSetInspectCycleAckn = true;
    }
    // InspectionPreStartAckn
    else if (strcmp("InspectionPreStartAckn", localName.c_str()) == 0)
    {
        m_oParseSetInspectionPreStartAckn = true;
    }
    // InspectionStartEndAckn
    else if (strcmp("InspectionStartEndAckn", localName.c_str()) == 0)
    {
        m_oParseSetInspectionStartEndAckn = true;
    }
    // InspectionOK
    else if (strcmp("InspectionOK", localName.c_str()) == 0)
    {
        m_oParseSetInspectionOK = true;
    }
    // SumErrorLatched
    else if (strcmp("SumErrorLatched", localName.c_str()) == 0)
    {
        m_oParseSetSumErrorLatched = true;
    }
    // InspectionIncomplete
    else if (strcmp("InspectionIncomplete", localName.c_str()) == 0)
    {
        m_oParseSetInspectionIncomplete = true;
    }
    // QualityErrorField
    else if (strcmp("QualityErrorField", localName.c_str()) == 0)
    {
        m_oParseSetQualityErrorField = true;
    }
    // SumErrorSeam
    else if (strcmp("SumErrorSeam", localName.c_str()) == 0)
    {
        m_oParseSetSumErrorSeam = true;
    }
    // SumErrorSeamSeries
    else if (strcmp("SumErrorSeamSeries", localName.c_str()) == 0)
    {
        m_oParseSetSumErrorSeamSeries = true;
    }
    // SystemReadyStatus
    else if (strcmp("SystemReadyStatus", localName.c_str()) == 0)
    {
        m_oParseSetSystemReadyStatus = true;
    }
    // SystemErrorField
    else if (strcmp("SystemErrorField", localName.c_str()) == 0)
    {
        m_oParseSetSystemErrorField = true;
    }
    // CalibResultsField
    else if (strcmp("CalibResultsField", localName.c_str()) == 0)
    {
        m_oParseSetCalibResultsField = true;
    }
    // PositionResultsField
    else if (strcmp("PositionResultsField", localName.c_str()) == 0)
    {
        m_oParseSetPositionResultsField = true;
    }
    // Product-Type Spiegelung
    else if (strcmp("ProductTypeMirror", localName.c_str()) == 0)
    {
        m_oParseSetProductType = true;
    }
    // Product-Number Spiegelung
    else if (strcmp("ProductNumberMirror", localName.c_str()) == 0)
    {
        m_oParseSetProductNumber = true;
    }
    // ExtendedProductInfo Spiegelung
    else if (strcmp("ExtendedProductInfoMirror_UTF8", localName.c_str()) == 0)
    {
        m_oParseSetExtendedProductInfo = true;
    }
    // Schaltschrank Temperaturueberwachung
    else if (strcmp("CabinetTemperatureOk", localName.c_str()) == 0)
    {
        m_oParseCabinetTemperatureOk = true;
    }
    // GeneralPurposeDigitalInputAckn
    else if (strcmp("GeneralPurposeDigitalInputAckn", localName.c_str()) == 0)
    {
        m_oParseSetGenPurposeDigInAckn = true;
    }
    // AcknProductNumberFull
    else if (strcmp("AcknProductNumberFull", localName.c_str()) == 0)
    {
        m_oParseAcknProductNumberFull = true;
    }
    // TriggerResultsReadyFull
    else if (strcmp("TriggerResultsReadyFull", localName.c_str()) == 0)
    {
        m_oParseTriggerResultsReadyFull = true;
    }
    // SetInspectionOKFull
    else if (strcmp("SetInspectionOKFull", localName.c_str()) == 0)
    {
        m_oParseSetInspectionOKFull = true;
    }
    // SetSumErrorLatchedFull
    else if (strcmp("SetSumErrorLatchedFull", localName.c_str()) == 0)
    {
        m_oParseSetSumErrorLatchedFull = true;
    }
    // SetQualityErrorFieldFull
    else if (strcmp("SetQualityErrorFieldFull", localName.c_str()) == 0)
    {
        m_oParseSetQualityErrorFieldFull = true;
    }
    // SetSystemReadyStatusFull
    else if (strcmp("SetSystemReadyStatusFull", localName.c_str()) == 0)
    {
        m_oParseSetSystemReadyStatusFull = true;
    }
    // SetCabinetTemperatureOkFull
    else if (strcmp("SetCabinetTemperatureOkFull", localName.c_str()) == 0)
    {
        m_oParseSetCabinetTemperatureOkFull = true;
    }
    // SetHMSignalsFieldFull
    else if (strcmp("SetHMSignalsFieldFull", localName.c_str()) == 0)
    {
        m_oParseSetHMSignalsFieldFull = true;
    }
    // SetSystemErrorFieldFull
    else if (strcmp("SetSystemErrorFieldFull", localName.c_str()) == 0)
    {
        m_oParseSetSystemErrorFieldFull = true;
    }
    // SetProductTypeFull, Spiegelung ProductTypeFull
    else if (strcmp("SetProductTypeFull", localName.c_str()) == 0)
    {
        m_oParseSetProductTypeFull = true;
    }
    // SetProductNumberFull, Spiegelung ProductNumberFull
    else if (strcmp("SetProductNumberFull", localName.c_str()) == 0)
    {
        m_oParseSetProductNumberFull = true;
    }

    // S6K_SystemFault
    else if (strcmp("S6K_SystemFault", localName.c_str()) == 0)
    {
        m_oParse_S6K_SystemFault = true;
    }
    // S6K_SystemReady
    else if (strcmp("S6K_SystemReady", localName.c_str()) == 0)
    {
        m_oParse_S6K_SystemReady = true;
    }
    // S6K_FastStop_DoubleBlank
    else if (strcmp("S6K_FastStop_DoubleBlank", localName.c_str()) == 0)
    {
        m_oParse_S6K_FastStop_DoubleBlank = true;
    }
    // S6K_SeamErrorCat1
    else if (strcmp("S6K_SeamErrorCat1", localName.c_str()) == 0)
    {
        m_oParse_S6K_SeamErrorCat1 = true;
    }
    // S6K_SeamErrorCat2
    else if (strcmp("S6K_SeamErrorCat2", localName.c_str()) == 0)
    {
        m_oParse_S6K_SeamErrorCat2 = true;
    }
    // S6K_QualityDataValid
    else if (strcmp("S6K_QualityDataValid", localName.c_str()) == 0)
    {
        m_oParse_S6K_QualityDataValid = true;
    }
    // S6K_AcknCycleData
    else if (strcmp("S6K_AcknCycleData", localName.c_str()) == 0)
    {
        m_oParse_S6K_AcknCycleData = true;
    }
    // S6K_CycleDataMirror
    else if (strcmp("S6K_CycleDataMirror", localName.c_str()) == 0)
    {
        m_oParse_S6K_CycleDataMirror = true;
    }
    // S6K_SeamNoMirror
    else if (strcmp("S6K_SeamNoMirror", localName.c_str()) == 0)
    {
        m_oParse_S6K_SeamNoMirror = true;
    }
    // S6K_AcknSeamNo
    else if (strcmp("S6K_AcknSeamNo", localName.c_str()) == 0)
    {
        m_oParse_S6K_AcknSeamNo = true;
    }
    // S6K_ResultDataValid
    else if (strcmp("S6K_ResultDataValid", localName.c_str()) == 0)
    {
        m_oParse_S6K_ResultDataValid = true;
    }
    // S6K_ResultDataCount
    else if (strcmp("S6K_ResultDataCount", localName.c_str()) == 0)
    {
        m_oParse_S6K_ResultDataCount = true;
    }
    // S6K_ResultsImage1
    else if (strcmp("S6K_ResultsImage1", localName.c_str()) == 0)
    {
        m_oParse_S6K_ResultsImage1 = true;
    }
    // S6K_ResultsImage2
    else if (strcmp("S6K_ResultsImage2", localName.c_str()) == 0)
    {
        m_oParse_S6K_ResultsImage2 = true;
    }
    // S6K_ResultsImage3
    else if (strcmp("S6K_ResultsImage3", localName.c_str()) == 0)
    {
        m_oParse_S6K_ResultsImage3 = true;
    }
    // S6K_ResultsImage4
    else if (strcmp("S6K_ResultsImage4", localName.c_str()) == 0)
    {
        m_oParse_S6K_ResultsImage4 = true;
    }
    // S6K_ResultsImage5
    else if (strcmp("S6K_ResultsImage5", localName.c_str()) == 0)
    {
        m_oParse_S6K_ResultsImage5 = true;
    }
    // S6K_ResultsImage6
    else if (strcmp("S6K_ResultsImage6", localName.c_str()) == 0)
    {
        m_oParse_S6K_ResultsImage6 = true;
    }
    // S6K_ResultsImage7
    else if (strcmp("S6K_ResultsImage7", localName.c_str()) == 0)
    {
        m_oParse_S6K_ResultsImage7 = true;
    }
    // S6K_ResultsImage8
    else if (strcmp("S6K_ResultsImage8", localName.c_str()) == 0)
    {
        m_oParse_S6K_ResultsImage8 = true;
    }
    // S6K_ResultsImage9
    else if (strcmp("S6K_ResultsImage9", localName.c_str()) == 0)
    {
        m_oParse_S6K_ResultsImage9 = true;
    }
    // S6K_ResultsImage10
    else if (strcmp("S6K_ResultsImage10", localName.c_str()) == 0)
    {
        m_oParse_S6K_ResultsImage10 = true;
    }
    // S6K_ResultsImage11
    else if (strcmp("S6K_ResultsImage11", localName.c_str()) == 0)
    {
        m_oParse_S6K_ResultsImage11 = true;
    }
    // S6K_ResultsImage12
    else if (strcmp("S6K_ResultsImage12", localName.c_str()) == 0)
    {
        m_oParse_S6K_ResultsImage12 = true;
    }
    // S6K_ResultsImage13
    else if (strcmp("S6K_ResultsImage13", localName.c_str()) == 0)
    {
        m_oParse_S6K_ResultsImage13 = true;
    }
    // S6K_ResultsImage14
    else if (strcmp("S6K_ResultsImage14", localName.c_str()) == 0)
    {
        m_oParse_S6K_ResultsImage14 = true;
    }
    // S6K_ResultsImage15
    else if (strcmp("S6K_ResultsImage15", localName.c_str()) == 0)
    {
        m_oParse_S6K_ResultsImage15 = true;
    }
    // S6K_ResultsImage16
    else if (strcmp("S6K_ResultsImage16", localName.c_str()) == 0)
    {
        m_oParse_S6K_ResultsImage16 = true;
    }
    // S6K_ResultsImage17
    else if (strcmp("S6K_ResultsImage17", localName.c_str()) == 0)
    {
        m_oParse_S6K_ResultsImage17 = true;
    }
    // S6K_ResultsImage18
    else if (strcmp("S6K_ResultsImage18", localName.c_str()) == 0)
    {
        m_oParse_S6K_ResultsImage18 = true;
    }
    // SM_ProcessingActive
    else if (strcmp("SM_ProcessingActive", localName.c_str()) == 0)
    {
        m_oParse_SM_ProcessingActive = true;
    }
    // SM_TakeoverStep
    else if (strcmp("SM_TakeoverStep", localName.c_str()) == 0)
    {
        m_oParse_SM_TakeoverStep = true;
    }
    // SM_StepField
    else if (strcmp("SM_StepField", localName.c_str()) == 0)
    {
        m_oParse_SM_StepField = true;
    }

    //########## HeadMonitor ######

    //HeadmonitorGateway
    else if (strcmp("HeadmonitorGateway", localName.c_str()) == 0)
    {
        m_oParseHeadmonitorGateway = true;
    }
    else if(strcmp("GlasNotPresent", localName.c_str()) == 0)
    {
        if ((strcmp("Bit", attributes.getLocalName(0).c_str()) == 0))
        {
            m_oBitGlasNotPresent = Tools::StringToInt(attributes.getValue(0));
        }
    }
    else if(strcmp("GlasDirty", localName.c_str()) == 0)
    {
        if ((strcmp("Bit", attributes.getLocalName(0).c_str()) == 0))
        {
            m_oBitGlasDirty = Tools::StringToInt(attributes.getValue(0));
        }
    }
    else if(strcmp("TempGlasFail", localName.c_str()) == 0)
    {
        if ((strcmp("Bit", attributes.getLocalName(0).c_str()) == 0))
        {
            m_oBitTempGlasFail= Tools::StringToInt(attributes.getValue(0));
        }
    }
    else if(strcmp("TempHeadFail", localName.c_str()) == 0)
    {
        if ((strcmp("Bit", attributes.getLocalName(0).c_str()) == 0))
        {
            m_oBitTempHeadFail = Tools::StringToInt(attributes.getValue(0));
        }
    }

    //########## General ######

    // ProductCode
    else if ((strcmp("ProductCode", localName.c_str()) == 0))
    {
        m_oGetProductCode = true;
        printFlag = false;
    }
    // VendorID
    else if (strcmp("VendorID", localName.c_str()) == 0)
    {
        m_oGetVendorID = true;
        printFlag = false;
    }
    // SlaveType
    else if (strcmp("SlaveType", localName.c_str()) == 0)
    {
        m_oGetSlaveType = true;
        printFlag = false;
    }
    // Instance
    else if ((strcmp("Instance", localName.c_str()) == 0))
    {
        m_oGetInstance = true;
        printFlag = false;
    }
    // StartBit
    else if ((strcmp("StartBit", localName.c_str()) == 0))
    {
        m_oGetStartBit = true;
        printFlag = false;
    }
    // TriggerLevel
    else if ((strcmp("TriggerLevel", localName.c_str()) == 0))
    {
        m_oGetTriggerLevel = true;
        printFlag = false;
    }
    // Length
    else if ((strcmp("Length", localName.c_str()) == 0))
    {
        m_oGetLength = true;
        printFlag = false;
    }
    // Output
    else if (strcmp("Output", localName.c_str()) == 0)
    {
        printFlag = false;
    }
    // Input
    else if (strcmp("Input", localName.c_str()) == 0)
    {
        printFlag = false;
    }

    //#############################################################

    if (printFlag) std::cout << "StartElement: " << localName << std::endl;
}

void SAX_VIConfigParser::endElement(const XMLString& uri, const XMLString& localName, const XMLString& qname)
{
    bool printFlag = true;

    //########## Inputs ######

    if ((strcmp("TriggerContinuouslyMode", localName.c_str()) == 0))
    {
        m_oParseGetTriggerContinuouslyMode = false;
        ClearTempValues();
    }
    else if ((strcmp("TriggerAutomaticMode", localName.c_str()) == 0))
    {
        m_oParseGetTriggerAutomaticMode = false;
        ClearTempValues();
    }
    else if ((strcmp("TriggerInspectionInfo", localName.c_str()) == 0))
    {
        m_oParseGetTriggerInspectionInfo = false;
        ClearTempValues();
    }
    else if ((strcmp("TriggerInspectionPreStart", localName.c_str()) == 0))
    {
        m_oParseGetTriggerInspectionPreStart = false;
        ClearTempValues();
    }
    else if ((strcmp("TriggerInspectionStartEnd", localName.c_str()) == 0))
    {
        m_oParseGetTriggerInspectionStartEnd = false;
        ClearTempValues();
    }
    else if ((strcmp("ChangeToStandardMode", localName.c_str()) == 0))
    {
        m_oParseGetChangeToStandardMode = false;
        ClearTempValues();
    }
    else if ((strcmp("TriggerUnblockLineLaser", localName.c_str()) == 0))
    {
        m_oParseGetTriggerUnblockLineLaser = false;
        ClearTempValues();
    }
    else if ((strcmp("Seamseries", localName.c_str()) == 0))
    {
        m_oParseGetSeamSeries = false;
        ClearTempValues();
    }
    else if ((strcmp("SeamNr", localName.c_str()) == 0))
    {
        m_oParseGetSeamNr = false;
        ClearTempValues();
    }
    else if ((strcmp("TriggerQuitSystemFault", localName.c_str()) == 0))
    {
        m_oParseGetTriggerQuitSystemFault = false;
        ClearTempValues();
    }
    else if ((strcmp("CalibrationType", localName.c_str()) == 0))
    {
        m_oParseGetCalibrationType = false;
        ClearTempValues();
    }
    else if ((strcmp("TriggerCalibrationMode", localName.c_str()) == 0))
    {
        m_oParseGetTriggerCalibrationMode = false;
        ClearTempValues();
    }
    else if ((strcmp("TriggerHomingYAxis", localName.c_str()) == 0))
    {
        m_oParseGetTriggerHomingYAxis = false;
        ClearTempValues();
    }
    else if ((strcmp("InvertedTriggerEmergencyStop", localName.c_str()) == 0))
    {
        m_oParseGetInvertedTriggerEmergencyStop = false;
        ClearTempValues();
    }
    else if ((strcmp("ProductType", localName.c_str()) == 0))
    {
        m_oParseGetProductType = false;
        ClearTempValues();
    }
    else if ((strcmp("ProductNumber", localName.c_str()) == 0))
    {
        m_oParseGetProductNumber = false;
        ClearTempValues();
    }
    else if ((strcmp("ExtendedProductInfo_UTF8", localName.c_str()) == 0))
    {
        m_oParseGetExtendedProductInfo = false;
        ClearTempValues();
    }
    else if ((strcmp("GeneralPurposeDigitalInput1", localName.c_str()) == 0))
    {
        m_oParseGenPurposeDigIn1 = false;
        ClearTempValues();
    }
    else if ((strcmp("GeneralPurposeDigitalInputAddress", localName.c_str()) == 0))
    {
        m_oParseGenPurposeDigInAddress = false;
        ClearTempValues();
    }
    else if ((strcmp("GeneralPurposeDigitalInputTakeOver", localName.c_str()) == 0))
    {
        m_oParseGenPurposeDigInTakeOver = false;
        ClearTempValues();
    }
    else if ((strcmp("GeneralPurposeDigitalOutputTakeOver", localName.c_str()) == 0))
    {
        m_oParseGenPurposeDigOutTakeOver = false;
        ClearTempValues();
    }
    else if ((strcmp("TriggerProductNumberFull", localName.c_str()) == 0))
    {
        m_oParseTriggerProductNumberFull = false;
        ClearTempValues();
    }
    else if ((strcmp("AcknResultsReadyFull", localName.c_str()) == 0))
    {
        m_oParseAcknResultsReadyFull = false;
        ClearTempValues();
    }
    else if ((strcmp("TriggerQuitSystemFaultFull", localName.c_str()) == 0))
    {
        m_oParseTriggerQuitSystemFaultFull = false;
        ClearTempValues();
    }
    else if ((strcmp("GetProductTypeFull", localName.c_str()) == 0))
    {
        m_oParseGetProductTypeFull = false;
        ClearTempValues();
    }
    else if ((strcmp("GetProductNumberFull", localName.c_str()) == 0))
    {
        m_oParseGetProductNumberFull = false;
        ClearTempValues();
    }

    else if ((strcmp("S6K_SouvisActive", localName.c_str()) == 0))
    {
        m_oParse_S6K_SouvisActive = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_SouvisInspection", localName.c_str()) == 0))
    {
        m_oParse_S6K_SouvisInspection = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_QuitSystemFault", localName.c_str()) == 0))
    {
        m_oParse_S6K_QuitSystemFault = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_MachineReady", localName.c_str()) == 0))
    {
        m_oParse_S6K_MachineReady = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_ProductNumber", localName.c_str()) == 0))
    {
        m_oParse_S6K_ProductNumber = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_AcknQualityData", localName.c_str()) == 0))
    {
        m_oParse_S6K_AcknQualityData = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_CycleDataValid", localName.c_str()) == 0))
    {
        m_oParse_S6K_CycleDataValid = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_CycleData", localName.c_str()) == 0))
    {
        m_oParse_S6K_CycleData = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_SeamNo", localName.c_str()) == 0))
    {
        m_oParse_S6K_SeamNo = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_SeamNoValid", localName.c_str()) == 0))
    {
        m_oParse_S6K_SeamNoValid = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_AcknResultData", localName.c_str()) == 0))
    {
        m_oParse_S6K_AcknResultData = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_MakePictures", localName.c_str()) == 0))
    {
        m_oParse_S6K_MakePictures = false;
        ClearTempValues();
    }
    else if ((strcmp("SM_AcknowledgeStep", localName.c_str()) == 0))
    {
        m_oParse_SM_AcknowledgeStep = false;
        ClearTempValues();
    }

    //########## Outputs ######

    else if ((strcmp("InspectCycleAckn", localName.c_str()) == 0))
    {
        m_oParseSetInspectCycleAckn = false;
        ClearTempValues();
    }
    else if ((strcmp("InspectionPreStartAckn", localName.c_str()) == 0))
    {
        m_oParseSetInspectionPreStartAckn = false;
        ClearTempValues();
    }
    else if ((strcmp("InspectionStartEndAckn", localName.c_str()) == 0))
    {
        m_oParseSetInspectionStartEndAckn = false;
        ClearTempValues();
    }
    else if ((strcmp("InspectionOK", localName.c_str()) == 0))
    {
        m_oParseSetInspectionOK = false;
        ClearTempValues();
    }
    else if ((strcmp("SumErrorLatched", localName.c_str()) == 0))
    {
        m_oParseSetSumErrorLatched = false;
        ClearTempValues();
    }
    else if ((strcmp("InspectionIncomplete", localName.c_str()) == 0))
    {
        m_oParseSetInspectionIncomplete = false;
        ClearTempValues();
    }
    else if ((strcmp("QualityErrorField", localName.c_str()) == 0))
    {
        m_oParseSetQualityErrorField = false;
        ClearTempValues();
    }
    else if ((strcmp("SumErrorSeam", localName.c_str()) == 0))
    {
        m_oParseSetSumErrorSeam = false;
        ClearTempValues();
    }
    else if ((strcmp("SumErrorSeamSeries", localName.c_str()) == 0))
    {
        m_oParseSetSumErrorSeamSeries = false;
        ClearTempValues();
    }
    else if ((strcmp("SystemReadyStatus", localName.c_str()) == 0))
    {
        m_oParseSetSystemReadyStatus = false;
        ClearTempValues();
    }
    else if ((strcmp("SystemErrorField", localName.c_str()) == 0))
    {
        m_oParseSetSystemErrorField = false;
        ClearTempValues();
    }
    else if ((strcmp("CalibResultsField", localName.c_str()) == 0))
    {
        m_oParseSetCalibResultsField = false;
        ClearTempValues();
    }
    else if ((strcmp("PositionResultsField", localName.c_str()) == 0))
    {
        m_oParseSetPositionResultsField = false;
        ClearTempValues();
    }
    else if ((strcmp("ProductTypeMirror", localName.c_str()) == 0))
    {
        m_oParseSetProductType = false;
        ClearTempValues();
    }
    else if ((strcmp("ProductNumberMirror", localName.c_str()) == 0))
    {
        m_oParseSetProductNumber = false;
        ClearTempValues();
    }
    else if ((strcmp("ExtendedProductInfoMirror_UTF8", localName.c_str()) == 0))
    {
        m_oParseSetExtendedProductInfo = false;
        ClearTempValues();
    }
    else if ((strcmp("CabinetTemperatureOk", localName.c_str()) == 0))
    {
        m_oParseCabinetTemperatureOk = false;
        ClearTempValues();
    }
    else if ((strcmp("GeneralPurposeDigitalInputAckn", localName.c_str()) == 0))
    {
        m_oParseSetGenPurposeDigInAckn = false;
        ClearTempValues();
    }
    else if ((strcmp("AcknProductNumberFull", localName.c_str()) == 0))
    {
        m_oParseAcknProductNumberFull = false;
        ClearTempValues();
    }
    else if ((strcmp("TriggerResultsReadyFull", localName.c_str()) == 0))
    {
        m_oParseTriggerResultsReadyFull = false;
        ClearTempValues();
    }
    else if ((strcmp("SetInspectionOKFull", localName.c_str()) == 0))
    {
        m_oParseSetInspectionOKFull = false;
        ClearTempValues();
    }
    else if ((strcmp("SetSumErrorLatchedFull", localName.c_str()) == 0))
    {
        m_oParseSetSumErrorLatchedFull = false;
        ClearTempValues();
    }
    else if ((strcmp("SetQualityErrorFieldFull", localName.c_str()) == 0))
    {
        m_oParseSetQualityErrorFieldFull = false;
        ClearTempValues();
    }
    else if ((strcmp("SetSystemReadyStatusFull", localName.c_str()) == 0))
    {
        m_oParseSetSystemReadyStatusFull = false;
        ClearTempValues();
    }
    else if ((strcmp("SetCabinetTemperatureOkFull", localName.c_str()) == 0))
    {
        m_oParseSetCabinetTemperatureOkFull = false;
        ClearTempValues();
    }
    else if ((strcmp("SetHMSignalsFieldFull", localName.c_str()) == 0))
    {
        m_oParseSetHMSignalsFieldFull = false;
        ClearTempValues();
    }
    else if ((strcmp("SetSystemErrorFieldFull", localName.c_str()) == 0))
    {
        m_oParseSetSystemErrorFieldFull = false;
        ClearTempValues();
    }
    else if ((strcmp("SetProductTypeFull", localName.c_str()) == 0))
    {
        m_oParseSetProductTypeFull = false;
        ClearTempValues();
    }
    else if ((strcmp("SetProductNumberFull", localName.c_str()) == 0))
    {
        m_oParseSetProductNumberFull = false;
        ClearTempValues();
    }

    else if ((strcmp("S6K_SystemFault", localName.c_str()) == 0))
    {
        m_oParse_S6K_SystemFault = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_SystemReady", localName.c_str()) == 0))
    {
        m_oParse_S6K_SystemReady = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_FastStop_DoubleBlank", localName.c_str()) == 0))
    {
        m_oParse_S6K_FastStop_DoubleBlank = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_SeamErrorCat1", localName.c_str()) == 0))
    {
        m_oParse_S6K_SeamErrorCat1 = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_SeamErrorCat2", localName.c_str()) == 0))
    {
        m_oParse_S6K_SeamErrorCat2 = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_QualityDataValid", localName.c_str()) == 0))
    {
        m_oParse_S6K_QualityDataValid = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_AcknCycleData", localName.c_str()) == 0))
    {
        m_oParse_S6K_AcknCycleData = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_CycleDataMirror", localName.c_str()) == 0))
    {
        m_oParse_S6K_CycleDataMirror = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_SeamNoMirror", localName.c_str()) == 0))
    {
        m_oParse_S6K_SeamNoMirror = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_AcknSeamNo", localName.c_str()) == 0))
    {
        m_oParse_S6K_AcknSeamNo = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_ResultDataValid", localName.c_str()) == 0))
    {
        m_oParse_S6K_ResultDataValid = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_ResultDataCount", localName.c_str()) == 0))
    {
        m_oParse_S6K_ResultDataCount = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_ResultsImage1", localName.c_str()) == 0))
    {
        m_oParse_S6K_ResultsImage1 = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_ResultsImage2", localName.c_str()) == 0))
    {
        m_oParse_S6K_ResultsImage2 = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_ResultsImage3", localName.c_str()) == 0))
    {
        m_oParse_S6K_ResultsImage3 = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_ResultsImage4", localName.c_str()) == 0))
    {
        m_oParse_S6K_ResultsImage4 = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_ResultsImage5", localName.c_str()) == 0))
    {
        m_oParse_S6K_ResultsImage5 = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_ResultsImage6", localName.c_str()) == 0))
    {
        m_oParse_S6K_ResultsImage6 = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_ResultsImage7", localName.c_str()) == 0))
    {
        m_oParse_S6K_ResultsImage7 = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_ResultsImage8", localName.c_str()) == 0))
    {
        m_oParse_S6K_ResultsImage8 = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_ResultsImage9", localName.c_str()) == 0))
    {
        m_oParse_S6K_ResultsImage9 = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_ResultsImage10", localName.c_str()) == 0))
    {
        m_oParse_S6K_ResultsImage10 = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_ResultsImage11", localName.c_str()) == 0))
    {
        m_oParse_S6K_ResultsImage11 = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_ResultsImage12", localName.c_str()) == 0))
    {
        m_oParse_S6K_ResultsImage12 = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_ResultsImage13", localName.c_str()) == 0))
    {
        m_oParse_S6K_ResultsImage13 = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_ResultsImage14", localName.c_str()) == 0))
    {
        m_oParse_S6K_ResultsImage14 = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_ResultsImage15", localName.c_str()) == 0))
    {
        m_oParse_S6K_ResultsImage15 = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_ResultsImage16", localName.c_str()) == 0))
    {
        m_oParse_S6K_ResultsImage16 = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_ResultsImage17", localName.c_str()) == 0))
    {
        m_oParse_S6K_ResultsImage17 = false;
        ClearTempValues();
    }
    else if ((strcmp("S6K_ResultsImage18", localName.c_str()) == 0))
    {
        m_oParse_S6K_ResultsImage18 = false;
        ClearTempValues();
    }
    else if ((strcmp("SM_ProcessingActive", localName.c_str()) == 0))
    {
        m_oParse_SM_ProcessingActive = false;
        ClearTempValues();
    }
    else if ((strcmp("SM_TakeoverStep", localName.c_str()) == 0))
    {
        m_oParse_SM_TakeoverStep = false;
        ClearTempValues();
    }
    else if ((strcmp("SM_StepField", localName.c_str()) == 0))
    {
        m_oParse_SM_StepField = false;
        ClearTempValues();
    }

    //########## HeadMonitor ######

    else if ( (strcmp("HM_Input", localName.c_str()) == 0))
    {
        if ((m_oTempSlaveType == 0) ||
            (m_oTempProductCode == 0) ||
            (m_oTempVendorID == 0) ||
            (m_oTempInstance == 0))
        {
            wmLogTr(eError, "QnxMsg.VI.VIConfErr1", "Invalid signal data of <%s> in VI_Config.xml, please correct !\n", (char *)"HM_Input");
            wmLogTr(eError, "QnxMsg.VI.VIConfErr2", "Signal <%s> is not activated!\n", (char *)"HM_Input");
            ClearTempValues();
        }
        else
        {
            m_HMinGatewayInfo.nSlaveType = m_oTempSlaveType;
            m_HMinGatewayInfo.nProductCode = m_oTempProductCode;
            m_HMinGatewayInfo.nVendorID = m_oTempVendorID;
            m_HMinGatewayInfo.nInstance = m_oTempInstance;

            m_HMinGatewayInfo.bitGlasNotPresent = m_oBitGlasNotPresent;
            m_HMinGatewayInfo.bitGlasDirty = m_oBitGlasDirty;
            m_HMinGatewayInfo.bitTempGlasFail = m_oBitTempGlasFail;
            m_HMinGatewayInfo.bitTempHeadFail = m_oBitTempHeadFail;
            m_HMinGatewayInfo.isValid = true;
            ClearTempValues();
        }
    }
    else if ( (strcmp("HM_Output", localName.c_str()) == 0))
    {
        if ((m_oTempSlaveType == 0) ||
            (m_oTempProductCode == 0) ||
            (m_oTempVendorID == 0) ||
            (m_oTempInstance == 0))
        {
            wmLogTr(eError, "QnxMsg.VI.VIConfErr1", "Invalid signal data of <%s> in VI_Config.xml, please correct !\n", (char *)"HM_Output");
            wmLogTr(eError, "QnxMsg.VI.VIConfErr2", "Signal <%s> is not activated!\n", (char *)"HM_Output");
            ClearTempValues();
        }
        else
        {
            m_HMoutGatewayInfo.nSlaveType = m_oTempSlaveType;
            m_HMoutGatewayInfo.nProductCode = m_oTempProductCode;
            m_HMoutGatewayInfo.nVendorID = m_oTempVendorID;
            m_HMoutGatewayInfo.nInstance = m_oTempInstance;

            m_HMoutGatewayInfo.bitGlasNotPresent = m_oBitGlasNotPresent;
            m_HMoutGatewayInfo.bitGlasDirty = m_oBitGlasDirty;
            m_HMoutGatewayInfo.bitTempGlasFail = m_oBitTempGlasFail;
            m_HMoutGatewayInfo.bitTempHeadFail = m_oBitTempHeadFail;
            m_HMoutGatewayInfo.isValid = true;
            ClearTempValues();
        }
    }
    else if((strcmp("HeadmonitorGateway", localName.c_str()) == 0))
    {
        m_oParseHeadmonitorGateway = false;
    }

    //########## General ######

    else if (strcmp("ProductCode", localName.c_str()) == 0)
    {
        m_oTempProductCode = Tools::StringToInt(m_oTempParseString);
        m_oTempParseString.clear();
        m_oGetProductCode = false;
        printFlag = false;
    }
    else if (strcmp("VendorID", localName.c_str()) == 0)
    {
        m_oTempVendorID = Tools::StringToInt(m_oTempParseString);
        m_oTempParseString.clear();
        m_oGetVendorID = false;
        printFlag = false;
    }
    else if (strcmp("SlaveType", localName.c_str()) == 0)
    {
        m_oTempSlaveType = Tools::StringToInt(m_oTempParseString);
        m_oTempParseString.clear();
        m_oGetSlaveType = false;
        printFlag = false;
    }
    else if (strcmp("Instance", localName.c_str()) == 0)
    {
        m_oTempInstance = Tools::StringToInt(m_oTempParseString);
        m_oTempParseString.clear();
        m_oGetInstance = false;
        printFlag = false;
    }
    else if (strcmp("StartBit", localName.c_str()) == 0)
    {
        m_oTempStartBit = Tools::StringToInt(m_oTempParseString);
        m_oTempParseString.clear();
        m_oGetStartBit = false;
        printFlag = false;
    }
    else if (strcmp("TriggerLevel", localName.c_str()) == 0)
    {
        m_oTempTriggerLevel = Tools::StringToInt(m_oTempParseString);
        m_oTempParseString.clear();
        m_oGetTriggerLevel = false;
        printFlag = false;
    }
    else if (strcmp("Length", localName.c_str()) == 0)
    {
        m_oTempLength = Tools::StringToInt(m_oTempParseString);
        m_oTempParseString.clear();
        m_oGetLength = false;
        printFlag = false;
    }

    else if (strcmp("Output", localName.c_str()) == 0)
    {
        //write mapped slave informations into according list

        if(m_oParseSetInspectCycleAckn)
        {
            insertInOutCommandList(AInspectCycleAckn, "InspectCycleAckn", 1);
        }
        else if(m_oParseSetInspectionPreStartAckn)
        {
            insertInOutCommandList(AInspectionPreStartAckn, "InspectionPreStartAckn", 1);
        }
        else if(m_oParseSetInspectionStartEndAckn)
        {
            insertInOutCommandList(AInspectionStartEndAckn, "InspectionStartEndAckn", 1);
        }
        else if(m_oParseSetInspectionOK)
        {
            insertInOutCommandList(AInspectionOK, "InspectionOK", 1);
        }
        else if(m_oParseSetSumErrorLatched)
        {
            insertInOutCommandList(ASumErrorLatched, "SumErrorLatched", 1);
        }
        else if(m_oParseSetInspectionIncomplete)
        {
            insertInOutCommandList(AInspectionIncomplete, "InspectionIncomplete", 1);
        }
        else if(m_oParseSetQualityErrorField)
        {
            insertInOutCommandList(AQualityErrorField, "QualityErrorField", m_oTempLength);
        }
        else if(m_oParseSetSumErrorSeam)
        {
            insertInOutCommandList(ASumErrorSeam, "SumErrorSeam", 1);
        }
        else if(m_oParseSetSumErrorSeamSeries)
        {
            insertInOutCommandList(ASumErrorSeamSeries, "SumErrorSeamSeries", 1);
        }
        else if(m_oParseSetSystemReadyStatus)
        {
            insertInOutCommandList(ASystemReady, "SystemReadyStatus", 1);
        }
        else if(m_oParseSetSystemErrorField)
        {
            insertInOutCommandList(ASystemErrorField, "SystemErrorField", m_oTempLength);
        }
        else if(m_oParseSetCalibResultsField)
        {
            insertInOutCommandList(ACalibResultsField, "CalibResultsField", m_oTempLength);
        }
        else if(m_oParseSetPositionResultsField)
        {
            insertInOutCommandList(APositionResultsField, "PositionResultsField", m_oTempLength);
        }
        else if(m_oParseSetProductType)
        {
            insertInOutCommandList(AProductType, "ProductTypeMirror", m_oTempLength);
        }
        else if(m_oParseSetProductNumber)
        {
            insertInOutCommandList(AProductNumber, "ProductNumberMirror", m_oTempLength);
        }
        else if(m_oParseSetExtendedProductInfo)
        {
            insertInOutCommandList(AExtendedProductInfo, "ExtendedProductInfoMirror_UTF8", m_oTempLength);
        }
        else if(m_oParseCabinetTemperatureOk)
        {
            insertInOutCommandList(ACabinetTemperatureOk, "CabinetTemperatureOk", 1);
        }
        else if(m_oParseSetGenPurposeDigInAckn)
        {
            insertInOutCommandList(AGenPurposeDigInAckn, "GenPurposeDigitalInputAckn", 1);
        }
        else if(m_oParseAcknProductNumberFull)
        {
            insertInOutCommandList(AAcknProductNumberFull, "AcknProductNumberFull", 1);
        }
        else if (m_oParseTriggerResultsReadyFull)
        {
            insertInOutCommandList(ATriggerResultsReadyFull, "TriggerResultsReadyFull", 1);
        }
        else if (m_oParseSetInspectionOKFull)
        {
            insertInOutCommandList(ASetInspectionOKFull, "SetInspectionOKFull", 1);
        }
        else if (m_oParseSetSumErrorLatchedFull)
        {
            insertInOutCommandList(ASetSumErrorLatchedFull, "SetSumErrorLatchedFull", 1);
        }
        else if(m_oParseSetQualityErrorFieldFull)
        {
            insertInOutCommandList(ASetQualityErrorFieldFull, "SetQualityErrorFieldFull", m_oTempLength);
        }
        else if(m_oParseSetSystemReadyStatusFull)
        {
            insertInOutCommandList(ASetSystemReadyStatusFull, "SetSystemReadyStatusFull", m_oTempLength);
        }
        else if(m_oParseSetCabinetTemperatureOkFull)
        {
            insertInOutCommandList(ASetCabinetTemperatureOkFull, "SetCabinetTemperatureOkFull", m_oTempLength);
        }
        else if(m_oParseSetHMSignalsFieldFull)
        {
            insertInOutCommandList(ASetHMSignalsFieldFull, "SetHMSignalsFieldFull", m_oTempLength);
        }
        else if(m_oParseSetSystemErrorFieldFull)
        {
            insertInOutCommandList(ASetSystemErrorFieldFull, "SetSystemErrorFieldFull", m_oTempLength);
        }
        else if(m_oParseSetProductTypeFull)
        {
            insertInOutCommandList(ASetProductTypeFull, "SetProductTypeFull", m_oTempLength);
        }
        else if (m_oParseSetProductNumberFull)
        {
            insertInOutCommandList(ASetProductNumberFull, "SetProductNumberFull", m_oTempLength);
        }

        else if (m_oParse_S6K_SystemFault)
        {
            insertInOutCommandList(A_S6K_SystemFault, "S6K_SystemFault", 1);
        }
        else if (m_oParse_S6K_SystemReady)
        {
            insertInOutCommandList(A_S6K_SystemReady, "S6K_SystemReady", 1);
        }
        else if (m_oParse_S6K_FastStop_DoubleBlank)
        {
            insertInOutCommandList(A_S6K_FastStopDoubleBlank, "S6K_FastStop_DoubleBlank", 1);
        }
        else if (m_oParse_S6K_SeamErrorCat1)
        {
            insertInOutCommandList(A_S6K_SeamErrorCat1, "S6K_SeamErrorCat1", m_oTempLength);
        }
        else if (m_oParse_S6K_SeamErrorCat2)
        {
            insertInOutCommandList(A_S6K_SeamErrorCat2, "S6K_SeamErrorCat2", m_oTempLength);
        }
        else if (m_oParse_S6K_QualityDataValid)
        {
            insertInOutCommandList(A_S6K_QualityDataValid, "S6K_QualityDataValid", 1);
        }
        else if (m_oParse_S6K_AcknCycleData)
        {
            insertInOutCommandList(A_S6K_AcknCycleData, "S6K_AcknCycleData", 1);
        }
        else if (m_oParse_S6K_CycleDataMirror)
        {
            insertInOutCommandList(A_S6K_CycleDataMirror, "S6K_CycleDataMirror", m_oTempLength);
        }
        else if (m_oParse_S6K_SeamNoMirror)
        {
            insertInOutCommandList(A_S6K_SeamNoMirror, "S6K_SeamNoMirror", m_oTempLength);
        }
        else if (m_oParse_S6K_AcknSeamNo)
        {
            insertInOutCommandList(A_S6K_AcknSeamNo, "S6K_AcknSeamNo", 1);
        }
        else if (m_oParse_S6K_ResultDataValid)
        {
            insertInOutCommandList(A_S6K_ResultDataValid, "S6K_ResultDataValid", 1);
        }
        else if (m_oParse_S6K_ResultDataCount)
        {
            insertInOutCommandList(A_S6K_ResultDataCount, "S6K_ResultDataCount", m_oTempLength);
        }
        else if (m_oParse_S6K_ResultsImage1)
        {
            insertInOutCommandList(A_S6K_ResultsImage1, "S6K_ResultsImage1", m_oTempLength);
        }
        else if (m_oParse_S6K_ResultsImage2)
        {
            insertInOutCommandList(A_S6K_ResultsImage2, "S6K_ResultsImage2", m_oTempLength);
        }
        else if (m_oParse_S6K_ResultsImage3)
        {
            insertInOutCommandList(A_S6K_ResultsImage3, "S6K_ResultsImage3", m_oTempLength);
        }
        else if (m_oParse_S6K_ResultsImage4)
        {
            insertInOutCommandList(A_S6K_ResultsImage4, "S6K_ResultsImage4", m_oTempLength);
        }
        else if (m_oParse_S6K_ResultsImage5)
        {
            insertInOutCommandList(A_S6K_ResultsImage5, "S6K_ResultsImage5", m_oTempLength);
        }
        else if (m_oParse_S6K_ResultsImage6)
        {
            insertInOutCommandList(A_S6K_ResultsImage6, "S6K_ResultsImage6", m_oTempLength);
        }
        else if (m_oParse_S6K_ResultsImage7)
        {
            insertInOutCommandList(A_S6K_ResultsImage7, "S6K_ResultsImage7", m_oTempLength);
        }
        else if (m_oParse_S6K_ResultsImage8)
        {
            insertInOutCommandList(A_S6K_ResultsImage8, "S6K_ResultsImage8", m_oTempLength);
        }
        else if (m_oParse_S6K_ResultsImage9)
        {
            insertInOutCommandList(A_S6K_ResultsImage9, "S6K_ResultsImage9", m_oTempLength);
        }
        else if (m_oParse_S6K_ResultsImage10)
        {
            insertInOutCommandList(A_S6K_ResultsImage10, "S6K_ResultsImage10", m_oTempLength);
        }
        else if (m_oParse_S6K_ResultsImage11)
        {
            insertInOutCommandList(A_S6K_ResultsImage11, "S6K_ResultsImage11", m_oTempLength);
        }
        else if (m_oParse_S6K_ResultsImage12)
        {
            insertInOutCommandList(A_S6K_ResultsImage12, "S6K_ResultsImage12", m_oTempLength);
        }
        else if (m_oParse_S6K_ResultsImage13)
        {
            insertInOutCommandList(A_S6K_ResultsImage13, "S6K_ResultsImage13", m_oTempLength);
        }
        else if (m_oParse_S6K_ResultsImage14)
        {
            insertInOutCommandList(A_S6K_ResultsImage14, "S6K_ResultsImage14", m_oTempLength);
        }
        else if (m_oParse_S6K_ResultsImage15)
        {
            insertInOutCommandList(A_S6K_ResultsImage15, "S6K_ResultsImage15", m_oTempLength);
        }
        else if (m_oParse_S6K_ResultsImage16)
        {
            insertInOutCommandList(A_S6K_ResultsImage16, "S6K_ResultsImage16", m_oTempLength);
        }
        else if (m_oParse_S6K_ResultsImage17)
        {
            insertInOutCommandList(A_S6K_ResultsImage17, "S6K_ResultsImage17", m_oTempLength);
        }
        else if (m_oParse_S6K_ResultsImage18)
        {
            insertInOutCommandList(A_S6K_ResultsImage18, "S6K_ResultsImage18", m_oTempLength);
        }
        else if (m_oParse_SM_ProcessingActive)
        {
            insertInOutCommandList(A_SM_ProcessingActive, "SM_ProcessingActive", 1);
        }
        else if (m_oParse_SM_TakeoverStep)
        {
            insertInOutCommandList(A_SM_TakeoverStep, "SM_TakeoverStep", 1);
        }
        else if (m_oParse_SM_StepField)
        {
            insertInOutCommandList(A_SM_StepField, "SM_StepField", m_oTempLength);
        }

        printFlag = false;

    } else if (strcmp("Input", localName.c_str()) == 0)
    {
        //write mapped slave informations into according list

        if (m_oParseGetTriggerContinuouslyMode)
        {
            insertInInCommandList(ETriggerStartStopContinuously, "TriggerContinuouslyMode", 1);
        }
        else if (m_oParseGetTriggerAutomaticMode)
        {
            if (m_oTempVendorID == VENDORID_BECKHOFF)
            {
                if ((m_oTempProductCode == PRODUCTCODE_EL3102) ||
                    (m_oTempProductCode == PRODUCTCODE_EL3702))
                {
                    m_oIsStartCycleAnalogInput = true;
                }
            }
            insertInInCommandList(ETriggerStartStopAutomatic, "TriggerAutomaticMode", 1);
        }
        else if (m_oParseGetTriggerInspectionInfo)
        {
            insertInInCommandList(ETriggerInspectionInfo, "TriggerInspectionInfo", 1);
        }
        else if (m_oParseGetTriggerInspectionPreStart)
        {
            insertInInCommandList(ETriggerInspectionPreStart, "TriggerInspectionPreStart", 1);
        }
        else if (m_oParseGetTriggerInspectionStartEnd)
        {
            if (m_oTempVendorID == VENDORID_BECKHOFF)
            {
                if ((m_oTempProductCode == PRODUCTCODE_EL3102) ||
                    (m_oTempProductCode == PRODUCTCODE_EL3702))
                {
                    m_oIsStartSeamAnalogInput = true;
                }
            }
            insertInInCommandList(ETriggerInspectionStartEnd, "TriggerInspectionStartEnd", 1);
        }
        else if (m_oParseGetChangeToStandardMode)
        {
            insertInInCommandList(EChangeToStandardMode, "ChangeToStandardMode", 1);
        }
        else if (m_oParseGetTriggerUnblockLineLaser)
        {
            insertInInCommandList(ETriggerUnblockLineLaser, "TriggerUnblockLineLaser", 1);
        }
        else if (m_oParseGetSeamSeries)
        {
            insertInInCommandList(ESeamseriesNr, "Seamseries", m_oTempLength);
        }
        else if (m_oParseGetSeamNr)
        {
            insertInInCommandList(ESeamNr, "SeamNr", m_oTempLength);
        }
        else if (m_oParseGetTriggerQuitSystemFault)
        {
            insertInInCommandList(ETriggerQuitSystemFault, "TriggerQuitSystemFault", 1);
        }
        else if (m_oParseGetCalibrationType)
        {
            insertInInCommandList(ECalibrationType, "CalibrationType", m_oTempLength);
        }
        else if (m_oParseGetTriggerCalibrationMode)
        {
            insertInInCommandList(ETriggerStartStopCalibration, "TriggerCalibrationMode", 1);
        }
        else if (m_oParseGetTriggerHomingYAxis)
        {
            insertInInCommandList(ETriggerHomingYAxis, "TriggerHomingYAxis", 1);
        }
        else if (m_oParseGetProductType)
        {
            insertInInCommandList(EProductType, "ProductType", m_oTempLength);
        }
        else if (m_oParseGetProductNumber)
        {
            m_oProductNumberExtern = true;
            insertInInCommandList(EProductNumber, "ProductNumber", m_oTempLength);
        }
        else if (m_oParseGetExtendedProductInfo)
        {
            m_oProductNumberExtern = true;
            insertInInCommandList(EExtendedProductInfo, "ExtendedProductInfo_UTF8", m_oTempLength);
        }
        else if (m_oParseGenPurposeDigIn1)
        {
            insertInInCommandList(EGenPurposeDigIn1, "GeneralPurposeDigitalInput1", m_oTempLength);
        }
        else if (m_oParseGenPurposeDigInAddress)
        {
            insertInInCommandList(EGenPurposeDigInAddress, "GeneralPurposeDigitalInputAddress", m_oTempLength);
        }
        else if (m_oParseGenPurposeDigInTakeOver)
        {
            insertInInCommandList(EGenPurposeDigInTakeOver, "GeneralPurposeDigitalInputTakeOver", m_oTempLength);
        }
        else if (m_oParseGenPurposeDigOutTakeOver)
        {
            insertInInCommandList(EGenPurposeDigOutTakeOver, "GeneralPurposeDigitalOutputTakeOver", m_oTempLength);
        }
        else if (m_oParseGetInvertedTriggerEmergencyStop)
        {
            insertInInCommandList(EInvertedTriggerEmergencyStop, "InvertedTriggerEmergencyStop", 1);
        }
        else if(m_oParseCabinetTemperatureOk)
        {
            insertInInCommandList(ECabinetTemperatureOk, "CabinetTemperatureOk", 1);
        }
        else if (m_oParseTriggerProductNumberFull)
        {
            m_oProductNumberExtern = true;
            insertInInCommandList(ETriggerProductNumberFull, "TriggerProductNumberFull", m_oTempLength);
        }
        else if (m_oParseAcknResultsReadyFull)
        {
            insertInInCommandList(EAcknResultsReadyFull, "AcknResultsReadyFull", m_oTempLength);
        }
        else if (m_oParseTriggerQuitSystemFaultFull)
        {
            insertInInCommandList(ETriggerQuitSystemFaultFull, "TriggerQuitSystemFaultFull", m_oTempLength);
        }
        else if (m_oParseGetProductTypeFull)
        {
            insertInInCommandList(EGetProductTypeFull, "GetProductTypeFull", m_oTempLength);
        }
        else if (m_oParseGetProductNumberFull)
        {
            m_oProductNumberExtern = true;
            insertInInCommandList(EGetProductNumberFull, "GetProductNumberFull", m_oTempLength);
        }

        else if (m_oParse_S6K_SouvisActive)
        {
            insertInInCommandList(E_S6K_SouvisActive, "S6K_SouvisActive", 1);
        }
        else if (m_oParse_S6K_SouvisInspection)
        {
            insertInInCommandList(E_S6K_SouvisInspection, "S6K_SouvisInspection", 1);
        }
        else if (m_oParse_S6K_QuitSystemFault)
        {
            insertInInCommandList(E_S6K_QuitSystemFault, "S6K_QuitSystemFault", 1);
        }
        else if (m_oParse_S6K_MachineReady)
        {
            insertInInCommandList(E_S6K_MachineReady, "S6K_MachineReady", 1);
        }
        else if (m_oParse_S6K_ProductNumber)
        {
            insertInInCommandList(E_S6K_ProductNumber, "S6K_ProductNumber", m_oTempLength);
        }
        else if (m_oParse_S6K_AcknQualityData)
        {
            insertInInCommandList(E_S6K_AcknQualityData, "S6K_AcknQualityData", 1);
        }
        else if (m_oParse_S6K_CycleDataValid)
        {
            insertInInCommandList(E_S6K_CycleDataValid, "S6K_CycleDataValid", 1);
        }
        else if (m_oParse_S6K_CycleData)
        {
            insertInInCommandList(E_S6K_CycleData, "S6K_CycleData", m_oTempLength);
        }
        else if (m_oParse_S6K_SeamNo)
        {
            insertInInCommandList(E_S6K_SeamNo, "S6K_SeamNo", m_oTempLength);
        }
        else if (m_oParse_S6K_SeamNoValid)
        {
            insertInInCommandList(E_S6K_SeamNoValid, "S6K_SeamNoValid", 1);
        }
        else if (m_oParse_S6K_AcknResultData)
        {
            insertInInCommandList(E_S6K_AcknResultData, "S6K_AcknResultData", 1);
        }
        else if (m_oParse_S6K_MakePictures)
        {
            insertInInCommandList(E_S6K_MakePictures, "S6K_MakePictures", 1);
        }
        else if (m_oParse_SM_AcknowledgeStep)
        {
            insertInInCommandList(E_SM_AcknowledgeStep, "SM_AcknowledgeStep", 1);
        }

        printFlag = false;
    }

    if (printFlag) std::cout << "EndElement: " << localName << std::endl;
}

void SAX_VIConfigParser::characters(const XMLChar ch[], int start, int length)
{
    if (m_oGetProductCode)
    {
        m_oTempParseString += std::string(ch + start, length);
    }
    if (m_oGetVendorID)
    {
        m_oTempParseString += std::string(ch + start, length);
    }
    if (m_oGetSlaveType)
    {
        m_oTempParseString += std::string(ch + start, length);
    }
    if (m_oGetInstance)
    {
        m_oTempParseString += std::string(ch + start, length);
    }
    if (m_oGetStartBit)
    {
        m_oTempParseString += std::string(ch + start, length);
    }
    if (m_oGetTriggerLevel)
    {
        m_oTempParseString += std::string(ch + start, length);
    }
    if (m_oGetLength)
    {
        m_oTempParseString += std::string(ch + start, length);
    }
}

// ContentHandler
void SAX_VIConfigParser::setDocumentLocator(const Locator* loc)
{
    _pLocator = loc;
}

// Not used
void SAX_VIConfigParser::skippedEntity(const XMLString& name)
{
}
void SAX_VIConfigParser::endPrefixMapping(const XMLString& prefix)
{
}
void SAX_VIConfigParser::startPrefixMapping(const XMLString& prefix, const XMLString& uri)
{
}
void SAX_VIConfigParser::processingInstruction(const XMLString& target, const XMLString& data)
{
}
void SAX_VIConfigParser::ignorableWhitespace(const XMLChar ch[], int start, int length)
{
}
void SAX_VIConfigParser::startDTD(const XMLString& name, const XMLString& publicId, const XMLString& systemId)
{
}
void SAX_VIConfigParser::endDTD()
{
}
void SAX_VIConfigParser::startEntity(const XMLString& name)
{
}
void SAX_VIConfigParser::endEntity(const XMLString& name)
{
}
void SAX_VIConfigParser::startCDATA()
{
}
void SAX_VIConfigParser::endCDATA()
{
}
void SAX_VIConfigParser::comment(const XMLChar ch[], int start, int length)
{
}
// End of Not used

} // namespace ethercat
} // namespace precitec


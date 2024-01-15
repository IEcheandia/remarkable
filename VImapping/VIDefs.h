/*
 * VIDefs.h
 *
 *  Created on: 26.07.2010
 *      Author: f.agrawal
 */

#ifndef VIDEFS_H_
#define VIDEFS_H_

enum CommandType
{
    ETriggerStartStopContinuously, ETriggerStartStopAutomatic, ETriggerInspectionInfo, ETriggerInspectionPreStart,
    ETriggerInspectionStartEnd, ETriggerUnblockLineLaser, EChangeToStandardMode,
    ETriggerQuitSystemFault, ETriggerHomingYAxis,
    EProductType, EProductNumber, EExtendedProductInfo, ESeamseriesNr, ESeamNr, ECalibrationType, ETriggerStartStopCalibration,
    EGenPurposeAnaIn1, EGenPurposeAnaIn2, EGenPurposeAnaIn3, EGenPurposeAnaIn4, EGenPurposeAnaIn5, EGenPurposeAnaIn6, EGenPurposeAnaIn7, EGenPurposeAnaIn8,
    ELaserPowerSignal, EEncoderInput1, EEncoderInput2, ERobotTrackSpeed,
    ETrackerPosOutput, ETrackerOSCOutput, ETrackerScannerOK, ETrackerScannerLimits,
    EZCError, EZCPosReached, ELCErrorSignal, ELCReadySignal, ELCLimitWarning,
    ELEDTemperatureHigh,
    EOversamplingSignal1,EOversamplingSignal2,EOversamplingSignal3,EOversamplingSignal4,
    EOversamplingSignal5,EOversamplingSignal6,EOversamplingSignal7,EOversamplingSignal8,
    ELWM40_1_Plasma, ELWM40_1_Temperature, ELWM40_1_BackReflection, ELWM40_1_AnalogInput,
    EInvertedTriggerEmergencyStop,ECabinetTemperatureOk,
    EGenPurposeDigIn1,EGenPurposeDigInAddress,EGenPurposeDigInTakeOver,EGenPurposeDigOutTakeOver,
    ETriggerProductNumberFull, EAcknResultsReadyFull, ETriggerQuitSystemFaultFull, EGetProductTypeFull, EGetProductNumberFull,

    E_S6K_SouvisActive, E_S6K_SouvisInspection, E_S6K_QuitSystemFault, E_S6K_MachineReady, E_S6K_ProductNumber, E_S6K_AcknQualityData,
    E_S6K_CycleDataValid, E_S6K_CycleData, E_S6K_SeamNo, E_S6K_SeamNoValid, E_S6K_AcknResultData, E_S6K_MakePictures,
    E_SM_AcknowledgeStep,

    ALineLaser1Intens, ALineLaser2Intens, AFieldLight1Intens, ATrackerScanWidth, ATrackerScanPos, ATrackerEnableDriver,
    ASystemReady, AInspectCycleAckn, AInspectionPreStartAckn, AInspectionStartEndAckn, AInspectionOK, ASumErrorLatched, AInspectionIncomplete,
    ASystemErrorField,
    AQualityErrorField, ACalibResultsField, APositionResultsField, AProductType, AProductNumber, AExtendedProductInfo, ASumErrorSeam, ASumErrorSeamSeries,
    AZCRefTravel, AZCAutomatic, AZCAnalogIn, ALCStartSignal, ALCPowerOffset, ACabinetTemperatureOk,
    AGenPurposeAnaOut1, AGenPurposeAnaOut2, AGenPurposeAnaOut3, AGenPurposeAnaOut4, AGenPurposeAnaOut5, AGenPurposeAnaOut6, AGenPurposeAnaOut7, AGenPurposeAnaOut8,
    AGenPurposeDigInAckn,
    AAcknProductNumberFull, ATriggerResultsReadyFull, ASetInspectionOKFull, ASetSumErrorLatchedFull,
    ASetQualityErrorFieldFull, ASetSystemReadyStatusFull, ASetCabinetTemperatureOkFull, ASetHMSignalsFieldFull, ASetSystemErrorFieldFull,
    ASetProductTypeFull, ASetProductNumberFull,

    A_S6K_SystemFault, A_S6K_SystemReady, A_S6K_FastStopDoubleBlank, A_S6K_SeamErrorCat1, A_S6K_SeamErrorCat2, A_S6K_QualityDataValid, A_S6K_AcknCycleData,
    A_S6K_CycleDataMirror, A_S6K_SeamNoMirror, A_S6K_AcknSeamNo, A_S6K_ResultDataValid, A_S6K_ResultDataCount,
    A_S6K_ResultsImage1, A_S6K_ResultsImage2, A_S6K_ResultsImage3, A_S6K_ResultsImage4, A_S6K_ResultsImage5,
    A_S6K_ResultsImage6, A_S6K_ResultsImage7, A_S6K_ResultsImage8, A_S6K_ResultsImage9, A_S6K_ResultsImage10,
    A_S6K_ResultsImage11, A_S6K_ResultsImage12, A_S6K_ResultsImage13, A_S6K_ResultsImage14, A_S6K_ResultsImage15,
    A_S6K_ResultsImage16, A_S6K_ResultsImage17, A_S6K_ResultsImage18,
    A_SM_ProcessingActive, A_SM_TakeoverStep, A_SM_StepField
};

struct SLAVE_PROXY_INFORMATION{
    bool m_oActive;
    int nSlaveType;
    unsigned int nProductCode;
    unsigned int nVendorID;
    unsigned int nInstance;
    unsigned int nStartBit;
    int nTriggerLevel;
    unsigned int nLength;
    EcatProductIndex m_oProductIndex;
    EcatInstance m_oInstance;
    EcatChannel m_oChannel;
};

struct COMMAND_INFORMATION{
    SLAVE_PROXY_INFORMATION proxyInfo;
    CommandType commandType;
};

#endif /* VIDEFS_H_ */

#include "gatewayModel.h"
#include "event/ethercatDefines.h"

#include <bitset>

namespace precitec
{
namespace gui
{
namespace components
{
namespace ethercat
{

GatewayModel::GatewayModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &GatewayModel::viConfigChanged, this, &GatewayModel::initGateway);
    connect(this, &GatewayModel::inputDataChanged, this,
            [this]
            {
                if (m_inputSize > 0)
                {
                    emit dataChanged(index(0, 0), index(m_inputSize - 1, 0), {Qt::UserRole + 3});
                }
            }
    );
    connect(this, &GatewayModel::outputDataChanged, this,
            [this]
            {
                if (m_outputSize > 0)
                {
                    emit dataChanged(index(m_inputSize, 0), index(rowCount() - 1, 0), {Qt::UserRole + 3});
                }
            }
    );
}

GatewayModel::~GatewayModel() = default;

QHash<int, QByteArray> GatewayModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {Qt::UserRole, QByteArrayLiteral("byte")},
        {Qt::UserRole + 1, QByteArrayLiteral("bit")},
        {Qt::UserRole + 2, QByteArrayLiteral("type")},
        {Qt::UserRole + 3, QByteArrayLiteral("state")}
    };
}

QString GatewayModel::localizeSignal(const ViConfigService::Signal &signal, int bit, bool skipNumbers)
{
    static const std::map<QString, const char*> s_languageMap{
        {QStringLiteral("TriggerAutomaticMode"), QT_TRANSLATE_NOOP("", "Inspection Cycle Active")},
        {QStringLiteral("TriggerInspectionInfo"), QT_TRANSLATE_NOOP("", "Take-Over Seam Series")},
        {QStringLiteral("TriggerInspectionStartEnd"), QT_TRANSLATE_NOOP("", "Inspection Seam Active")},
        {QStringLiteral("TriggerUnblockLineLaser"), QT_TRANSLATE_NOOP("", "Reserved (Line laser on)")},
        {QStringLiteral("ChangeToStandardMode"), QT_TRANSLATE_NOOP("", "Single seam mode active")},
        {QStringLiteral("TriggerContinuouslyMode"), QT_TRANSLATE_NOOP("", "Option: Start continuous processing")},
        {QStringLiteral("Seamseries"), QT_TRANSLATE_NOOP("", "Seam Series Number Bit %1")},
        {QStringLiteral("SeamNr"), QT_TRANSLATE_NOOP("", "Seam Number Bit %1")},
        {QStringLiteral("SM_AcknowledgeStep"), QT_TRANSLATE_NOOP("", "Option: Ack. Step")},
        {QStringLiteral("TriggerQuitSystemFault"), QT_TRANSLATE_NOOP("", "Ack. System Fault")},
        {QStringLiteral("CalibrationType"), QT_TRANSLATE_NOOP("", "Calibration Type Bit %1")},
        {QStringLiteral("TriggerCalibrationMode"), QT_TRANSLATE_NOOP("", "Calibration Active")},
        {QStringLiteral("GeneralPurposeDigitalOutputTakeOver"), QT_TRANSLATE_NOOP("", "Option: take-over output value")},
        {QStringLiteral("GeneralPurposeDigitalInputTakeOver"), QT_TRANSLATE_NOOP("", "Option: take-over input value")},
        {QStringLiteral("GeneralPurposeDigitalInputAddress"), QT_TRANSLATE_NOOP("", "Option: input/output value address Bit %1")},
        {QStringLiteral("GeneralPurposeDigitalInput1"), QT_TRANSLATE_NOOP("", "Option: input value Bit %1")},
        {QStringLiteral("ProductType"), QT_TRANSLATE_NOOP("", "Part Type Bit %1")},
        {QStringLiteral("ProductNumber"), QT_TRANSLATE_NOOP("", "Part No Bit %1")},
        {QStringLiteral("ExtendedProductInfo_UTF8"), QT_TRANSLATE_NOOP("", "Option: extended product info (UTF-8) Bit %1")},
        {QStringLiteral("InspectCycleAckn"), QT_TRANSLATE_NOOP("", "Ack. Inspection Cycle")},
        {QStringLiteral("InspectionOK"), QT_TRANSLATE_NOOP("", "Part OK (OK=1)")},
        {QStringLiteral("SumErrorLatched"), QT_TRANSLATE_NOOP("", "Part NOK (NOK = 1)")},
        {QStringLiteral("SumErrorSeamSeries"), QT_TRANSLATE_NOOP("", "Seam Series NOK")},
        {QStringLiteral("SumErrorSeam"), QT_TRANSLATE_NOOP("", "Seam NOK")},
        {QStringLiteral("QualityErrorField"), QT_TRANSLATE_NOOP("", "Quality Fault Type %1")},
        {QStringLiteral("InspectionIncomplete"), QT_TRANSLATE_NOOP("", "Reserved (Inspection Incomplete)")},
        {QStringLiteral("SM_ProcessingActive"), QT_TRANSLATE_NOOP("", "ScanMaster seam processing is active")},
        {QStringLiteral("SM_TakeoverStep"), QT_TRANSLATE_NOOP("", "Option: new step field is present")},
        {QStringLiteral("SM_StepField"), QT_TRANSLATE_NOOP("", "Option: step field Bit %1")},
        {QStringLiteral("SystemReadyStatus"), QT_TRANSLATE_NOOP("", "System Ready (collective signal)")},
        {QStringLiteral("CabinetTemperatureOk"), QT_TRANSLATE_NOOP("", "Option: control cabinet temperature OK (OK = 1)")},
        {QStringLiteral("SystemErrorField0"), QT_TRANSLATE_NOOP("", "System Fault Lighting")},
        {QStringLiteral("SystemErrorField1"), QT_TRANSLATE_NOOP("", "System Fault Image Acquisition")},
        {QStringLiteral("SystemErrorField2"), QT_TRANSLATE_NOOP("", "System Fault Axes")},
        {QStringLiteral("SystemErrorField3"), QT_TRANSLATE_NOOP("", "System Fault Internal Software Problem")},
        {QStringLiteral("SystemErrorField4"), QT_TRANSLATE_NOOP("", "System Fault Switch Cabinet Temperature")},
        {QStringLiteral("SystemErrorField5"), QT_TRANSLATE_NOOP("", "System Fault Fieldbus Communication")},
        {QStringLiteral("SystemErrorField6"), QT_TRANSLATE_NOOP("", "System Fault Data Consistency")},
        {QStringLiteral("SystemErrorField7"), QT_TRANSLATE_NOOP("", "System Fault PC hardware")},
        {QStringLiteral("SystemErrorField8"), QT_TRANSLATE_NOOP("", "System Fault Head Monitor")},
        {QStringLiteral("SystemErrorField9"), QT_TRANSLATE_NOOP("", "System Fault External Equipment")},
        {QStringLiteral("SystemErrorField10"), QT_TRANSLATE_NOOP("", "System Fault Reserved")},
        {QStringLiteral("SystemErrorField11"), QT_TRANSLATE_NOOP("", "System Fault Reserved")},
        {QStringLiteral("SystemErrorField12"), QT_TRANSLATE_NOOP("", "System Fault Reserved")},
        {QStringLiteral("SystemErrorField13"), QT_TRANSLATE_NOOP("", "System Fault Reserved")},
        {QStringLiteral("SystemErrorField14"), QT_TRANSLATE_NOOP("", "System Fault Reserved")},
        {QStringLiteral("SystemErrorField15"), QT_TRANSLATE_NOOP("", "System Fault Reserved")},
        {QStringLiteral("CalibResultsField0"), QT_TRANSLATE_NOOP("", "Calibration Finished")},
        {QStringLiteral("CalibResultsField1"), QT_TRANSLATE_NOOP("", "Calibration OK")},
        {QStringLiteral("CalibResultsField2"), QT_TRANSLATE_NOOP("", "Calibration NOK")},
        {QStringLiteral("GeneralPurposeDigitalInputAckn"), QT_TRANSLATE_NOOP("", "Option: acknowledge input/output value")},
        {QStringLiteral("PositionResultsField"), QT_TRANSLATE_NOOP("", "Option: measurement result Bit %1")},
        {QStringLiteral("ProductTypeMirror"), QT_TRANSLATE_NOOP("", "Part Type Bit %1")},
        {QStringLiteral("ProductNumberMirror"), QT_TRANSLATE_NOOP("", "Part No Bit %1")},
        {QStringLiteral("ExtendedProductInfoMirror_UTF8"), QT_TRANSLATE_NOOP("", "Option: extended product info (UTF-8) Bit %1")},
        {QStringLiteral("GlasNotPresent"), QT_TRANSLATE_NOOP("", "Option: Head Monitor: GlassNotPresent")},
        {QStringLiteral("GlasDirty"), QT_TRANSLATE_NOOP("", "Option: Head Monitor: GlassDirty")},
        {QStringLiteral("TempGlasFail"), QT_TRANSLATE_NOOP("", "Option: Head Monitor: TempGlassFail")},
        {QStringLiteral("TempHeadFail"), QT_TRANSLATE_NOOP("", "Option: Head Monitor: TempHeadFail")},
        {QStringLiteral("TriggerProductNumberFull"), QT_TRANSLATE_NOOP("", "Read part type, part number")},
        {QStringLiteral("AcknResultsReadyFull"), QT_TRANSLATE_NOOP("", "Ack. results")},
        {QStringLiteral("TriggerQuitSystemFaultFull"), QT_TRANSLATE_NOOP("", "Ack. System Fault")},
        {QStringLiteral("GetProductTypeFull"), QT_TRANSLATE_NOOP("", "Part Type Bit %1")},
        {QStringLiteral("GetProductNumberFull"), QT_TRANSLATE_NOOP("", "Part No Bit %1")},
        {QStringLiteral("AcknProductNumberFull"), QT_TRANSLATE_NOOP("", "Ack. part type, part number")},
        {QStringLiteral("TriggerResultsReadyFull"), QT_TRANSLATE_NOOP("", "Takeover results")},
        {QStringLiteral("SetInspectionOKFull"), QT_TRANSLATE_NOOP("", "Part OK (OK=1)")},
        {QStringLiteral("SetSumErrorLatchedFull"), QT_TRANSLATE_NOOP("", "Part NOK (NOK = 1)")},
        {QStringLiteral("SetQualityErrorFieldFull"), QT_TRANSLATE_NOOP("", "Quality Fault Type %1")},
        {QStringLiteral("SetSystemReadyStatusFull"), QT_TRANSLATE_NOOP("", "System Ready (collective signal)")},
        {QStringLiteral("SetCabinetTemperatureOkFull"), QT_TRANSLATE_NOOP("", "Option: control cabinet temperature OK (OK = 1)")},
        {QStringLiteral("SetHMSignalsFieldFull0"), QT_TRANSLATE_NOOP("", "Option: Head Monitor: GlassNotPresent")},
        {QStringLiteral("SetHMSignalsFieldFull1"), QT_TRANSLATE_NOOP("", "Option: Head Monitor: GlassDirty")},
        {QStringLiteral("SetHMSignalsFieldFull2"), QT_TRANSLATE_NOOP("", "Option: Head Monitor: TempGlassFail")},
        {QStringLiteral("SetHMSignalsFieldFull3"), QT_TRANSLATE_NOOP("", "Option: Head Monitor: TempHeadFail")},
        {QStringLiteral("SetSystemErrorFieldFull0"), QT_TRANSLATE_NOOP("", "System Fault Lighting")},
        {QStringLiteral("SetSystemErrorFieldFull1"), QT_TRANSLATE_NOOP("", "System Fault Image Acquisition")},
        {QStringLiteral("SetSystemErrorFieldFull2"), QT_TRANSLATE_NOOP("", "System Fault Axes")},
        {QStringLiteral("SetSystemErrorFieldFull3"), QT_TRANSLATE_NOOP("", "System Fault Internal Software Problem")},
        {QStringLiteral("SetSystemErrorFieldFull4"), QT_TRANSLATE_NOOP("", "System Fault Switch Cabinet Temperature")},
        {QStringLiteral("SetSystemErrorFieldFull5"), QT_TRANSLATE_NOOP("", "System Fault Fieldbus Communication")},
        {QStringLiteral("SetSystemErrorFieldFull6"), QT_TRANSLATE_NOOP("", "System Fault Data Consistency")},
        {QStringLiteral("SetSystemErrorFieldFull7"), QT_TRANSLATE_NOOP("", "System Fault PC hardware")},
        {QStringLiteral("SetSystemErrorFieldFull8"), QT_TRANSLATE_NOOP("", "System Fault Head Monitor")},
        {QStringLiteral("SetSystemErrorFieldFull9"), QT_TRANSLATE_NOOP("", "System Fault External Equipment")},
        {QStringLiteral("SetSystemErrorFieldFull10"), QT_TRANSLATE_NOOP("", "System Fault Reserved")},
        {QStringLiteral("SetSystemErrorFieldFull11"), QT_TRANSLATE_NOOP("", "System Fault Reserved")},
        {QStringLiteral("SetSystemErrorFieldFull12"), QT_TRANSLATE_NOOP("", "System Fault Reserved")},
        {QStringLiteral("SetSystemErrorFieldFull13"), QT_TRANSLATE_NOOP("", "System Fault Reserved")},
        {QStringLiteral("SetSystemErrorFieldFull14"), QT_TRANSLATE_NOOP("", "System Fault Reserved")},
        {QStringLiteral("SetSystemErrorFieldFull15"), QT_TRANSLATE_NOOP("", "System Fault Reserved")},
        {QStringLiteral("SetProductTypeFull"), QT_TRANSLATE_NOOP("", "Part Type Bit %1")},
        {QStringLiteral("SetProductNumberFull"), QT_TRANSLATE_NOOP("", "Part No Bit %1")},
        {QStringLiteral("S6K_MakePictures"), QT_TRANSLATE_NOOP("", "Make pictures")},
        {QStringLiteral("S6K_SouvisActive"), QT_TRANSLATE_NOOP("", "Souvis Active")},
        {QStringLiteral("S6K_SouvisInspection"), QT_TRANSLATE_NOOP("", "Souvis Inspection")},
        {QStringLiteral("S6K_QuitSystemFault"), QT_TRANSLATE_NOOP("", "Ack. System Fault")},
        {QStringLiteral("S6K_MachineReady"), QT_TRANSLATE_NOOP("", "Machine Ready")},
        {QStringLiteral("S6K_ProductNumber"), QT_TRANSLATE_NOOP("", "Product Number Bit %1")},
        {QStringLiteral("S6K_AcknQualityData"), QT_TRANSLATE_NOOP("", "Ack. Quality Data")},
        {QStringLiteral("S6K_CycleDataValid"), QT_TRANSLATE_NOOP("", "Cycle Data Valid")},
        {QStringLiteral("S6K_CycleData"), QT_TRANSLATE_NOOP("", "Cycle Data Bit %1")},
        {QStringLiteral("S6K_SeamNo"), QT_TRANSLATE_NOOP("", "Seam Number Bit %1")},
        {QStringLiteral("S6K_SeamNoValid"), QT_TRANSLATE_NOOP("", "Seam Number Valid")},
        {QStringLiteral("S6K_AcknResultData"), QT_TRANSLATE_NOOP("", "Ack. Results")},
        {QStringLiteral("S6K_SystemFault"), QT_TRANSLATE_NOOP("", "System Fault")},
        {QStringLiteral("S6K_SystemReady"), QT_TRANSLATE_NOOP("", "System Ready")},
        {QStringLiteral("S6K_FastStop_DoubleBlank"), QT_TRANSLATE_NOOP("", "Fault Double Blank")},
        {QStringLiteral("S6K_QualityDataValid"), QT_TRANSLATE_NOOP("", "Quality Data Valid")},
        {QStringLiteral("S6K_AcknCycleData"), QT_TRANSLATE_NOOP("", "Ack. Cycle Data")},
        {QStringLiteral("S6K_CycleDataMirror"), QT_TRANSLATE_NOOP("", "Cycle Data Bit %1")},
        {QStringLiteral("S6K_SeamNoMirror"), QT_TRANSLATE_NOOP("", "Seam Number Bit %1")},
        {QStringLiteral("S6K_AcknSeamNo"), QT_TRANSLATE_NOOP("", "Ack. Seam Number")},
        {QStringLiteral("S6K_ResultDataValid"), QT_TRANSLATE_NOOP("", "Results Valid")},
        {QStringLiteral("S6K_ResultDataCount"), QT_TRANSLATE_NOOP("", "Results Count Bit %1")},
        {QStringLiteral("S6K_SeamErrorCat10"), QT_TRANSLATE_NOOP("", "OK/NOK seam 1 (NOK=1) Category 1")},
        {QStringLiteral("S6K_SeamErrorCat11"), QT_TRANSLATE_NOOP("", "OK/NOK seam 2 (NOK=1) Category 1")},
        {QStringLiteral("S6K_SeamErrorCat12"), QT_TRANSLATE_NOOP("", "OK/NOK seam 3 (NOK=1) Category 1")},
        {QStringLiteral("S6K_SeamErrorCat13"), QT_TRANSLATE_NOOP("", "OK/NOK seam 4 (NOK=1) Category 1")},
        {QStringLiteral("S6K_SeamErrorCat14"), QT_TRANSLATE_NOOP("", "OK/NOK seam 5 (NOK=1) Category 1")},
        {QStringLiteral("S6K_SeamErrorCat15"), QT_TRANSLATE_NOOP("", "OK/NOK seam 6 (NOK=1) Category 1")},
        {QStringLiteral("S6K_SeamErrorCat16"), QT_TRANSLATE_NOOP("", "OK/NOK seam 7 (NOK=1) Category 1")},
        {QStringLiteral("S6K_SeamErrorCat17"), QT_TRANSLATE_NOOP("", "OK/NOK seam 8 (NOK=1) Category 1")},
        {QStringLiteral("S6K_SeamErrorCat18"), QT_TRANSLATE_NOOP("", "OK/NOK seam 9 (NOK=1) Category 1")},
        {QStringLiteral("S6K_SeamErrorCat19"), QT_TRANSLATE_NOOP("", "OK/NOK seam 10 (NOK=1) Category 1")},
        {QStringLiteral("S6K_SeamErrorCat110"), QT_TRANSLATE_NOOP("", "OK/NOK seam 11 (NOK=1) Category 1")},
        {QStringLiteral("S6K_SeamErrorCat111"), QT_TRANSLATE_NOOP("", "OK/NOK seam 12 (NOK=1) Category 1")},
        {QStringLiteral("S6K_SeamErrorCat112"), QT_TRANSLATE_NOOP("", "OK/NOK seam 13 (NOK=1) Category 1")},
        {QStringLiteral("S6K_SeamErrorCat113"), QT_TRANSLATE_NOOP("", "OK/NOK seam 14 (NOK=1) Category 1")},
        {QStringLiteral("S6K_SeamErrorCat114"), QT_TRANSLATE_NOOP("", "OK/NOK seam 15 (NOK=1) Category 1")},
        {QStringLiteral("S6K_SeamErrorCat115"), QT_TRANSLATE_NOOP("", "OK/NOK seam 16 (NOK=1) Category 1")},
        {QStringLiteral("S6K_SeamErrorCat116"), QT_TRANSLATE_NOOP("", "OK/NOK seam 17 (NOK=1) Category 1")},
        {QStringLiteral("S6K_SeamErrorCat117"), QT_TRANSLATE_NOOP("", "OK/NOK seam 18 (NOK=1) Category 1")},
        {QStringLiteral("S6K_SeamErrorCat118"), QT_TRANSLATE_NOOP("", "OK/NOK seam 19 (NOK=1) Category 1")},
        {QStringLiteral("S6K_SeamErrorCat119"), QT_TRANSLATE_NOOP("", "OK/NOK seam 20 (NOK=1) Category 1")},
        {QStringLiteral("S6K_SeamErrorCat120"), QT_TRANSLATE_NOOP("", "OK/NOK seam 21 (NOK=1) Category 1")},
        {QStringLiteral("S6K_SeamErrorCat121"), QT_TRANSLATE_NOOP("", "OK/NOK seam 22 (NOK=1) Category 1")},
        {QStringLiteral("S6K_SeamErrorCat122"), QT_TRANSLATE_NOOP("", "OK/NOK seam 23 (NOK=1) Category 1")},
        {QStringLiteral("S6K_SeamErrorCat123"), QT_TRANSLATE_NOOP("", "OK/NOK seam 24 (NOK=1) Category 1")},
        {QStringLiteral("S6K_SeamErrorCat20"), QT_TRANSLATE_NOOP("", "OK/NOK seam 1 (NOK=1) Category 2")},
        {QStringLiteral("S6K_SeamErrorCat21"), QT_TRANSLATE_NOOP("", "OK/NOK seam 2 (NOK=1) Category 2")},
        {QStringLiteral("S6K_SeamErrorCat22"), QT_TRANSLATE_NOOP("", "OK/NOK seam 3 (NOK=1) Category 2")},
        {QStringLiteral("S6K_SeamErrorCat23"), QT_TRANSLATE_NOOP("", "OK/NOK seam 4 (NOK=1) Category 2")},
        {QStringLiteral("S6K_SeamErrorCat24"), QT_TRANSLATE_NOOP("", "OK/NOK seam 5 (NOK=1) Category 2")},
        {QStringLiteral("S6K_SeamErrorCat25"), QT_TRANSLATE_NOOP("", "OK/NOK seam 6 (NOK=1) Category 2")},
        {QStringLiteral("S6K_SeamErrorCat26"), QT_TRANSLATE_NOOP("", "OK/NOK seam 7 (NOK=1) Category 2")},
        {QStringLiteral("S6K_SeamErrorCat27"), QT_TRANSLATE_NOOP("", "OK/NOK seam 8 (NOK=1) Category 2")},
        {QStringLiteral("S6K_SeamErrorCat28"), QT_TRANSLATE_NOOP("", "OK/NOK seam 9 (NOK=1) Category 2")},
        {QStringLiteral("S6K_SeamErrorCat29"), QT_TRANSLATE_NOOP("", "OK/NOK seam 10 (NOK=1) Category 2")},
        {QStringLiteral("S6K_SeamErrorCat210"), QT_TRANSLATE_NOOP("", "OK/NOK seam 11 (NOK=1) Category 2")},
        {QStringLiteral("S6K_SeamErrorCat211"), QT_TRANSLATE_NOOP("", "OK/NOK seam 12 (NOK=1) Category 2")},
        {QStringLiteral("S6K_SeamErrorCat212"), QT_TRANSLATE_NOOP("", "OK/NOK seam 13 (NOK=1) Category 2")},
        {QStringLiteral("S6K_SeamErrorCat213"), QT_TRANSLATE_NOOP("", "OK/NOK seam 14 (NOK=1) Category 2")},
        {QStringLiteral("S6K_SeamErrorCat214"), QT_TRANSLATE_NOOP("", "OK/NOK seam 15 (NOK=1) Category 2")},
        {QStringLiteral("S6K_SeamErrorCat215"), QT_TRANSLATE_NOOP("", "OK/NOK seam 16 (NOK=1) Category 2")},
        {QStringLiteral("S6K_SeamErrorCat216"), QT_TRANSLATE_NOOP("", "OK/NOK seam 17 (NOK=1) Category 2")},
        {QStringLiteral("S6K_SeamErrorCat217"), QT_TRANSLATE_NOOP("", "OK/NOK seam 18 (NOK=1) Category 2")},
        {QStringLiteral("S6K_SeamErrorCat218"), QT_TRANSLATE_NOOP("", "OK/NOK seam 19 (NOK=1) Category 2")},
        {QStringLiteral("S6K_SeamErrorCat219"), QT_TRANSLATE_NOOP("", "OK/NOK seam 20 (NOK=1) Category 2")},
        {QStringLiteral("S6K_SeamErrorCat220"), QT_TRANSLATE_NOOP("", "OK/NOK seam 21 (NOK=1) Category 2")},
        {QStringLiteral("S6K_SeamErrorCat221"), QT_TRANSLATE_NOOP("", "OK/NOK seam 22 (NOK=1) Category 2")},
        {QStringLiteral("S6K_SeamErrorCat222"), QT_TRANSLATE_NOOP("", "OK/NOK seam 23 (NOK=1) Category 2")},
        {QStringLiteral("S6K_SeamErrorCat223"), QT_TRANSLATE_NOOP("", "OK/NOK seam 24 (NOK=1) Category 2")},
        {QStringLiteral("S6K_ResultsImage1"), QT_TRANSLATE_NOOP("", "Results of image 1 Bit %1")},
        {QStringLiteral("S6K_ResultsImage2"), QT_TRANSLATE_NOOP("", "Results of image 2 Bit %1")},
        {QStringLiteral("S6K_ResultsImage3"), QT_TRANSLATE_NOOP("", "Results of image 3 Bit %1")},
        {QStringLiteral("S6K_ResultsImage4"), QT_TRANSLATE_NOOP("", "Results of image 4 Bit %1")},
        {QStringLiteral("S6K_ResultsImage5"), QT_TRANSLATE_NOOP("", "Results of image 5 Bit %1")},
        {QStringLiteral("S6K_ResultsImage6"), QT_TRANSLATE_NOOP("", "Results of image 6 Bit %1")},
        {QStringLiteral("S6K_ResultsImage7"), QT_TRANSLATE_NOOP("", "Results of image 7 Bit %1")},
        {QStringLiteral("S6K_ResultsImage8"), QT_TRANSLATE_NOOP("", "Results of image 8 Bit %1")},
        {QStringLiteral("S6K_ResultsImage9"), QT_TRANSLATE_NOOP("", "Results of image 9 Bit %1")},
        {QStringLiteral("S6K_ResultsImage10"), QT_TRANSLATE_NOOP("", "Results of image 10 Bit %1")},
        {QStringLiteral("S6K_ResultsImage11"), QT_TRANSLATE_NOOP("", "Results of image 11 Bit %1")},
        {QStringLiteral("S6K_ResultsImage12"), QT_TRANSLATE_NOOP("", "Results of image 12 Bit %1")},
        {QStringLiteral("S6K_ResultsImage13"), QT_TRANSLATE_NOOP("", "Results of image 13 Bit %1")},
        {QStringLiteral("S6K_ResultsImage14"), QT_TRANSLATE_NOOP("", "Results of image 14 Bit %1")},
        {QStringLiteral("S6K_ResultsImage15"), QT_TRANSLATE_NOOP("", "Results of image 15 Bit %1")},
        {QStringLiteral("S6K_ResultsImage16"), QT_TRANSLATE_NOOP("", "Results of image 16 Bit %1")},
        {QStringLiteral("S6K_ResultsImage17"), QT_TRANSLATE_NOOP("", "Results of image 17 Bit %1")},
        {QStringLiteral("S6K_ResultsImage18"), QT_TRANSLATE_NOOP("", "Results of image 18 Bit %1")}
    };

    static const std::map<QString, const char*> s_languageMapNoBits{
        {QStringLiteral("Seamseries"), QT_TRANSLATE_NOOP("", "Seam Series Number")},
        {QStringLiteral("SeamNr"), QT_TRANSLATE_NOOP("", "Seam Number")},
        {QStringLiteral("CalibrationType"), QT_TRANSLATE_NOOP("", "Calibration Type")},
        {QStringLiteral("GeneralPurposeDigitalInputAddress"), QT_TRANSLATE_NOOP("", "Option: input/output value address")},
        {QStringLiteral("GeneralPurposeDigitalInput1"), QT_TRANSLATE_NOOP("", "Option: input value")},
        {QStringLiteral("ProductType"), QT_TRANSLATE_NOOP("", "Part Type")},
        {QStringLiteral("ProductNumber"), QT_TRANSLATE_NOOP("", "Part No")},
        {QStringLiteral("ExtendedProductInfo_UTF8"), QT_TRANSLATE_NOOP("", "Option: extended product info (UTF-8)")},
        {QStringLiteral("QualityErrorField"), QT_TRANSLATE_NOOP("", "Quality Fault Type")},
        {QStringLiteral("SM_StepField"), QT_TRANSLATE_NOOP("", "Option: step field")},
        {QStringLiteral("PositionResultsField"), QT_TRANSLATE_NOOP("", "Option: measurement result")},
        {QStringLiteral("ProductTypeMirror"), QT_TRANSLATE_NOOP("", "Part Type")},
        {QStringLiteral("ProductNumberMirror"), QT_TRANSLATE_NOOP("", "Part No")},
        {QStringLiteral("ExtendedProductInfoMirror_UTF8"), QT_TRANSLATE_NOOP("", "Option: extended product info (UTF-8)")},
        {QStringLiteral("GetProductTypeFull"), QT_TRANSLATE_NOOP("", "Part Type")},
        {QStringLiteral("GetProductNumberFull"), QT_TRANSLATE_NOOP("", "Part No")},
        {QStringLiteral("SetQualityErrorFieldFull"), QT_TRANSLATE_NOOP("", "Quality Fault Type")},
        {QStringLiteral("SetProductTypeFull"), QT_TRANSLATE_NOOP("", "Part Type")},
        {QStringLiteral("SetProductNumberFull"), QT_TRANSLATE_NOOP("", "Part No")},
        {QStringLiteral("S6K_ProductNumber"), QT_TRANSLATE_NOOP("", "Product Number")},
        {QStringLiteral("S6K_CycleData"), QT_TRANSLATE_NOOP("", "Cycle Data")},
        {QStringLiteral("S6K_SeamNo"), QT_TRANSLATE_NOOP("", "Seam Number")},
        {QStringLiteral("S6K_CycleDataMirror"), QT_TRANSLATE_NOOP("", "Cycle Data")},
        {QStringLiteral("S6K_SeamNoMirror"), QT_TRANSLATE_NOOP("", "Seam Number")},
        {QStringLiteral("S6K_ResultDataCount"), QT_TRANSLATE_NOOP("", "Results Count")},
        {QStringLiteral("S6K_ResultsImage1"), QT_TRANSLATE_NOOP("", "Results of image 1")},
        {QStringLiteral("S6K_ResultsImage2"), QT_TRANSLATE_NOOP("", "Results of image 2")},
        {QStringLiteral("S6K_ResultsImage3"), QT_TRANSLATE_NOOP("", "Results of image 3")},
        {QStringLiteral("S6K_ResultsImage4"), QT_TRANSLATE_NOOP("", "Results of image 4")},
        {QStringLiteral("S6K_ResultsImage5"), QT_TRANSLATE_NOOP("", "Results of image 5")},
        {QStringLiteral("S6K_ResultsImage6"), QT_TRANSLATE_NOOP("", "Results of image 6")},
        {QStringLiteral("S6K_ResultsImage7"), QT_TRANSLATE_NOOP("", "Results of image 7")},
        {QStringLiteral("S6K_ResultsImage8"), QT_TRANSLATE_NOOP("", "Results of image 8")},
        {QStringLiteral("S6K_ResultsImage9"), QT_TRANSLATE_NOOP("", "Results of image 9")},
        {QStringLiteral("S6K_ResultsImage10"), QT_TRANSLATE_NOOP("", "Results of image 10")},
        {QStringLiteral("S6K_ResultsImage11"), QT_TRANSLATE_NOOP("", "Results of image 11")},
        {QStringLiteral("S6K_ResultsImage12"), QT_TRANSLATE_NOOP("", "Results of image 12")},
        {QStringLiteral("S6K_ResultsImage13"), QT_TRANSLATE_NOOP("", "Results of image 13")},
        {QStringLiteral("S6K_ResultsImage14"), QT_TRANSLATE_NOOP("", "Results of image 14")},
        {QStringLiteral("S6K_ResultsImage15"), QT_TRANSLATE_NOOP("", "Results of image 15")},
        {QStringLiteral("S6K_ResultsImage16"), QT_TRANSLATE_NOOP("", "Results of image 16")},
        {QStringLiteral("S6K_ResultsImage17"), QT_TRANSLATE_NOOP("", "Results of image 17")},
        {QStringLiteral("S6K_ResultsImage18"), QT_TRANSLATE_NOOP("", "Results of image 18")}
    };
    if (skipNumbers)
    {
        auto it = s_languageMapNoBits.find(signal.name);
        if (it == s_languageMapNoBits.end())
        {
            it = s_languageMapNoBits.find(signal.name + QString::number(bit));
        }
        if (it != s_languageMapNoBits.end())
        {
            return tr(it->second);
        }
    }

    auto it = s_languageMap.find(signal.name);
    if (it == s_languageMap.end())
    {
        it = s_languageMap.find(signal.name + QString::number(bit));
        if (it == s_languageMap.end())
        {
            return signal.name;
        }
    }
    if (signal.length == 1)
    {
        return tr(it->second);
    } else
    {
        return tr(it->second).arg(bit);
    }
}

QVariant GatewayModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    auto it = m_signals.find(index.row());
    const bool isInput = index.row() < int(m_inputSize);
    const int byteNumber = isInput ? index.row() / 8 : (index.row() - m_inputSize) / 8 ;
    switch (role)
    {
    case Qt::DisplayRole:
        return it != m_signals.end() ? localizeSignal(it->second.first, it->second.second) : tr("Reserved");
    case Qt::UserRole:
        return byteNumber;
    case Qt::UserRole + 1:
        return index.row() % 8;
    case Qt::UserRole + 2:
        return QVariant::fromValue(isInput ? ViConfigService::SignalType::Input : ViConfigService::SignalType::Output);
    case Qt::UserRole +3:
    {
        const auto &data = isInput ? m_inputData : m_outputData;
        if (data.size() <= byteNumber)
        {
            return false;
        }
        std::bitset<8> mask{};
        mask.set(index.row() % 8);
        return bool(data.at(byteNumber) & mask.to_ulong());
    }
    }
    return {};
}

int GatewayModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_inputSize + m_outputSize;
}

void GatewayModel::setGateway(quint32 instance, quint32 inputSize, quint32 outputSize)
{
    m_instance = instance;
    m_inputSize = inputSize;
    m_outputSize = outputSize;
    initGateway();
}

void GatewayModel::initGateway()
{
    if (!m_viConfig)
    {
        return;
    }
    const auto gatewaySignals = m_viConfig->getSignals(VENDORID_HMS, PRODUCTCODE_ANYBUS_GW, m_instance);
    beginResetModel();
    m_signals.clear();

    const auto outputOffset = m_inputSize;
    for (auto it = gatewaySignals.begin(); it != gatewaySignals.end(); it++)
    {
        auto row = (*it).startBit;
        if ((*it).type == ViConfigService::SignalType::Output)
        {
            row += outputOffset;
        }
        for (quint32 i = 0; i < (*it).length; i++)
        {
            if ((*it).type == ViConfigService::SignalType::Input && (row + i >= outputOffset))
            {
                continue;
            }
            m_signals.emplace(row + i, std::make_pair((*it), i));
        }
    }

    endResetModel();
}

void GatewayModel::setViConfig(ViConfigService *service)
{
    if (m_viConfig == service)
    {
        return;
    }
    m_viConfig = service;
    disconnect(m_viConfigDestroyed);
    disconnect(m_viConfigSignalsChanged);
    if (m_viConfig)
    {
        m_viConfigDestroyed = connect(m_viConfig, &ViConfigService::destroyed, this, std::bind(&GatewayModel::setViConfig, this, nullptr));
        m_viConfigSignalsChanged = connect(m_viConfig, &ViConfigService::signalsChanged, this, &GatewayModel::initGateway);
    } else
    {
        m_viConfigDestroyed = {};
        m_viConfigSignalsChanged = {};
    }
    emit viConfigChanged();
}

void GatewayModel::setInputData(const QByteArray &data)
{
    if (m_inputData == data)
    {
        return;
    }
    m_inputData = data;
    emit inputDataChanged();
}

void GatewayModel::setOutputData(const QByteArray &data)
{
    if (m_outputData == data)
    {
        return;
    }
    m_outputData = data;
    emit outputDataChanged();
}

}
}
}
}

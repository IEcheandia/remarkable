#pragma once


#if (((_MANAGED == 1) || (_M_CEE == 1)) && PrecitecJunctionDll) || WM_RUNNER_DOMAIN
	#pragma message( " Managed build " )
#else
	// #pragma message( " Native build " ) // Sieht aus wie warning, daher kommentiert.
#endif

#if (((_MANAGED == 1) || (_M_CEE == 1)) && PrecitecJunctionDll) || WM_RUNNER_DOMAIN
namespace Precitec { namespace Junction {
#else
namespace precitec
{
namespace interface
{
#endif

	// liste aller moeglichen Resultattypen

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!      IMPORTANT: Do not forget to keep ResultType consistent with the enum defined in \win\Util\Precitec.Common\ServiceEntities\SGM2POCOEntities.cs      !!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#if (((_MANAGED == 1) || (_M_CEE == 1)) && PrecitecJunctionDll) || WM_RUNNER_DOMAIN
	public enum class ResultType {
#else
	enum ResultType {
#endif
		GapWidth = 0, 				// ab hier PreProcessResultEvent (Scantracker)
		GapPosition,				// (Position an Feldbus, Scantracker)
		Missmatch,
		AxisPositionAbsolute,		// (LD50)
		AxisPositionRelative,		// (LD50)
		SeamWidth, 					// ab hier PostProcessResultEvenr
		KeyholePosition,

		//Missmatch, Concavity,
		Convexity,
		Roundness,
		NoSeam,
		NoStructure,
		SideBurn,
		Pores,
		Holes,
		Spurt,
		Notch,
		EdgeSkew,
		SeamThickness,
		UnEqualLegs,
		SeamLength,
		ReangeLength,
		Weldbed,					// ab hier InProcessResultEvent
		ControlOut,
		NoLaser,
		NoLight,

		LaserPower,    // 25 (2012/02/14)
		Value,

		CoordPosition,
		CoordPositionX,
		CoordPositionY,
		CoordPositionZ,

		CalibrationTopLayer,    // calibration layer information, not really a resulttype
		CalibrationBottomLayer,
		CalibrationOK,          // calibration graph evaluated everything as expected

		EFAINFeatures,

		// Default result for analysis, indicates that no problem has occured. should be last IO
		AnalysisOK,
		NumResults,

		LD50_Y_AbsolutePosition = 100,
		LD50_Y_RelativePosition,
		LD50_Z_AbsolutePosition,
		LD50_Z_RelativePosition,
		ScanTracker_Amplitude,
		ScanTracker_Offset,
		LaserControl_PowerOffset,
		Z_Collimator_Position,
		Fieldbus_PositionResult,
		GeneralPurpose_AnalogOut1,
		GeneralPurpose_DigitalOut1,    // 110
		S6K_EdgePosition,              // 111
		S6K_GapWidth,
		S6K_SeamWidth,
		S6K_HeightDifference,
		S6K_Concavity,
		S6K_Convexity,
		S6K_PoreWidth,
        S6K_GreyLevel,
        S6K_AblationWidth,
        S6K_AblationBrightness,        // 120
        S6K_MillingEdgePosition,       // 121
        ScanmasterSeamWelding,
		GeneralPurpose_AnalogOut2,
		GeneralPurpose_AnalogOut3,
		GeneralPurpose_AnalogOut4,
		GeneralPurpose_AnalogOut5,
		GeneralPurpose_AnalogOut6,
		GeneralPurpose_AnalogOut7,
		GeneralPurpose_AnalogOut8,
        ScanmasterScannerMoving,       // 130
        S6K_ConcavPosToCenter,
        S6K_Unknown,
        GeneralPurpose_DigitalOut2,
        GeneralPurpose_DigitalOut3,
        GeneralPurpose_DigitalOut4,
        GeneralPurpose_DigitalOut5,
        GeneralPurpose_DigitalOut6,
        GeneralPurpose_DigitalOut7,
        GeneralPurpose_DigitalOut8,
        EndOfSeamMarker,               // 140
        ScanmasterSeamWeldingAndEndOfSeamMarker,
        ScanmasterSpotWelding,
        PrepareContour,
        WeldPreparedContour,
        ScanmasterHeight,
        S6K_CrossSection_Result1,
        S6K_CrossSection_Result2,
        S6K_CrossSection_Result3,
        S6K_CrossSection_Result4,
        LWMStandardResult,

		NIOOffset = 1000,
		XCoordOutOfLimits,
		YCoordOutOfLimits,
		ZCoordOutOfLimits,
		ValueOutOfLimits,
		RankViolation,
		GapPositionError,
		LaserPowerOutOfLimits,
		SensorOutOfLimits,

		// these 4 surveillance values are special things, aren't they!?
		Surveillance01=1009,
		Surveillance02=1010,
		Surveillance03=1011,
		Surveillance04=1012,

        NoResultsError,

		LastSingleNio,

        BPTinySeam,

		// Analysis errors substituting old all in one bucket rank violation error
		AnalysisErrorOffset=1200,
		AnalysisErrBadLaserline = 1201,
		AnalysisErrNoBeadOrGap,
		AnalysisErrBlackImage,
		AnalysisErrBadImage,
		AnalysisErrBadCalibration,
		AnalysisErrBadROI,                   // Invalid ROI
		AnalysisErrDefectiveSeam,            // spatter, sparks or a defective seam
		AnalysisErrNotchDefect,              // notch
		AnalysisErrNoStep,

		LastAnalysisError,

		LuaError,				// LUA runtime error

		// SumErrorTypes
		SumErrorOffset=1500,   // only needed for NumNios!
		SumErrorNone=1500,
        //SignalSumError
		SignalSumErrorAccumulatedOutlierStaticBoundary,
		SignalSumErrorSingleOutlierStaticBoundary,
		SignalSumErrorAccumulatedAreaStaticBoundary,
		SignalSumErrorSingleAreaStaticBoundary,
        SignalSumErrorInlierStaticBoundary,
        SignalSumErrorPeakStaticBoundary,
        SignalSumErrorDualOutlierInRangeStaticBoundary,
		SignalSumErrorAccumulatedOutlierReferenceBoundary,
		SignalSumErrorSingleOutlierReferenceBoundary,
        SignalSumErrorAccumulatedAreaReferenceBoundary,
        SignalSumErrorSingleAreaReferenceBoundary,
        SignalSumErrorInlierReferenceBoundary,
        SignalSumErrorPeakReferenceBoundary,
        SignalSumErrorDualOutlierInRangeReferenceBoundary,
        //AnalysisSumError
        AnalysisSumErrorAccumulatedOutlier,
        AnalysisSumErrorAdjacentOutlier,
        //LevelTwoError
        LevelTwoErrorAccumulated,
        LevelTwoErrorAdjacent,
        LevelTwoErrorSelected,
        LevelTwoErrorErrorOnlyAccumulated,
        LevelTwoErrorErrorOnlyAdjacent,
        LineStop,
        EndOfSumErrors = 1550,

        QualityFaultTypeA = 1800,
        QualityFaultTypeB,
        QualityFaultTypeC,
        QualityFaultTypeD,
        QualityFaultTypeE,
        QualityFaultTypeF,
        QualityFaultTypeG,
        QualityFaultTypeH,
        QualityFaultTypeI,
        QualityFaultTypeJ,
        QualityFaultTypeK,
        QualityFaultTypeL,
        QualityFaultTypeM,
        QualityFaultTypeN,
        QualityFaultTypeO,
        QualityFaultTypeP,
        QualityFaultTypeQ,
        QualityFaultTypeR,
        QualityFaultTypeS,
        QualityFaultTypeT,
        QualityFaultTypeU,
        QualityFaultTypeV,
        QualityFaultTypeW,
        QualityFaultTypeX,

        QualityFaultTypeA_Cat2 = 1900,
        QualityFaultTypeB_Cat2,
        QualityFaultTypeC_Cat2,
        QualityFaultTypeD_Cat2,
        QualityFaultTypeE_Cat2,
        QualityFaultTypeF_Cat2,
        QualityFaultTypeG_Cat2,
        QualityFaultTypeH_Cat2,
        QualityFaultTypeI_Cat2,
        QualityFaultTypeJ_Cat2,
        QualityFaultTypeK_Cat2,
        QualityFaultTypeL_Cat2,
        QualityFaultTypeM_Cat2,
        QualityFaultTypeN_Cat2,
        QualityFaultTypeO_Cat2,
        QualityFaultTypeP_Cat2,
        QualityFaultTypeQ_Cat2,
        QualityFaultTypeR_Cat2,
        QualityFaultTypeS_Cat2,
        QualityFaultTypeT_Cat2,
        QualityFaultTypeU_Cat2,
        QualityFaultTypeV_Cat2,
        QualityFaultTypeW_Cat2,
        QualityFaultTypeX_Cat2,

        FastStop_DoubleBlank = 2000,

        FirstLineProfile = 20000,
        LineProfile1 = FirstLineProfile,
        LineProfile2,
        LineProfile3,
		LastLineProfile = 20010,

        ProcessTime = 20100,  //result used for debug
        InspectManagerTime,
        InspectManagerImageProcessingMode,

		EndOfResultTypes,
		// Nobody need NumNios: calculated value is 337 ????
		// NumNios = (EndOfResultTypes - SumErrorOffset) + (LastSingleNio - AnalysisErrorOffset - 1) + (LastAnalysisError - NIOOffset - 1)
	};

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// !!      IMPORTANT: in c# use Precitec.Junction.ResultType   !!
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

}}

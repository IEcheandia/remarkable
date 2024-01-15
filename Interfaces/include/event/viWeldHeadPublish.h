#ifndef VIWELDHEADPUBLISH_H_
#define VIWELDHEADPUBLISH_H_



namespace precitec
{

namespace interface
{


enum MotionMode {Pending = -1, Offline = 0, Position = 1, Position_Relative = 11, Position_Absolute = 12 , Velocity = 3, Home = 6};
enum ErrorCode {eErrorUpperLimit, eErrorLowerLimit, eErrorNotInRequestedMode};

enum HeadAxisID {eAxisX = 1, eAxisY = 2, eAxisZ = 3, eAxisTracker = 4, eAxisNone = 5};

enum AxisStatusBits {eHWLimitPos = 0x01, eHWLimitNeg = 0x02, eSWLimitPos = 0x04, eSWLimitNeg = 0x08, eOutOfRange = 0x10,
					 eBrakeOpen = 0x20, eHomePosNotOK = 0x40, eGenFault = 0x80};

enum DrivingType {eAbsoluteDriving, eRelativeDriving};

enum OutputID {eOutput1 = 1, eOutput2 = 2, eOutput3 = 3, eOutput4 = 4, eOutput5 = 5, eOutput6 = 6, eOutput7 = 7, eOutput8 = 8};

enum LEDPanelNo {LED_PANEL_1 = 0, LED_PANEL_2 = 1, LED_PANEL_3 = 2, LED_PANEL_4 = 3, LED_PANEL_5 = 4, LED_PANEL_6 = 5, LED_PANEL_7 = 6, LED_PANEL_8 = 7};

enum ScanmasterResultType {eScanmasterSeamWelding, eScanmasterScannerMoving, eScanmasterSeamWeldingAndEndOfSeamMarker, eScanmasterSpotWelding, eScanmasterPrepareContour, eScanmasterWeldPreparedContour};

struct HeadInfo{

	MotionMode status;
	short modeOfOperation;
	unsigned short statusWord;
	unsigned short errorCode;
	int positionUserUnit;
	int actVelocity;
	short actTorque;
	bool m_oHomingDirPos;
	bool m_oSoftLimitsActive;
	int m_oSoftUpperLimit;
	int m_oSoftLowerLimit;
	unsigned int m_oAxisStatusBits;
	unsigned short m_oScanWidthUMSent;
	int m_oScanWidthVoltSent;
	short m_oScanPosUMSent;
	int m_oScanPosVoltSent;
	bool m_oLineLaser1OnOff;
	short m_oLineLaser1Intensity;
	bool m_oLineLaser2OnOff;
	short m_oLineLaser2Intensity;
	bool m_oFieldLight1OnOff;
	short m_oFieldLight1Intensity;
	unsigned short m_oLEDChan1Para;
	unsigned short m_oLEDChan2Para;
	unsigned short m_oLEDChan3Para;
	unsigned short m_oLEDChan4Para;
	unsigned short m_oLEDChan1PulseWidth;
	unsigned short m_oLEDChan2PulseWidth;
	unsigned short m_oLEDChan3PulseWidth;
	unsigned short m_oLEDChan4PulseWidth;
};

struct TrackerInfo{
	bool ScannerOK;
	bool ScannerLimits;
};

} // namespace interface
}

#endif /*VIWELDHEADPUBLISH_H_*/

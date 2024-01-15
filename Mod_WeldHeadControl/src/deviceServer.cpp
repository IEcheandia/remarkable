/**
 * @file
 * @brief   DeviceServer von VI_WeldHeadControl
 *
 * @author  EA
 * @date    15.05.2013
 * @version 1.0
 */

#include <iostream>
#include <unistd.h>
#include <string>

#include "viWeldHead/deviceServer.h"

#define DEBUG_DEVICESERVER 0

namespace precitec
{

namespace ethercat
{

/**
 * @brief CTOR
 *
 */
DeviceServer::DeviceServer(WeldingHeadControl& p_rWeldingHeadControl) :
		m_rWeldingHeadControl(p_rWeldingHeadControl)
{
	//m_oTestStatus1 = false;
	//m_oTestStatus2 = false;
	//m_oTestValue1 = 0;
}

/**
 * @brief DTOR
 *
 */
DeviceServer::~DeviceServer()
{
}

int DeviceServer::initialize(Configuration const& config, int subDevice)
{
	return 0;
}

void DeviceServer::uninitialize()
{
}

void DeviceServer::reinitialize()
{
}

KeyHandle DeviceServer::set(SmpKeyValue keyValue, int subDevice)
{
	ValueTypes keyType = keyValue->type();

	switch( keyType )
	{
		case TBool:
        {
			if ( keyValue->key() == "LineLaser1OnOff" )
			{
				m_rWeldingHeadControl.setLineLaser1OnOff(keyValue->value<bool>());
				m_rWeldingHeadControl.SetLineLaser1Intensity();
			}
			if ( keyValue->key() == "LineLaser2OnOff" )
			{
				m_rWeldingHeadControl.setLineLaser2OnOff(keyValue->value<bool>());
				m_rWeldingHeadControl.SetLineLaser2Intensity();
			}
			if ( keyValue->key() == "FieldLight1OnOff" )
			{
				m_rWeldingHeadControl.setFieldLight1OnOff(keyValue->value<bool>());
				m_rWeldingHeadControl.SetFieldLight1Intensity();
			}
			if ( keyValue->key() == "LEDPanel1OnOff" )
			{
				m_rWeldingHeadControl.setLEDPanelOnOff(LED_PANEL_1, keyValue->value<bool>());
			}
			if ( keyValue->key() == "LEDPanel2OnOff" )
			{
				m_rWeldingHeadControl.setLEDPanelOnOff(LED_PANEL_2, keyValue->value<bool>());
			}
			if ( keyValue->key() == "LEDPanel3OnOff" )
			{
				m_rWeldingHeadControl.setLEDPanelOnOff(LED_PANEL_3, keyValue->value<bool>());
			}
			if ( keyValue->key() == "LEDPanel4OnOff" )
			{
				m_rWeldingHeadControl.setLEDPanelOnOff(LED_PANEL_4, keyValue->value<bool>());
			}
			if ( keyValue->key() == "LEDPanel5OnOff" )
			{
				m_rWeldingHeadControl.setLEDPanelOnOff(LED_PANEL_5, keyValue->value<bool>());
			}
			if ( keyValue->key() == "LEDPanel6OnOff" )
			{
				m_rWeldingHeadControl.setLEDPanelOnOff(LED_PANEL_6, keyValue->value<bool>());
			}
			if ( keyValue->key() == "LEDPanel7OnOff" )
			{
				m_rWeldingHeadControl.setLEDPanelOnOff(LED_PANEL_7, keyValue->value<bool>());
			}
			if ( keyValue->key() == "LEDPanel8OnOff" )
			{
				m_rWeldingHeadControl.setLEDPanelOnOff(LED_PANEL_8, keyValue->value<bool>());
			}
			if ( keyValue->key() == "LED_SaveIntensityPersistent" )
			{
				m_rWeldingHeadControl.LED_SaveIntensityPersistent();
			}

			if ( keyValue->key() == "ScanTrackerExpertMode" )
			{
				m_rWeldingHeadControl.SetTrackerExpertMode( keyValue->value<bool>() );
			}
			if ( keyValue->key() == "TrackerDriverOnOff" )
			{
				m_rWeldingHeadControl.SetTrackerDriverOnOff( keyValue->value<bool>() );
			}
			if ( keyValue->key() == "ScanWidthOutOfGapWidth" )
			{
				m_rWeldingHeadControl.SetScanWidthOutOfGapWidth( keyValue->value<bool>() );
			}
			if ( keyValue->key() == "ScanPosOutOfGapPos" )
			{
				m_rWeldingHeadControl.SetScanPosOutOfGapPos( keyValue->value<bool>() );
			}
			if ( keyValue->key() == "ScanTrackerAskStatus" )
			{
				if (m_rWeldingHeadControl.getSerialToTracker() != NULL)
				{
					m_rWeldingHeadControl.getSerialToTracker()->trackerCmdShow();
				}
			}
			if ( keyValue->key() == "ScanTrackerAskRevisions" )
			{
				if (m_rWeldingHeadControl.getSerialToTracker() != NULL)
				{
					m_rWeldingHeadControl.getSerialToTracker()->trackerCmdRevisions();
				}
			}
			if ( keyValue->key() == "ScanTrackerAskSerialNumbers" )
			{
				if (m_rWeldingHeadControl.getSerialToTracker() != NULL)
				{
					m_rWeldingHeadControl.getSerialToTracker()->trackerCmdShowSerial();
				}
			}
			if ( keyValue->key() == "TestFunctionTracker4" )
			{
				if (m_rWeldingHeadControl.getSerialToTracker() != NULL)
				{
					m_rWeldingHeadControl.getSerialToTracker()->trackerCmdLimitTable();
				}
			}
			if ( keyValue->key() == "TestFunctionTracker5" )
			{
				if (m_rWeldingHeadControl.getSerialToTracker() != NULL)
				{
					m_rWeldingHeadControl.getSerialToTracker()->trackerCmdHelp();
				}
			}
			if ( keyValue->key() == "Z_Collimator_Homing" )
			{
				m_rWeldingHeadControl.doZCollHoming();
			}
			if ( keyValue->key() == "Z_Collimator_CenterPosition" )
			{
				m_rWeldingHeadControl.doZCollDriving(eRelativeDriving, 0);
			}
			if ( keyValue->key() == "X_Axis_SoftLimitsOnOff" )
			{
				if (m_rWeldingHeadControl.getStatesHeadX() != NULL)
				{
					m_rWeldingHeadControl.getStatesHeadX()->SetSoftLimitsActive( keyValue->value<bool>() );
				}
			}
			if ( keyValue->key() == "X_Axis_HomingDirectionPositive" )
			{
				if (m_rWeldingHeadControl.getStatesHeadX() != NULL)
				{
					if (keyValue->value<bool>()) // Homing Richtung positiv
					{
					    m_rWeldingHeadControl.getStatesHeadX()->SetAxisHomingDirPos( true ); // Homing Richtung positiv
					    m_rWeldingHeadControl.getStatesHeadX()->SetAxisHomeOffset( -100 ); // 0.1 mm zurueck
					}
					else // Homing Richtung negativ
					{
					    m_rWeldingHeadControl.getStatesHeadX()->SetAxisHomingDirPos( false ); // Homing Richtung negativ
					    m_rWeldingHeadControl.getStatesHeadX()->SetAxisHomeOffset( 100 ); // 0.1 mm zurueck
					}
				}
			}
			if ( keyValue->key() == "Y_Axis_SoftLimitsOnOff" )
			{
				if (m_rWeldingHeadControl.getStatesHeadY() != NULL)
				{
					m_rWeldingHeadControl.getStatesHeadY()->SetSoftLimitsActive( keyValue->value<bool>() );
				}
			}
			if ( keyValue->key() == "Y_Axis_HomingDirectionPositive" )
			{
				if (m_rWeldingHeadControl.getStatesHeadY() != NULL)
				{
					if (keyValue->value<bool>()) // Homing Richtung positiv
					{
					    m_rWeldingHeadControl.getStatesHeadY()->SetAxisHomingDirPos( true ); // Homing Richtung positiv
					    m_rWeldingHeadControl.getStatesHeadY()->SetAxisHomeOffset( -100 ); // 0.1 mm zurueck
					}
					else // Homing Richtung negativ
					{
					    m_rWeldingHeadControl.getStatesHeadY()->SetAxisHomingDirPos( false ); // Homing Richtung negativ
					    m_rWeldingHeadControl.getStatesHeadY()->SetAxisHomeOffset( 100 ); // 0.1 mm zurueck
					}
				}
			}
            if ( keyValue->key() == "TestFunctionAxisY_1" )
            {
                if (m_rWeldingHeadControl.getStatesHeadY() != NULL)
                {
                    m_rWeldingHeadControl.getStatesHeadY()->TestFunctionAxis_1();
                }
            }
            if ( keyValue->key() == "TestFunctionAxisY_2" )
            {
                if (m_rWeldingHeadControl.getStatesHeadY() != NULL)
                {
                    m_rWeldingHeadControl.getStatesHeadY()->TestFunctionAxis_2();
                }
            }
            if ( keyValue->key() == "TestFunctionAxisY_3" )
            {
                if (m_rWeldingHeadControl.getStatesHeadY() != NULL)
                {
                    m_rWeldingHeadControl.getStatesHeadY()->TestFunctionAxis_3();
                }
            }
            if ( keyValue->key() == "TestFunctionAxisY_4" )
            {
                if (m_rWeldingHeadControl.getStatesHeadY() != NULL)
                {
                    m_rWeldingHeadControl.getStatesHeadY()->TestFunctionAxis_4();
                }
            }
            if ( keyValue->key() == "TestFunctionAxisY_5" )
            {
                if (m_rWeldingHeadControl.getStatesHeadY() != NULL)
                {
                    m_rWeldingHeadControl.getStatesHeadY()->TestFunctionAxis_5();
                }
            }
			if ( keyValue->key() == "Z_Axis_SoftLimitsOnOff" )
			{
				if (m_rWeldingHeadControl.getStatesHeadZ() != NULL)
				{
					m_rWeldingHeadControl.getStatesHeadZ()->SetSoftLimitsActive( keyValue->value<bool>() );
				}
			}
			if ( keyValue->key() == "Z_Axis_HomingDirectionPositive" )
			{
				if (m_rWeldingHeadControl.getStatesHeadZ() != NULL)
				{
					if (keyValue->value<bool>() == 0) // Homing Richtung positiv
					{
					    m_rWeldingHeadControl.getStatesHeadZ()->SetAxisHomingDirPos( true ); // Homing Richtung positiv
					    m_rWeldingHeadControl.getStatesHeadZ()->SetAxisHomeOffset( -100 ); // 0.1 mm zurueck
					}
					else // Homing Richtung negativ
					{
					    m_rWeldingHeadControl.getStatesHeadZ()->SetAxisHomingDirPos( false ); // Homing Richtung negativ
					    m_rWeldingHeadControl.getStatesHeadZ()->SetAxisHomeOffset( 100 ); // 0.1 mm zurueck
					}
				}
			}
            if ( keyValue->key() == "DebugInfo_AxisController" )
            {
                m_rWeldingHeadControl.setDebugInfo_AxisController(keyValue->value<bool>());
            }
			if ( keyValue->key() == "ClearEncoderCounter1" )
			{
				m_rWeldingHeadControl.ClearEncoderCounter(eEncoderInputNo1);
			}
			if ( keyValue->key() == "ClearEncoderCounter2" )
			{
				m_rWeldingHeadControl.ClearEncoderCounter(eEncoderInputNo2);
			}

			if ( keyValue->key() == "LC_Send_Data" )
			{
				if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
				{
					m_rWeldingHeadControl.getDataToLaserControl()->sendDataBlockToLC();
				}
			}
			if ( keyValue->key() == "LC_Start_Processing" )
			{
				m_rWeldingHeadControl.SetLCStartSignal( keyValue->value<bool>() );
			}

            if ( keyValue->key() == "Scanner_DriveToZero" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->ScannerDriveToZero();
                }
            }
            if ( keyValue->key() == "Scanner_DriveToPosition" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->ScannerDriveToPosition();
                }
            }
            if ( keyValue->key() == "Scanner_DriveWithOCTReference" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->ScannerDriveWithOCTReference();
                }
            }
            if ( keyValue->key() == "Scanner_SetOCTReference" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->ScannerSetOCTReference();
                }
            }
            if ( keyValue->key() == "Scanner_StartWeldingPreview" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->ScannerStartWeldingPreview();
                }
            }
            if ( keyValue->key() == "Scanner_StopWeldingPreview" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->ScannerStopWeldingPreview();
                }
            }
            if ( keyValue->key() == "Scanner_TestFunction1" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->ScannerTestFunction1();
                }
            }
            if ( keyValue->key() == "Scanner_TestFunction2" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->ScannerTestFunction2();
                }
            }
            if (keyValue->key() == "ScanTracker2D_CustomFigure")
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->setScanTracker2DCustomFigure(keyValue->value<bool>());
                }
            }
            if ( keyValue->key() == "LEDSendData" )
            {
                m_rWeldingHeadControl.sendLEDData();
            }
            if ( keyValue->key() == "IsLaserPowerDigital" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->SetLaserPowerIsDigital(keyValue->value<bool>() );
                }
            }
            if ( keyValue->key() == "SaveLoggedScannerData")
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->SaveLoggedScannerData();
                }
            }
            if (keyValue->key() == "Generate_ScanTracker2D_Figure")
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->generateScantracker2DList();
                }
            }
            if ( keyValue->key() == "ScannerDebugMode")
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->SetEnableDebugMode(keyValue->value<bool>() );
                }
            }
            if ( keyValue->key() == "ScannerCompensateHeight")
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->setScannerCompensateHeight(keyValue->value<bool>() );
                }
            }
            if ( keyValue->key() == "IsCompensationHeightFixed")
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->setIsCompensateHeightFixed(keyValue->value<bool>() );
                }
            }

#if DEBUG_DEVICESERVER
			printf("set %s: %d\n", keyValue->key().c_str(), keyValue->value<bool>());
#endif
			break;
        }

		case TInt:
        {
			if ( keyValue->key() == "LineLaser1Intensity" )
			{
				m_rWeldingHeadControl.setLineLaser1Intensity(keyValue->value<int>());
				m_rWeldingHeadControl.SetLineLaser1Intensity();
			}
			if ( keyValue->key() == "LineLaser2Intensity" )
			{
				m_rWeldingHeadControl.setLineLaser2Intensity(keyValue->value<int>());
				m_rWeldingHeadControl.SetLineLaser2Intensity();
			}
			if ( keyValue->key() == "FieldLight1Intensity" )
			{
				m_rWeldingHeadControl.setFieldLight1Intensity(keyValue->value<int>());
				m_rWeldingHeadControl.SetFieldLight1Intensity();
			}
			if ( keyValue->key() == "LEDPanel1Intensity" )
			{
				m_rWeldingHeadControl.setLEDPanelIntensity(LED_PANEL_1, keyValue->value<int>());
			}
			if ( keyValue->key() == "LEDPanel2Intensity" )
			{
				m_rWeldingHeadControl.setLEDPanelIntensity(LED_PANEL_2, keyValue->value<int>());
			}
			if ( keyValue->key() == "LEDPanel3Intensity" )
			{
				m_rWeldingHeadControl.setLEDPanelIntensity(LED_PANEL_3, keyValue->value<int>());
			}
			if ( keyValue->key() == "LEDPanel4Intensity" )
			{
				m_rWeldingHeadControl.setLEDPanelIntensity(LED_PANEL_4, keyValue->value<int>());
			}
			if ( keyValue->key() == "LEDPanel5Intensity" )
			{
				m_rWeldingHeadControl.setLEDPanelIntensity(LED_PANEL_5, keyValue->value<int>());
			}
			if ( keyValue->key() == "LEDPanel6Intensity" )
			{
				m_rWeldingHeadControl.setLEDPanelIntensity(LED_PANEL_6, keyValue->value<int>());
			}
			if ( keyValue->key() == "LEDPanel7Intensity" )
			{
				m_rWeldingHeadControl.setLEDPanelIntensity(LED_PANEL_7, keyValue->value<int>());
			}
			if ( keyValue->key() == "LEDPanel8Intensity" )
			{
				m_rWeldingHeadControl.setLEDPanelIntensity(LED_PANEL_8, keyValue->value<int>());
			}
			if ( keyValue->key() == "LEDPanel1PulseWidth" )
			{
				m_rWeldingHeadControl.setLEDPanelPulseWidth(LED_PANEL_1, keyValue->value<int>());
			}
			if ( keyValue->key() == "LEDPanel2PulseWidth" )
			{
				m_rWeldingHeadControl.setLEDPanelPulseWidth(LED_PANEL_2, keyValue->value<int>());
			}
			if ( keyValue->key() == "LEDPanel3PulseWidth" )
			{
				m_rWeldingHeadControl.setLEDPanelPulseWidth(LED_PANEL_3, keyValue->value<int>());
			}
			if ( keyValue->key() == "LEDPanel4PulseWidth" )
			{
				m_rWeldingHeadControl.setLEDPanelPulseWidth(LED_PANEL_4, keyValue->value<int>());
			}
			if ( keyValue->key() == "LEDPanel5PulseWidth" )
			{
				m_rWeldingHeadControl.setLEDPanelPulseWidth(LED_PANEL_5, keyValue->value<int>());
			}
			if ( keyValue->key() == "LEDPanel6PulseWidth" )
			{
				m_rWeldingHeadControl.setLEDPanelPulseWidth(LED_PANEL_6, keyValue->value<int>());
			}
			if ( keyValue->key() == "LEDPanel7PulseWidth" )
			{
				m_rWeldingHeadControl.setLEDPanelPulseWidth(LED_PANEL_7, keyValue->value<int>());
			}
			if ( keyValue->key() == "LEDPanel8PulseWidth" )
			{
				m_rWeldingHeadControl.setLEDPanelPulseWidth(LED_PANEL_8, keyValue->value<int>());
			}
			if ( keyValue->key() == "FocalLength" )
			{
				m_rWeldingHeadControl.SetFocalLength(keyValue->value<int>());
			}
			if ( keyValue->key() == "ScanTrackerFrequency" )
			{
				m_rWeldingHeadControl.SetTrackerFrequencyStep( static_cast<TrackerFrequencyStep>(keyValue->value<int>() ) );
			}
			if ( keyValue->key() == "ScanTrackerFrequencyContinuously" )
			{
				m_rWeldingHeadControl.SetTrackerFrequencyCont( static_cast<unsigned short>(keyValue->value<int>() ) );
			}
			if ( keyValue->key() == "ScanWidthFixed" )
			{
				m_rWeldingHeadControl.SetTrackerScanWidthFixed(keyValue->value<int>());
			}
			if ( keyValue->key() == "GapWidthToScanWidthOffset" )
			{
				m_rWeldingHeadControl.SetGapWidthToScanWidthOffset(keyValue->value<int>());
			}
			if ( keyValue->key() == "GapWidthToScanWidthGradient" )
			{
				m_rWeldingHeadControl.SetGapWidthToScanWidthGradient(keyValue->value<int>());
			}
			if ( keyValue->key() == "ScanPosFixed" )
			{
				m_rWeldingHeadControl.SetTrackerScanPosFixed(keyValue->value<int>());
			}
			if ( keyValue->key() == "TrackerMaxAmplitude" )
			{
				m_rWeldingHeadControl.SetTrackerMaxAmplitude(keyValue->value<int>());
			}
			if ( keyValue->key() == "Z_Collimator_PositionAbsolute" )
			{
				m_rWeldingHeadControl.doZCollDriving(eAbsoluteDriving, keyValue->value<int>());
			}
            if ( keyValue->key() == "Z_Collimator_ActualPosition_um" )
            {
                // only to ask position, no set functionality
            }
			if ( keyValue->key() == "X_Axis_AbsolutePosition" )
			{
				m_rWeldingHeadControl.SetHeadValue(interface::eAxisX, keyValue->value<int>(), Position_Absolute);
			}
			if ( keyValue->key() == "X_Axis_SetUpperLimit" )
			{
				if (m_rWeldingHeadControl.getStatesHeadX() != NULL)
				{
					m_rWeldingHeadControl.getStatesHeadX()->SetUpperLimit( keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "X_Axis_SetLowerLimit" )
			{
				if (m_rWeldingHeadControl.getStatesHeadX() != NULL)
				{
					m_rWeldingHeadControl.getStatesHeadX()->SetLowerLimit( keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "X_Axis_Velocity" )
			{
				if (m_rWeldingHeadControl.getStatesHeadX() != NULL)
				{
					m_rWeldingHeadControl.getStatesHeadX()->SetAxisVelocity( keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "X_Axis_Acceleration" )
			{
				if (m_rWeldingHeadControl.getStatesHeadX() != NULL)
				{
					m_rWeldingHeadControl.getStatesHeadX()->SetAxisAcceleration( keyValue->value<int>() );
					m_rWeldingHeadControl.getStatesHeadX()->SetAxisDeceleration( keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "Y_Axis_AbsolutePosition" )
			{
				m_rWeldingHeadControl.SetHeadValue(interface::eAxisY, keyValue->value<int>(), Position_Absolute);
			}
			if ( keyValue->key() == "Y_Axis_SetUpperLimit" )
			{
				if (m_rWeldingHeadControl.getStatesHeadY() != NULL)
				{
					m_rWeldingHeadControl.getStatesHeadY()->SetUpperLimit( keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "Y_Axis_SetLowerLimit" )
			{
				if (m_rWeldingHeadControl.getStatesHeadY() != NULL)
				{
					m_rWeldingHeadControl.getStatesHeadY()->SetLowerLimit( keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "Y_Axis_Velocity" )
			{
				if (m_rWeldingHeadControl.getStatesHeadY() != NULL)
				{
					m_rWeldingHeadControl.getStatesHeadY()->SetAxisVelocity( keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "Y_Axis_Acceleration" )
			{
				if (m_rWeldingHeadControl.getStatesHeadY() != NULL)
				{
					m_rWeldingHeadControl.getStatesHeadY()->SetAxisAcceleration( keyValue->value<int>() );
					m_rWeldingHeadControl.getStatesHeadY()->SetAxisDeceleration( keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "Z_Axis_AbsolutePosition" )
			{
				m_rWeldingHeadControl.SetHeadValue(interface::eAxisZ, keyValue->value<int>(), Position_Absolute);
			}
			if ( keyValue->key() == "Z_Axis_SetUpperLimit" )
			{
				if (m_rWeldingHeadControl.getStatesHeadZ() != NULL)
				{
					m_rWeldingHeadControl.getStatesHeadZ()->SetUpperLimit( keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "Z_Axis_SetLowerLimit" )
			{
				if (m_rWeldingHeadControl.getStatesHeadZ() != NULL)
				{
					m_rWeldingHeadControl.getStatesHeadZ()->SetLowerLimit( keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "Z_Axis_Velocity" )
			{
				if (m_rWeldingHeadControl.getStatesHeadZ() != NULL)
				{
					m_rWeldingHeadControl.getStatesHeadZ()->SetAxisVelocity( keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "Z_Axis_Acceleration" )
			{
				if (m_rWeldingHeadControl.getStatesHeadZ() != NULL)
				{
					m_rWeldingHeadControl.getStatesHeadZ()->SetAxisAcceleration( keyValue->value<int>() );
					m_rWeldingHeadControl.getStatesHeadZ()->SetAxisDeceleration( keyValue->value<int>() );
				}
			}

			// Data regarding LaserControl
			if ( keyValue->key() == "LC_Parameter_No1" )
			{
				if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
				{
					m_rWeldingHeadControl.getDataToLaserControl()->setDataVariable( eLCVarIdx1, keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "LC_Parameter_No2" )
			{
				if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
				{
					m_rWeldingHeadControl.getDataToLaserControl()->setDataVariable( eLCVarIdx2, keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "LC_Parameter_No3" )
			{
				if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
				{
					m_rWeldingHeadControl.getDataToLaserControl()->setDataVariable( eLCVarIdx3, keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "LC_Parameter_No4" )
			{
				if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
				{
					m_rWeldingHeadControl.getDataToLaserControl()->setDataVariable( eLCVarIdx4, keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "LC_Parameter_No5" )
			{
				if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
				{
					m_rWeldingHeadControl.getDataToLaserControl()->setDataVariable( eLCVarIdx5, keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "LC_Parameter_No6" )
			{
				if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
				{
					m_rWeldingHeadControl.getDataToLaserControl()->setDataVariable( eLCVarIdx6, keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "LC_Parameter_No7" )
			{
				if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
				{
					m_rWeldingHeadControl.getDataToLaserControl()->setDataVariable( eLCVarIdx7, keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "LC_Parameter_No8" )
			{
				if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
				{
					m_rWeldingHeadControl.getDataToLaserControl()->setDataVariable( eLCVarIdx8, keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "LC_Parameter_No9" )
			{
				if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
				{
					m_rWeldingHeadControl.getDataToLaserControl()->setDataVariable( eLCVarIdx9, keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "LC_Parameter_No10" )
			{
				if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
				{
					m_rWeldingHeadControl.getDataToLaserControl()->setDataVariable( eLCVarIdx10, keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "LC_Parameter_No11" )
			{
				if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
				{
					m_rWeldingHeadControl.getDataToLaserControl()->setDataVariable( eLCVarIdx11, keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "LC_Parameter_No12" )
			{
				if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
				{
					m_rWeldingHeadControl.getDataToLaserControl()->setDataVariable( eLCVarIdx12, keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "LC_Parameter_No13" )
			{
				if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
				{
					m_rWeldingHeadControl.getDataToLaserControl()->setDataVariable( eLCVarIdx13, keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "LC_Parameter_No14" )
			{
				if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
				{
					m_rWeldingHeadControl.getDataToLaserControl()->setDataVariable( eLCVarIdx14, keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "LC_Parameter_No15" )
			{
				if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
				{
					m_rWeldingHeadControl.getDataToLaserControl()->setDataVariable( eLCVarIdx15, keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "LC_Parameter_No16" )
			{
				if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
				{
					m_rWeldingHeadControl.getDataToLaserControl()->setDataVariable( eLCVarIdx16, keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "LC_Parameter_No17" )
			{
				if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
				{
					m_rWeldingHeadControl.getDataToLaserControl()->setDataVariable( eLCVarIdx17, keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "LC_Parameter_No18" )
			{
				if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
				{
					m_rWeldingHeadControl.getDataToLaserControl()->setDataVariable( eLCVarIdx18, keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "LC_Parameter_No19" )
			{
				if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
				{
					m_rWeldingHeadControl.getDataToLaserControl()->setDataVariable( eLCVarIdx19, keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "LC_Parameter_No20" )
			{
				if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
				{
					m_rWeldingHeadControl.getDataToLaserControl()->setDataVariable( eLCVarIdx20, keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "LC_Parameter_No21" )
			{
				if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
				{
					m_rWeldingHeadControl.getDataToLaserControl()->setDataVariable( eLCVarIdx21, keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "LC_Parameter_No22" )
			{
				if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
				{
					m_rWeldingHeadControl.getDataToLaserControl()->setDataVariable( eLCVarIdx22, keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "LC_Parameter_No23" )
			{
				if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
				{
					m_rWeldingHeadControl.getDataToLaserControl()->setDataVariable( eLCVarIdx23, keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "LC_Parameter_No24" )
			{
				if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
				{
					m_rWeldingHeadControl.getDataToLaserControl()->setDataVariable( eLCVarIdx24, keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "LC_Parameter_No25" )
			{
				if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
				{
					m_rWeldingHeadControl.getDataToLaserControl()->setDataVariable( eLCVarIdx25, keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "LC_Parameter_No26" )
			{
				if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
				{
					m_rWeldingHeadControl.getDataToLaserControl()->setDataVariable( eLCVarIdx26, keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "LC_Parameter_No27" )
			{
				if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
				{
					m_rWeldingHeadControl.getDataToLaserControl()->setDataVariable( eLCVarIdx27, keyValue->value<int>() );
				}
			}
			if ( keyValue->key() == "LC_Parameter_No28" )
			{
				if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
				{
					m_rWeldingHeadControl.getDataToLaserControl()->setDataVariable( eLCVarIdx28, keyValue->value<int>() );
				}
			}

            if ( keyValue->key() == "Scanner_LaserOnDelay_us" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->SetScannerLaserOnDelay(keyValue->value<int>() );
                }
            }
            if ( keyValue->key() == "Scanner_LaserOffDelay_us" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->SetScannerLaserOffDelay(keyValue->value<int>() );
                }
            }
            if (keyValue->key() == "ScanTracker2D_LaserDelay")
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->setScanTracker2DLaserDelay(keyValue->value<int>());
                }
            }
            if ( keyValue->key() == "Scanner_LaserPowerStatic" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->SetLaserPowerStatic(keyValue->value<int>() );
                }
            }
            if ( keyValue->key() == "Scanner_LaserPowerStaticRing" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->SetLaserPowerStaticRing(keyValue->value<int>() );
                }
            }
            auto oLaserPowerParameter = keyValue->key().find("Scanner_LaserPower_Parameter");
            if (oLaserPowerParameter != std::string::npos)
            {
                std::string oParaNumber = keyValue->key().substr(28);
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->SetLaserPowerParameter(static_cast<LaserPowerParameter>(std::stoi(oParaNumber) - 1), keyValue->value<int>() );
                }
            }
            /*
             * Scanner_WeldingFigureNumber gives you the correct file with the number for getting the correct welding figure
             */
            if ( keyValue->key() == "Scanner_WeldingFigureNumber" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->SetWeldingFigureNumber(keyValue->value<int>() );
                }
            }
            /*
             * Scanner_FileNumber gives you the correct file with the number for getting the correct wobble figure
             */
            if ( keyValue->key() == "Scanner_FileNumber" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->SetWobbelFigureNumber(keyValue->value<int>() );
                }
            }
            if ( keyValue->key() == "Scanner_Wobble_Frequency" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->SetScannerWobbleFrequency(keyValue->value<int>());
                }
            }
            if ( keyValue->key() == "Scanner_Wobble_Mode" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->SetWobbleMode(keyValue->value<int>());
                }
            }
            if ( keyValue->key() == "OCT_Reference_Arm" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->SetOCTReferenceArm(keyValue->value<int>());
                }
            }

            // parameters regarding LWM40
            if ( keyValue->key() == "LWM40_No1_AmpPlasma" )
            {
                m_rWeldingHeadControl.SetLWM40_No1_AmpPlasma( keyValue->value<int>() );
            }
            if ( keyValue->key() == "LWM40_No1_AmpTemperature" )
            {
                m_rWeldingHeadControl.SetLWM40_No1_AmpTemperature( keyValue->value<int>() );
            }
            if ( keyValue->key() == "LWM40_No1_AmpBackReflection" )
            {
                m_rWeldingHeadControl.SetLWM40_No1_AmpBackReflection( keyValue->value<int>() );
            }
            if ( keyValue->key() == "LWM40_No1_AmpAnalogInput" )
            {
                m_rWeldingHeadControl.SetLWM40_No1_AmpAnalogInput( keyValue->value<int>() );
            }
            if ( keyValue->key() == "DebugModePeriod" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->SetMeasurementPeriod( keyValue->value<int>() );
                }
            }
            for (std::size_t i = 0u; i < 4u; i++)
            {
                const std::string loggerSignal = std::string{"ScannerLoggerSignal"} + std::to_string(i + 1);
                if (keyValue->key() == loggerSignal)
                {
                    if (m_rWeldingHeadControl.getScanlab() != nullptr)
                    {
                        m_rWeldingHeadControl.getScanlab()->setLoggedSignal(i, keyValue->value<int>());
                    }
                }
            }
            if ( keyValue->key() == "StatusSignalHead1Axis1")
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->setStatusSignalHead1Axis(0, keyValue->value<int>() );
                }
            }
            if ( keyValue->key() == "StatusSignalHead1Axis2")
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->setStatusSignalHead1Axis(1, keyValue->value<int>() );
                }
            }
            if ( keyValue->key() == "PositionDifferenceTolerance" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->setPositionDifferenceTolerance(keyValue->value<int>());
                }
            }
            if (keyValue->key() == "Laser_Power_Compensation_10us" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->setLaserPowerDelayCompensation(keyValue->value<int>());
                    m_rWeldingHeadControl.saveKeyValueToWeldHeadDefaults(keyValue->key(), keyValue->value<int>());
                }
            }
            if (keyValue->key() == "CorrectionFileMode")
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->setCorrectionFileMode(keyValue->value<int>());
                }
            }

#if DEBUG_DEVICESERVER
			printf("set %s: %d\n", keyValue->key().c_str(), keyValue->value<int>());
#endif
			break;
        }

		case TDouble:
        {
			if ( keyValue->key() == "Z_Collimator_SystemOffset" )
			{
				m_rWeldingHeadControl.SetZCSystemOffset(keyValue->value<double>());
			}
			if ( keyValue->key() == "Z_Collimator_LensToFocusRatio" )
			{
				m_rWeldingHeadControl.SetZCLensToFocusRatio(keyValue->value<double>());
			}
            if (keyValue->key() == "LiquidLensPosition")
            {
                m_rWeldingHeadControl.setLiquidLensPosition(keyValue->value<double>());
            }

            if ( keyValue->key() == "Scanner_New_X_Position" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->SetScannerNewXPosition(keyValue->value<double>());
                }
            }
            if ( keyValue->key() == "Scanner_New_Y_Position" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->SetScannerNewYPosition(keyValue->value<double>());
                }
            }
            if ( keyValue->key() == "Scanner_Actual_X_Position" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->SetScannerActXPosition(keyValue->value<double>());
                }
            }
            if ( keyValue->key() == "Scanner_Actual_Y_Position" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->SetScannerActYPosition(keyValue->value<double>());
                }
            }
            if ( keyValue->key() == "Scanner_Jump_Speed" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->SetScannerJumpSpeed(keyValue->value<double>());
                }
            }
            if ( keyValue->key() == "Scanner_Mark_Speed" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->SetScannerMarkSpeed(keyValue->value<double>());
                }
            }
            if ( keyValue->key() == "Scanner_Wobble_X_Size" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->SetScannerWobbleXSize(keyValue->value<double>());
                }
            }
            if ( keyValue->key() == "Scanner_Wobble_Y_Size" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->SetScannerWobbleYSize(keyValue->value<double>());
                }
            }
            if ( keyValue->key() == "Scanner_Wobble_Radius" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->SetScannerWobbleRadius(keyValue->value<double>());
                }
            }
            if ( keyValue->key() == "LaserDelay" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->SetLaserDelay(keyValue->value<double>() );
                }
            }
            if ( keyValue->key() == "PositionDifferenceToleranceMillimeter" )
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->setPositionDifferenceTolerance(keyValue->value<double>());
                }
            }
            if (keyValue->key() == "ScanTracker2D_Angle")
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->setScanTracker2DAngle(keyValue->value<double>());
                }
            }
            if (keyValue->key() == "ScanTracker2D_ScanWidthFixed_X")
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->setScanTracker2DScanWidthFixedX(keyValue->value<double>());
                }
            }
            if (keyValue->key() == "ScanTracker2D_ScanWidthFixed_Y")
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->setScanTracker2DScanWidthFixedY(keyValue->value<double>());
                }
            }
            if (keyValue->key() == "ScanTracker2D_ScanPosFixed_X")
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->setScanTracker2DScanPosFixedX(keyValue->value<double>());
                }
            }
            if (keyValue->key() == "ScanTracker2D_ScanPosFixed_Y")
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->setScanTracker2DScanPosFixedY(keyValue->value<double>());
                }
            }
            if (keyValue->key() == "CompensationHeight")
            {
                if (m_rWeldingHeadControl.getScanlab() != nullptr)
                {
                    m_rWeldingHeadControl.getScanlab()->setCompensationHeight(keyValue->value<double>());
                }
            }

#if DEBUG_DEVICESERVER
			printf("set %s: %d\n", keyValue->key().c_str(), keyValue->value<double>());
#endif
			break;
        }

		default:
			break;
	}
	return KeyHandle();
}

void DeviceServer::set(Configuration config, int subDevice)
{
    for (const auto &keyValue : config)
    {
        set(keyValue, subDevice);
    }
}

SmpKeyValue DeviceServer::get(Key key, int subDevice)
{
#if DEBUG_DEVICESERVER
	printf("get %s\n", key.c_str());
#endif

	// Data regarding X-Axis
	if ( key == "X_Axis_Enabled")
	{
		return SmpKeyValue(new TKeyValue<bool>("X_Axis_Enabled", m_rWeldingHeadControl.isAxisXEnabled(), false, true, false) );
	}
	if ( key == "X_Axis_AbsolutePosition")
	{
		return SmpKeyValue(new TKeyValue<int>("X_Axis_AbsolutePosition", m_rWeldingHeadControl.getRequiredPositionX(), -100000, 100000, 1000) );
	}
	if ( key == "X_Axis_SoftLimitsOnOff")
	{
		bool OnOff = false;
		if (m_rWeldingHeadControl.getStatesHeadX() != NULL)
		{
			OnOff = m_rWeldingHeadControl.getStatesHeadX()->GetSoftLimitsActive();
		}
		return SmpKeyValue(new TKeyValue<bool>("X_Axis_SoftLimitsOnOff", OnOff, false, true, true) );
	}
	if ( key == "X_Axis_SetUpperLimit")
	{
		int upper = 30000;
		if (m_rWeldingHeadControl.getStatesHeadX() != NULL)
		{
			upper = m_rWeldingHeadControl.getStatesHeadX()->GetUpperLimit();
		}
		return SmpKeyValue(new TKeyValue<int>("X_Axis_SetUpperLimit", upper, -100000, 100000, 30000) );
	}
	if ( key == "X_Axis_SetLowerLimit")
	{
		int lower = 1000;
		if (m_rWeldingHeadControl.getStatesHeadX() != NULL)
		{
			lower = m_rWeldingHeadControl.getStatesHeadX()->GetLowerLimit();
		}
		return SmpKeyValue(new TKeyValue<int>("X_Axis_SetLowerLimit", lower, -100000, 100000, 1000) );
	}
	if ( key == "X_Axis_Velocity")
	{
		int value = 100;
		if (m_rWeldingHeadControl.getStatesHeadX() != NULL)
		{
			value = m_rWeldingHeadControl.getStatesHeadX()->GetAxisVelocity();
		}
		return SmpKeyValue(new TKeyValue<int>("X_Axis_Velocity", value, 1, 100, 100) );
	}
	if ( key == "X_Axis_Acceleration")
	{
		int value = 100;
		if (m_rWeldingHeadControl.getStatesHeadX() != NULL)
		{
			value = m_rWeldingHeadControl.getStatesHeadX()->GetAxisAcceleration();
		}
		return SmpKeyValue(new TKeyValue<int>("X_Axis_Acceleration", value, 1, 100, 100) );
	}
	if ( key == "X_Axis_HomingDirectionPositive")
	{
		bool value = true; // Homing Richtung positiv
		if (m_rWeldingHeadControl.getStatesHeadX() != NULL)
		{
			if (m_rWeldingHeadControl.getStatesHeadX()->GetAxisHomingDirPos()) // Homing Richtung positiv
				value = true; // Homing Richtung positiv
			else
				value = false; // Homing Richtung negativ
		}
		return SmpKeyValue(new TKeyValue<bool>("X_Axis_HomingDirectionPositive", value, false, true, true) );
	}

	// Data regarding Y-Axis
	if ( key == "Y_Axis_Enabled")
	{
		return SmpKeyValue(new TKeyValue<bool>("Y_Axis_Enabled", m_rWeldingHeadControl.isAxisYEnabled(), false, true, false) );
	}
	if ( key == "Y_Axis_AbsolutePosition")
	{
		return SmpKeyValue(new TKeyValue<int>("Y_Axis_AbsolutePosition", m_rWeldingHeadControl.getRequiredPositionY(), -100000, 100000, 1000) );
	}
	if ( key == "Y_Axis_SoftLimitsOnOff")
	{
		bool OnOff = false;
		if (m_rWeldingHeadControl.getStatesHeadY() != NULL)
		{
			OnOff = m_rWeldingHeadControl.getStatesHeadY()->GetSoftLimitsActive();
		}
		return SmpKeyValue(new TKeyValue<bool>("Y_Axis_SoftLimitsOnOff", OnOff, false, true, true) );
	}
	if ( key == "Y_Axis_SetUpperLimit")
	{
		int upper = 30000;
		if (m_rWeldingHeadControl.getStatesHeadY() != NULL)
		{
			upper = m_rWeldingHeadControl.getStatesHeadY()->GetUpperLimit();
		}
		return SmpKeyValue(new TKeyValue<int>("Y_Axis_SetUpperLimit", upper, -100000, 100000, 30000) );
	}
	if ( key == "Y_Axis_SetLowerLimit")
	{
		int lower = 1000;
		if (m_rWeldingHeadControl.getStatesHeadY() != NULL)
		{
			lower = m_rWeldingHeadControl.getStatesHeadY()->GetLowerLimit();
		}
		return SmpKeyValue(new TKeyValue<int>("Y_Axis_SetLowerLimit", lower, -100000, 100000, 1000) );
	}
	if ( key == "Y_Axis_Velocity")
	{
		int value = 100;
		if (m_rWeldingHeadControl.getStatesHeadY() != NULL)
		{
			value = m_rWeldingHeadControl.getStatesHeadY()->GetAxisVelocity();
		}
		return SmpKeyValue(new TKeyValue<int>("Y_Axis_Velocity", value, 1, 100, 100) );
	}
	if ( key == "Y_Axis_Acceleration")
	{
		int value = 100;
		if (m_rWeldingHeadControl.getStatesHeadY() != NULL)
		{
			value = m_rWeldingHeadControl.getStatesHeadY()->GetAxisAcceleration();
		}
		return SmpKeyValue(new TKeyValue<int>("Y_Axis_Acceleration", value, 1, 100, 100) );
	}
	if ( key == "Y_Axis_HomingDirectionPositive")
	{
		bool value = true; // Homing Richtung positiv
		if (m_rWeldingHeadControl.getStatesHeadY() != NULL)
		{
			if (m_rWeldingHeadControl.getStatesHeadY()->GetAxisHomingDirPos()) // Homing Richtung positiv
				value = true; // Homing Richtung positiv
			else
				value = false; // Homing Richtung negativ
		}
		return SmpKeyValue(new TKeyValue<bool>("Y_Axis_HomingDirectionPositive", value, false, true, true) );
	}
	if ( key == "TestFunctionAxisY_1")
	{
		return SmpKeyValue(new TKeyValue<bool>( "TestFunctionAxisY_1", false, false, true, false ) );
	}
	if ( key == "TestFunctionAxisY_2")
	{
		return SmpKeyValue(new TKeyValue<bool>( "TestFunctionAxisY_2", false, false, true, false ) );
	}
	if ( key == "TestFunctionAxisY_3")
	{
		return SmpKeyValue(new TKeyValue<bool>( "TestFunctionAxisY_3", false, false, true, false ) );
	}
	if ( key == "TestFunctionAxisY_4")
	{
		return SmpKeyValue(new TKeyValue<bool>( "TestFunctionAxisY_4", false, false, true, false ) );
	}
	if ( key == "TestFunctionAxisY_5")
	{
		return SmpKeyValue(new TKeyValue<bool>( "TestFunctionAxisY_5", false, false, true, false ) );
	}

	// Data regarding Z-Axis
	if ( key == "Z_Axis_Enabled")
	{
		return SmpKeyValue(new TKeyValue<bool>("Z_Axis_Enabled", m_rWeldingHeadControl.isAxisZEnabled(), false, true, false) );
	}
	if ( key == "Z_Axis_AbsolutePosition")
	{
		return SmpKeyValue(new TKeyValue<int>("Z_Axis_AbsolutePosition", m_rWeldingHeadControl.getRequiredPositionZ(), -100000, 100000, 1000) );
	}
	if ( key == "Z_Axis_SoftLimitsOnOff")
	{
		bool OnOff = false;
		if (m_rWeldingHeadControl.getStatesHeadZ() != NULL)
		{
			OnOff = m_rWeldingHeadControl.getStatesHeadZ()->GetSoftLimitsActive();
		}
		return SmpKeyValue(new TKeyValue<bool>("Z_Axis_SoftLimitsOnOff", OnOff, false, true, true) );
	}
	if ( key == "Z_Axis_SetUpperLimit")
	{
		int upper = 30000;
		if (m_rWeldingHeadControl.getStatesHeadZ() != NULL)
		{
			upper = m_rWeldingHeadControl.getStatesHeadZ()->GetUpperLimit();
		}
		return SmpKeyValue(new TKeyValue<int>("Z_Axis_SetUpperLimit", upper, -100000, 100000, 30000) );
	}
	if ( key == "Z_Axis_SetLowerLimit")
	{
		int lower = 1000;
		if (m_rWeldingHeadControl.getStatesHeadZ() != NULL)
		{
			lower = m_rWeldingHeadControl.getStatesHeadZ()->GetLowerLimit();
		}
		return SmpKeyValue(new TKeyValue<int>("Z_Axis_SetLowerLimit", lower, -100000, 100000, 1000) );
	}
	if ( key == "Z_Axis_Velocity")
	{
		int value = 100;
		if (m_rWeldingHeadControl.getStatesHeadZ() != NULL)
		{
			value = m_rWeldingHeadControl.getStatesHeadZ()->GetAxisVelocity();
		}
		return SmpKeyValue(new TKeyValue<int>("Z_Axis_Velocity", value, 1, 100, 100) );
	}
	if ( key == "Z_Axis_Acceleration")
	{
		int value = 100;
		if (m_rWeldingHeadControl.getStatesHeadZ() != NULL)
		{
			value = m_rWeldingHeadControl.getStatesHeadZ()->GetAxisAcceleration();
		}
		return SmpKeyValue(new TKeyValue<int>("Z_Axis_Acceleration", value, 1, 100, 100) );
	}
	if ( key == "Z_Axis_HomingDirectionPositive")
	{
		bool value = true; // Homing Richtung positiv
		if (m_rWeldingHeadControl.getStatesHeadZ() != NULL)
		{
			if (m_rWeldingHeadControl.getStatesHeadZ()->GetAxisHomingDirPos()) // Homing Richtung positiv
				value = true; // Homing Richtung positiv
			else
				value = false; //
		}
		return SmpKeyValue(new TKeyValue<bool>("Z_Axis_HomingDirectionPositive", value, false, true, true) );
	}
    if ( key == "DebugInfo_AxisController")
    {
        return SmpKeyValue(new TKeyValue<bool>( "DebugInfo_AxisController", m_rWeldingHeadControl.getDebugInfo_AxisController(), false, true, false ) );
    }

	// Data regarding LineLaser1
	if ( key == "LineLaser1Enabled")
	{
		return SmpKeyValue(new TKeyValue<bool>( "LineLaser1Enabled", m_rWeldingHeadControl.isLineLaser1Enabled(), false, true, false ) );
	}
	if ( key == "LineLaser1OnOff")
	{
		return SmpKeyValue(new TKeyValue<bool>( "LineLaser1OnOff", m_rWeldingHeadControl.getLineLaser1OnOff(), false, true, false ) );
	}
	if ( key == "LineLaser1Intensity")
	{
		return SmpKeyValue(new TKeyValue<int>( "LineLaser1Intensity", m_rWeldingHeadControl.getLineLaser1Intensity(), 0, 100, 50 ) );
	}
	if ( key == "LineLaser1ViaCamera")
	{
		return SmpKeyValue(new TKeyValue<bool>("LineLaser1ViaCamera", m_rWeldingHeadControl.isLineLaser1OutputViaCamera(), false, true, false ) );
	}

	// Data regarding LineLaser2
	if ( key == "LineLaser2Enabled")
	{
		return SmpKeyValue(new TKeyValue<bool>( "LineLaser2Enabled", m_rWeldingHeadControl.isLineLaser2Enabled(), false, true, false ) );
	}
	if ( key == "LineLaser2OnOff")
	{
		return SmpKeyValue(new TKeyValue<bool>( "LineLaser2OnOff", m_rWeldingHeadControl.getLineLaser2OnOff(), false, true, false ) );
	}
	if ( key == "LineLaser2Intensity")
	{
		return SmpKeyValue(new TKeyValue<int>( "LineLaser2Intensity", m_rWeldingHeadControl.getLineLaser2Intensity(), 0, 100, 50 ) );
	}
	if ( key == "LineLaser2ViaCamera")
	{
		return SmpKeyValue(new TKeyValue<bool>("LineLaser2ViaCamera", m_rWeldingHeadControl.isLineLaser2OutputViaCamera(), false, true, false ) );
	}

	// Data regarding FieldLight1
	if ( key == "FieldLight1Enabled")
	{
		return SmpKeyValue(new TKeyValue<bool>("FieldLight1Enabled", m_rWeldingHeadControl.isFieldLight1Enabled(), false, true, false ) );
	}
	if ( key == "FieldLight1OnOff")
	{
		return SmpKeyValue(new TKeyValue<bool>( "FieldLight1OnOff", m_rWeldingHeadControl.getFieldLight1OnOff(), false, true, false ) );
	}
	if ( key == "FieldLight1Intensity")
	{
		return SmpKeyValue(new TKeyValue<int>( "FieldLight1Intensity", m_rWeldingHeadControl.getFieldLight1Intensity(), 0, 100, 50 ) );
	}
	if ( key == "FieldLight1ViaCamera")
	{
		return SmpKeyValue(new TKeyValue<bool>("FieldLight1ViaCamera", m_rWeldingHeadControl.isFieldLight1OutputViaCamera(), false, true, false ) );
	}

	// Data regarding LEDPanel
	if ( key == "LED_IlluminationEnabled")
	{
		return SmpKeyValue(new TKeyValue<bool>( "LED_IlluminationEnabled", m_rWeldingHeadControl.isLED_IlluminationEnabled(), false, true, false ) );
	}
	if ( key == "LED_CONTROLLER_TYPE")
	{
		return SmpKeyValue(new TKeyValue<int>( "LED_CONTROLLER_TYPE", m_rWeldingHeadControl.getLEDControllerType(), 0, 10, 0 ) );
	}
	if ( key == "LEDPanel1OnOff")
	{
		return SmpKeyValue(new TKeyValue<bool>( "LEDPanel1OnOff", m_rWeldingHeadControl.getLEDPanelOnOff(LED_PANEL_1), false, true, false ) );
	}
	if ( key == "LEDPanel1Intensity")
	{
		return SmpKeyValue(new TKeyValue<int>( "LEDPanel1Intensity", m_rWeldingHeadControl.getLEDPanelIntensity(LED_PANEL_1), 0, 100, 50 ) );
	}
	if ( key == "LEDPanel1PulseWidth")
	{
		return SmpKeyValue(new TKeyValue<int>( "LEDPanel1PulseWidth", m_rWeldingHeadControl.getLEDPanelPulseWidth(LED_PANEL_1), 40, 400, 80 ) );
	}
	if ( key == "LEDPanel2OnOff")
	{
		return SmpKeyValue(new TKeyValue<bool>( "LEDPanel2OnOff", m_rWeldingHeadControl.getLEDPanelOnOff(LED_PANEL_2), false, true, false ) );
	}
	if ( key == "LEDPanel2Intensity")
	{
		return SmpKeyValue(new TKeyValue<int>( "LEDPanel2Intensity", m_rWeldingHeadControl.getLEDPanelIntensity(LED_PANEL_2), 0, 100, 50 ) );
	}
	if ( key == "LEDPanel2PulseWidth")
	{
		return SmpKeyValue(new TKeyValue<int>( "LEDPanel2PulseWidth", m_rWeldingHeadControl.getLEDPanelPulseWidth(LED_PANEL_2), 40, 400, 80 ) );
	}
	if ( key == "LEDPanel3OnOff")
	{
		return SmpKeyValue(new TKeyValue<bool>( "LEDPanel3OnOff", m_rWeldingHeadControl.getLEDPanelOnOff(LED_PANEL_3), false, true, false ) );
	}
	if ( key == "LEDPanel3Intensity")
	{
		return SmpKeyValue(new TKeyValue<int>( "LEDPanel3Intensity", m_rWeldingHeadControl.getLEDPanelIntensity(LED_PANEL_3), 0, 100, 50 ) );
	}
	if ( key == "LEDPanel3PulseWidth")
	{
		return SmpKeyValue(new TKeyValue<int>( "LEDPanel3PulseWidth", m_rWeldingHeadControl.getLEDPanelPulseWidth(LED_PANEL_3), 40, 400, 80 ) );
	}
	if ( key == "LEDPanel4OnOff")
	{
		return SmpKeyValue(new TKeyValue<bool>( "LEDPanel4OnOff", m_rWeldingHeadControl.getLEDPanelOnOff(LED_PANEL_4), false, true, false ) );
	}
	if ( key == "LEDPanel4Intensity")
	{
		return SmpKeyValue(new TKeyValue<int>( "LEDPanel4Intensity", m_rWeldingHeadControl.getLEDPanelIntensity(LED_PANEL_4), 0, 100, 50 ) );
	}
	if ( key == "LEDPanel4PulseWidth")
	{
		return SmpKeyValue(new TKeyValue<int>( "LEDPanel4PulseWidth", m_rWeldingHeadControl.getLEDPanelPulseWidth(LED_PANEL_4), 40, 400, 80 ) );
	}
	if ( key == "LEDPanel5OnOff")
	{
		return SmpKeyValue(new TKeyValue<bool>( "LEDPanel5OnOff", m_rWeldingHeadControl.getLEDPanelOnOff(LED_PANEL_5), false, true, false ) );
	}
	if ( key == "LEDPanel5Intensity")
	{
		return SmpKeyValue(new TKeyValue<int>( "LEDPanel5Intensity", m_rWeldingHeadControl.getLEDPanelIntensity(LED_PANEL_5), 0, 100, 50 ) );
	}
	if ( key == "LEDPanel5PulseWidth")
	{
		return SmpKeyValue(new TKeyValue<int>( "LEDPanel5PulseWidth", m_rWeldingHeadControl.getLEDPanelPulseWidth(LED_PANEL_5), 40, 400, 80 ) );
	}
	if ( key == "LEDPanel6OnOff")
	{
		return SmpKeyValue(new TKeyValue<bool>( "LEDPanel6OnOff", m_rWeldingHeadControl.getLEDPanelOnOff(LED_PANEL_6), false, true, false ) );
	}
	if ( key == "LEDPanel6Intensity")
	{
		return SmpKeyValue(new TKeyValue<int>( "LEDPanel6Intensity", m_rWeldingHeadControl.getLEDPanelIntensity(LED_PANEL_6), 0, 100, 50 ) );
	}
	if ( key == "LEDPanel6PulseWidth")
	{
		return SmpKeyValue(new TKeyValue<int>( "LEDPanel6PulseWidth", m_rWeldingHeadControl.getLEDPanelPulseWidth(LED_PANEL_6), 40, 400, 80 ) );
	}
	if ( key == "LEDPanel7OnOff")
	{
		return SmpKeyValue(new TKeyValue<bool>( "LEDPanel7OnOff", m_rWeldingHeadControl.getLEDPanelOnOff(LED_PANEL_7), false, true, false ) );
	}
	if ( key == "LEDPanel7Intensity")
	{
		return SmpKeyValue(new TKeyValue<int>( "LEDPanel7Intensity", m_rWeldingHeadControl.getLEDPanelIntensity(LED_PANEL_7), 0, 100, 50 ) );
	}
	if ( key == "LEDPanel7PulseWidth")
	{
		return SmpKeyValue(new TKeyValue<int>( "LEDPanel7PulseWidth", m_rWeldingHeadControl.getLEDPanelPulseWidth(LED_PANEL_7), 40, 400, 80 ) );
	}
	if ( key == "LEDPanel8OnOff")
	{
		return SmpKeyValue(new TKeyValue<bool>( "LEDPanel8OnOff", m_rWeldingHeadControl.getLEDPanelOnOff(LED_PANEL_8), false, true, false ) );
	}
	if ( key == "LEDPanel8Intensity")
	{
		return SmpKeyValue(new TKeyValue<int>( "LEDPanel8Intensity", m_rWeldingHeadControl.getLEDPanelIntensity(LED_PANEL_8), 0, 100, 50 ) );
	}
	if ( key == "LEDPanel8PulseWidth")
	{
		return SmpKeyValue(new TKeyValue<int>( "LEDPanel8PulseWidth", m_rWeldingHeadControl.getLEDPanelPulseWidth(LED_PANEL_8), 40, 400, 80 ) );
	}
	if ( key == "LED_SaveIntensityPersistent")
	{
		return SmpKeyValue(new TKeyValue<bool>("LED_SaveIntensityPersistent", false, false, true, false) );
	}
	if ( key == "LEDSendData")
	{
		return SmpKeyValue(new TKeyValue<bool>("LEDSendData", false, false, true, false) );
	}

	// Data regarding ScanTracker
	if ( key == "ScanTrackerEnabled")
	{
		return SmpKeyValue(new TKeyValue<bool>( "ScanTrackerEnabled", m_rWeldingHeadControl.isScanTrackerEnabled(), false, true, false ) );
	}
	if ( key == "FocalLength")
	{
		return SmpKeyValue(new TKeyValue<int>( "FocalLength", m_rWeldingHeadControl.GetFocalLength(), 0, 500, 250 ) );
	}
	if ( key == "ScanTrackerExpertMode")
	{
		return SmpKeyValue(new TKeyValue<bool>( "ScanTrackerExpertMode", m_rWeldingHeadControl.GetTrackerExpertMode(), false, true, false ) );
	}
	if ( key == "TrackerDriverOnOff")
	{
		return SmpKeyValue(new TKeyValue<bool>( "TrackerDriverOnOff", m_rWeldingHeadControl.GetTrackerDriverOnOff(), false, true, false ) );
	}
	if ( key == "ScanTrackerFrequency")
	{
		return SmpKeyValue(new TKeyValue<int>( "ScanTrackerFrequency",
						   static_cast<int>(m_rWeldingHeadControl.GetTrackerFrequencyStep()), eFreq30, eFreq500, eFreq250 ) );
	}
	if ( key == "ScanTrackerFrequencyContinuously")
	{
		return SmpKeyValue(new TKeyValue<int>( "ScanTrackerFrequencyContinuously",
						   static_cast<int>(m_rWeldingHeadControl.GetTrackerFrequencyCont()), 2, 500, 100 ) );
	}
	if ( key == "ScanWidthOutOfGapWidth")
	{
		return SmpKeyValue(new TKeyValue<bool>( "ScanWidthOutOfGapWidth", m_rWeldingHeadControl.GetScanWidthOutOfGapWidth(), false, true, false ) );
	}
	if ( key == "ScanWidthFixed")
	{
		return SmpKeyValue(new TKeyValue<int>( "ScanWidthFixed", m_rWeldingHeadControl.GetScanWidthFixed(), 0, 32800, 0 ) );
	}
	if ( key == "GapWidthToScanWidthOffset")
	{
		return SmpKeyValue(new TKeyValue<int>( "GapWidthToScanWidthOffset", m_rWeldingHeadControl.GetGapWidthToScanWidthOffset(), 0, 1000, 0 ) );
	}
	if ( key == "GapWidthToScanWidthGradient")
	{
		return SmpKeyValue(new TKeyValue<int>( "GapWidthToScanWidthGradient", m_rWeldingHeadControl.GetGapWidthToScanWidthGradient(), 0, 1000, 1 ) );
	}
	if ( key == "ScanPosOutOfGapPos")
	{
		return SmpKeyValue(new TKeyValue<bool>( "ScanPosOutOfGapPos", m_rWeldingHeadControl.GetScanPosOutOfGapPos(), false, true, true ) );
	}
	if ( key == "ScanPosFixed")
	{
		return SmpKeyValue(new TKeyValue<int>( "ScanPosFixed", m_rWeldingHeadControl.GetScanPosFixed(), -12000, 12000, 0 ) );
	}
	if ( key == "TrackerMaxAmplitude")
	{
		return SmpKeyValue(new TKeyValue<int>( "TrackerMaxAmplitude", m_rWeldingHeadControl.GetTrackerMaxAmplitude(), 0, 20000, 0 ) );
	}
	if ( key == "ScanTrackerAskStatus")
	{
		return SmpKeyValue(new TKeyValue<bool>( "ScanTrackerAskStatus", false, false, true, false ) );
	}
	if ( key == "ScanTrackerAskRevisions")
	{
		return SmpKeyValue(new TKeyValue<bool>( "ScanTrackerAskRevisions", false, false, true, false ) );
	}
	if ( key == "ScanTrackerAskSerialNumbers")
	{
		return SmpKeyValue(new TKeyValue<bool>( "ScanTrackerAskSerialNumbers", false, false, true, false ) );
	}
	if ( key == "TestFunctionTracker4")
	{
		return SmpKeyValue(new TKeyValue<bool>( "TestFunctionTracker4", false, false, true, false ) );
	}
	if ( key == "TestFunctionTracker5")
	{
		return SmpKeyValue(new TKeyValue<bool>( "TestFunctionTracker5", false, false, true, false ) );
	}
	if ( key == "LC_Parameter_No16")
	{
		int value = 0;
		if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
		{
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx16);
		}
		return SmpKeyValue(new TKeyValue<int>("LC_Parameter_No16", value, 0, 100, 0) );
	}
	if ( key == "LC_Parameter_No17")
	{
		int value = 0;
		if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
		{
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx17);
		}
		return SmpKeyValue(new TKeyValue<int>("LC_Parameter_No17", value, 0, 100, 0) );
	}
	if ( key == "LC_Parameter_No18")
	{
		int value = 0;
		if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
		{
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx18);
		}
		return SmpKeyValue(new TKeyValue<int>("LC_Parameter_No18", value, 0, 100, 0) );
	}
	if ( key == "LC_Parameter_No19")
	{
		int value = 0;
		if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
		{
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx19);
		}
		return SmpKeyValue(new TKeyValue<int>("LC_Parameter_No19", value, 0, 100, 0) );
	}
	if ( key == "LC_Parameter_No20")
	{
		int value = 0;
		if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
		{
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx20);
		}
		return SmpKeyValue(new TKeyValue<int>("LC_Parameter_No20", value, -25, 25, 0) );
	}
	if ( key == "LC_Parameter_No21")
	{
		int value = 0;
		if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
		{
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx21);
		}
		return SmpKeyValue(new TKeyValue<int>("LC_Parameter_No21", value, -25, 25, 0) );
	}
	if ( key == "LC_Parameter_No22")
	{
		int value = 0;
		if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
		{
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx22);
		}
		return SmpKeyValue(new TKeyValue<int>("LC_Parameter_No22", value, -25, 25, 0) );
	}
	if ( key == "LC_Parameter_No23")
	{
		int value = 0;
		if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
		{
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx23);
		}
		return SmpKeyValue(new TKeyValue<int>("LC_Parameter_No23", value, -25, 25, 0) );
	}
	if ( key == "LC_Parameter_No24")
	{
		int value = 0;
		if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
		{
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx24);
		}
		return SmpKeyValue(new TKeyValue<int>("LC_Parameter_No24", value, 0, 100, 0) );
	}
	if ( key == "LC_Parameter_No25")
	{
		int value = 0;
		if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
		{
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx25);
		}
		return SmpKeyValue(new TKeyValue<int>("LC_Parameter_No25", value, 0, 100, 0) );
	}
	if ( key == "LC_Parameter_No26")
	{
		int value = 0;
		if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
		{
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx26);
		}
		return SmpKeyValue(new TKeyValue<int>("LC_Parameter_No26", value, 0, 100, 0) );
	}
	if ( key == "LC_Parameter_No27")
	{
		int value = 0;
		if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
		{
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx27);
		}
		return SmpKeyValue(new TKeyValue<int>("LC_Parameter_No27", value, 0, 100, 0) );
	}
	if ( key == "LC_Parameter_No28")
	{
		int value = 0;
		if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
		{
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx28);
		}
		return SmpKeyValue(new TKeyValue<int>("LC_Parameter_No28", value, -100, 100, 0) );
	}

	// motorisierter Z-Kollimator
	if ( key == "ZCollimatorEnabled")
	{
		return SmpKeyValue(new TKeyValue<bool>( "ZCollimatorEnabled", m_rWeldingHeadControl.isZCollimatorEnabled(), false, true, false ) );
	}
	if ( key == "Z_Collimator_Homing")
	{
		return SmpKeyValue(new TKeyValue<bool>( "Z_Collimator_Homing", false, false, true, false ) );
	}
	if ( key == "Z_Collimator_CenterPosition")
	{
		return SmpKeyValue(new TKeyValue<bool>( "Z_Collimator_CenterPosition", false, false, true, false ) );
	}
	if ( key == "Z_Collimator_SystemOffset")
	{
		return SmpKeyValue(new TKeyValue<double>( "Z_Collimator_SystemOffset", m_rWeldingHeadControl.GetZCSystemOffset(), -50.0, 50.0, 0.0, 2 ) ); // 2 digits after decimal point
	}
	if ( key == "Z_Collimator_LensToFocusRatio")
	{
		return SmpKeyValue(new TKeyValue<double>( "Z_Collimator_LensToFocusRatio", m_rWeldingHeadControl.GetZCLensToFocusRatio(), 0.0, 15.0, 3.73, 2 ) ); // 2 digits after decimal point
	}
	if ( key == "Z_Collimator_PositionAbsolute")
	{
        if (m_rWeldingHeadControl.getZCollimatorType() == eZColliAnalog)
        {
            return SmpKeyValue(new TKeyValue<int>( "Z_Collimator_PositionAbsolute", m_rWeldingHeadControl.GetZCActPosition(), 0, 16000, 0 ) );
        }
        else if (m_rWeldingHeadControl.getZCollimatorType() == eZColliDigitalV1)
        {
            return SmpKeyValue( new TKeyValue<int>( "Z_Collimator_PositionAbsolute", m_rWeldingHeadControl.GetZCActPosition(), 925, 16785, 8854) );
        }
    }
    if ( key == "Z_Collimator_ActualPosition_um")
    {
        if (m_rWeldingHeadControl.getZCollimatorType() == eZColliDigitalV1)
        {
            return SmpKeyValue(new TKeyValue<int>( "Z_Collimator_ActualPosition_um", m_rWeldingHeadControl.GetZCActualPositionV2(), 925, 16785, 8854 ) );
        }
    }
    if (key == "LiquidLensPosition")
    {
        return SmpKeyValue(new TKeyValue<double>("LiquidLensPosition", m_rWeldingHeadControl.getLiquidLensPosition(), -2.0, 3.0, 0.00, 2)); // 2 digits after decimal point
    }

	// Data regarding LaserControl
	if ( key == "LaserControlEnable")
	{
		return SmpKeyValue(new TKeyValue<bool>( "LaserControlEnable", m_rWeldingHeadControl.isLaserControlEnabled(), false, true, false ) );
	}
	if ( key == "LC_Start_Processing")
	{
		return SmpKeyValue(new TKeyValue<bool>( "LC_Start_Processing", m_rWeldingHeadControl.GetLCStartSignal(), false, true, false ) );
	}
	if ( key == "LC_Send_Data")
	{
		return SmpKeyValue(new TKeyValue<bool>("LC_Send_Data", false, false, true, false) );
	}
	if ( key == "LC_Parameter_No1")
	{
		int value = 0;
		if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
		{
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx1);
		}
		return SmpKeyValue(new TKeyValue<int>("LC_Parameter_No1", value, 0, 10, 0) );
	}
	if ( key == "LC_Parameter_No2")
	{
		int value = 0;
		if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
		{
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx2);
		}
		return SmpKeyValue(new TKeyValue<int>("LC_Parameter_No2", value, -999, 999, 0) );
	}
	if ( key == "LC_Parameter_No3")
	{
		int value = 0;
		if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
		{
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx3);
		}
		return SmpKeyValue(new TKeyValue<int>("LC_Parameter_No3", value, 0, 100, 0) );
	}
	if ( key == "LC_Parameter_No4")
	{
		int value = 0;
		if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
		{
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx4);
		}
		return SmpKeyValue(new TKeyValue<int>("LC_Parameter_No4", value, 0, 100, 0) );
	}
	if ( key == "LC_Parameter_No5")
	{
		int value = 0;
		if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
		{
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx5);
		}
		return SmpKeyValue(new TKeyValue<int>("LC_Parameter_No5", value, 0, 100, 0) );
	}
	if ( key == "LC_Parameter_No6")
	{
		int value = 0;
		if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
		{
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx6);
		}
		return SmpKeyValue(new TKeyValue<int>("LC_Parameter_No6", value, 0, 100, 0) );
	}
	if ( key == "LC_Parameter_No7")
	{
		int value = 0;
		if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
		{
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx7);
		}
		return SmpKeyValue(new TKeyValue<int>("LC_Parameter_No7", value, -25, 25, 0) );
	}
	if ( key == "LC_Parameter_No8")
	{
		int value = 0;
		if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
		{
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx8);
		}
		return SmpKeyValue(new TKeyValue<int>("LC_Parameter_No8", value, -25, 25, 0) );
	}
	if ( key == "LC_Parameter_No9")
	{
		int value = 0;
		if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
		{
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx9);
		}
		return SmpKeyValue(new TKeyValue<int>("LC_Parameter_No9", value, -25, 25, 0) );
	}
	if ( key == "LC_Parameter_No10")
	{
		int value = 0;
		if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
		{
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx10);
		}
		return SmpKeyValue(new TKeyValue<int>("LC_Parameter_No10", value, -25, 25, 0) );
	}
	if ( key == "LC_Parameter_No11")
	{
		int value = 0;
		if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
		{
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx11);
		}
		return SmpKeyValue(new TKeyValue<int>("LC_Parameter_No11", value, 0, 100, 0) );
	}
	if ( key == "LC_Parameter_No12")
	{
		int value = 0;
		if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
		{
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx12);
		}
		return SmpKeyValue(new TKeyValue<int>("LC_Parameter_No12", value, 0, 100, 0) );
	}
	if ( key == "LC_Parameter_No13")
	{
		int value = 0;
		if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
		{
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx13);
		}
		return SmpKeyValue(new TKeyValue<int>("LC_Parameter_No13", value, 0, 100, 0) );
	}
	if ( key == "LC_Parameter_No14")
	{
		int value = 0;
		if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
		{
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx14);
		}
		return SmpKeyValue(new TKeyValue<int>("LC_Parameter_No14", value, 0, 100, 0) );
	}
	if ( key == "LC_Parameter_No15")
	{
		int value = 0;
		if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
		{
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx15);
		}
		return SmpKeyValue(new TKeyValue<int>("LC_Parameter_No15", value, -100, 100, 0) );
	}

    // Data regarding Scanlab scanner
    if ( key == "Scanner_DriveToZero")
    {
        return SmpKeyValue(new TKeyValue<bool>( "Scanner_DriveToZero", false, false, true, false ) );
    }
    if ( key == "Scanner_DriveToPosition")
    {
        return SmpKeyValue(new TKeyValue<bool>( "Scanner_DriveToPosition", false, false, true, false ) );
    }
    if ( key == "Scanner_DriveWithOCTReference")
    {
        return SmpKeyValue(new TKeyValue<bool>( "Scanner_DriveWithOCTReference", false, false, true, false ) );
    }
    if ( key == "Scanner_SetOCTReference")
    {
        return SmpKeyValue(new TKeyValue<bool>( "Scanner_SetOCTReference", false, false, true, false ) );
    }
    if ( key == "Scanner_StartWeldingPreview")
    {
        return SmpKeyValue(new TKeyValue<bool>( "Scanner_StartWeldingPreview", false, false, true, false ) );
    }
    if ( key == "Scanner_StopWeldingPreview")
    {
        return SmpKeyValue(new TKeyValue<bool>( "Scanner_StopWeldingPreview", false, false, true, false ) );
    }
    if ( key == "Scanner_TestFunction1")
    {
        return SmpKeyValue(new TKeyValue<bool>( "Scanner_TestFunction1", false, false, true, false ) );
    }
    if ( key == "Scanner_TestFunction2")
    {
        return SmpKeyValue(new TKeyValue<bool>( "Scanner_TestFunction2", false, false, true, false ) );
    }
    if ( key == "SaveLoggedScannerData")
    {
        return SmpKeyValue(new TKeyValue<bool>( "SaveLoggedScannerData", false, false, true, false ) );
    }
    if (key == "ScanTracker2D_CustomFigure")
    {
        bool value{false};
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            value = m_rWeldingHeadControl.getScanlab()->getScanTracker2DCustomFigure();
        }
        return SmpKeyValue(new TKeyValue<bool>("ScanTracker2D_CustomFigure", value, false, true, false));
    }
    if (key == "Generate_ScanTracker2D_Figure")
    {
        return SmpKeyValue(new TKeyValue<bool>("Generate_ScanTracker2D_Figure", false, false, true, false));
    }
    if ( key == "ScannerDebugMode")
    {
        bool oValue = false;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->GetEnableDebugMode();
        }
        return SmpKeyValue(new TKeyValue<bool>( "ScannerDebugMode", oValue, false, true, false ) );
    }
    if ( key == "Scanner_New_X_Position")
    {
        double oValue = 0.0;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->GetScannerNewXPosition();
        }
        return SmpKeyValue(new TKeyValue<double>( "Scanner_New_X_Position", oValue, -500.0, 500.0, 0.0 ) );
    }
    if ( key == "Scanner_New_Y_Position")
    {
        double oValue = 0.0;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->GetScannerNewYPosition();
        }
        return SmpKeyValue(new TKeyValue<double>( "Scanner_New_Y_Position", oValue, -500.0, 500.0, 0.0 ) );
    }
    if ( key == "Scanner_Actual_X_Position")
    {
        double oValue = 0.0;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->GetScannerActXPosition();
        }
        auto oKeyValue = SmpKeyValue(new TKeyValue<double>( "Scanner_Actual_X_Position", oValue, -500.0, 500.0, 0.0 ) );
        oKeyValue->setReadOnly(true);
        return oKeyValue;
    }
    if ( key == "Scanner_Actual_Y_Position")
    {
        double oValue = 0.0;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->GetScannerActYPosition();
        }
        auto oKeyValue = SmpKeyValue(new TKeyValue<double>( "Scanner_Actual_Y_Position", oValue, -500.0, 500.0, 0.0 ) );
        oKeyValue->setReadOnly(true);
        return oKeyValue;
    }
    if ( key == "OCT_Reference_Arm")
    {
        int oValue = 0;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->GetOCTReferenceArm();
        }
        return SmpKeyValue(new TKeyValue<int>( "OCT_Reference_Arm", oValue, 1, 4, 1 ) );
    }
    if ( key == "Scanner_Jump_Speed")
    {
        double oValue = 0.0;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->GetScannerJumpSpeed();
        }
        auto oKeyValue = SmpKeyValue(new TKeyValue<double>( "Scanner_Jump_Speed", oValue, 0.01, 200.0, 0.01 ) );
        oKeyValue->setPrecision(2);
        return oKeyValue;
    }
    if ( key == "Scanner_Mark_Speed")
    {
        double oValue = 0.0;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->GetScannerMarkSpeed();
        }
        auto oKeyValue = SmpKeyValue(new TKeyValue<double>( "Scanner_Mark_Speed", oValue, 0.01, 200.0, 0.01 ) );
        oKeyValue->setPrecision(2);
        return oKeyValue;
    }
    if ( key == "Scanner_Wobble_Frequency")
    {
        int oValue = 0;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->GetScannerWobbleFrequency();
        }
        return SmpKeyValue(new TKeyValue<int>( "Scanner_Wobble_Frequency", oValue, -6000, 6000, 0 ) );
    }
    if ( key == "Scanner_Wobble_X_Size")
    {
        double oValue = 0.0;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->GetScannerWobbleXSize();
        }
        return SmpKeyValue(new TKeyValue<double> ( "Scanner_Wobble_X_Size", oValue, 0.0, 20.0, 0.0 ) );
    }
    if (key == "Scanner_Wobble_Y_Size")
    {
        double oValue = 0.0;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->GetScannerWobbleYSize();
        }
        return SmpKeyValue(new TKeyValue<double> ( "Scanner_Wobble_Y_Size", oValue, 0.0, 20.0, 0.0 ) );
    }
    if ( key == "Scanner_Wobble_Radius")
    {
        double oValue = 0.0;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->GetScannerWobbleRadius();
        }
        return SmpKeyValue(new TKeyValue<double>( "Scanner_Wobble_Radius", oValue, 0.0, 10.0, 0.3 ) );
    }
    if ( key == "Scanner_LaserOnDelay_us")
    {
        int oValue = 0;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->GetScannerLaserOnDelay();
        }
        return SmpKeyValue(new TKeyValue<int>( "Scanner_LaserOnDelay_us", oValue, -100000, 100000, 0 ) );
    }
    if ( key == "Scanner_LaserOffDelay_us")
    {
        int oValue = 0;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->GetScannerLaserOffDelay();
        }
        return SmpKeyValue(new TKeyValue<int>( "Scanner_LaserOffDelay_us", oValue, -100000, 100000, 0 ) );
    }
    if (key == "ScanTracker2D_LaserDelay")
    {
        int oValue = 0;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->getScanTracker2DLaserDelay();
        }
        return SmpKeyValue(new TKeyValue<int>("ScanTracker2D_LaserDelay", oValue, -100, 100, 0));
    }
    if ( key == "Scanner_LaserPowerStatic")
    {
        int oValue = 0;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->GetLaserPowerStatic();
        }
        return SmpKeyValue(new TKeyValue<int>( "Scanner_LaserPowerStatic", oValue, 0, 100, 0 ) );
    }
    if ( key == "Scanner_LaserPowerStaticRing")
    {
        int oValue = 0;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->GetLaserPowerStaticRing();
        }
        return SmpKeyValue(new TKeyValue<int>( "Scanner_LaserPowerStaticRing", oValue, 0, 100, 0 ) );
    }
    auto oLaserPowerParameter = key.find("Scanner_LaserPower_Parameter");
    if (oLaserPowerParameter != std::string::npos)
    {
        std::string oParaNumber = key.substr(28);
        int oValue = 10;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->GetLaserPowerParameter(static_cast<LaserPowerParameter>(std::stoi(oParaNumber) - 1));
        }
        return SmpKeyValue(new TKeyValue<int>( key, oValue, 0, 100, 10 ) );
    }

    /*
    * Scanner_WeldingFigureNumber gives you the correct file with the number for getting the correct welding figure
    */
    if ( key == "Scanner_WeldingFigureNumber" )
    {
        int oValue = 0;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->GetWeldingFigureNumber();
        }
        return SmpKeyValue(new TKeyValue<int>( "Scanner_WeldingFigureNumber", oValue, -1, 10, -1 ) );
    }
    /*
    * Scanner_FileNumber gives you the correct file with the number for getting the correct wobble figure
    */
    if ( key == "Scanner_FileNumber" )
    {
        int oValue = 0;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->GetWobbelFigureNumber();
        }
        return SmpKeyValue(new TKeyValue<int>( "Scanner_FileNumber", oValue, -1, 20, -1 ) );
    }
    if ( key == "IsLaserPowerDigital" )
    {
        bool oValue = false;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->GetLaserPowerIsDigital();
        }
        return SmpKeyValue(new TKeyValue<bool>( "IsLaserPowerDigital", oValue, false, true, false ) );
    }
    if ( key == "LaserDelay" )
    {
        double oValue = 0.0;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->GetLaserDelay();
        }
        return SmpKeyValue(new TKeyValue<double>( "LaserDelay", oValue, 0.0, 25.0, 0.0 ) );
    }
    if ( key == "Scanner_Wobble_Mode")
    {
        int oValue = 0;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->GetWobbleMode();
        }
        return SmpKeyValue(new TKeyValue<int>( "Scanner_Wobble_Mode", oValue, -2, 2, -2 ) );
    }
    if (key == "ScanTracker2D_Angle")
    {
        double oValue{0.0};
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->getScanTracker2DAngle();
        }
        return SmpKeyValue(new TKeyValue<double>("ScanTracker2D_Angle", oValue, -360.0, 360.0, 0.0));
    }
    if (key == "ScanTracker2D_ScanWidthFixed_X")
    {
        double oValue{0.0};
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->getScanTracker2DScanWidthFixedX();
        }
        return SmpKeyValue(new TKeyValue<double>("ScanTracker2D_ScanWidthFixed_X", oValue, 0.0, 20.0, 1.0));
    }
    if (key == "ScanTracker2D_ScanWidthFixed_Y")
    {
        double oValue{0.0};
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->getScanTracker2DScanWidthFixedY();
        }
        return SmpKeyValue(new TKeyValue<double>("ScanTracker2D_ScanWidthFixed_Y", oValue, 0.0, 20.0, 1.0));
    }
    if (key == "ScanTracker2D_ScanPosFixed_X")
    {
        double oValue{0.0};
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->getScanTracker2DScanPosFixedX();
        }
        return SmpKeyValue(new TKeyValue<double>("ScanTracker2D_ScanPosFixed_X", oValue, -200.0, 200.0, 0.0));
    }
    if (key == "ScanTracker2D_ScanPosFixed_Y")
    {
        double oValue{0.0};
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->getScanTracker2DScanPosFixedY();
        }
        return SmpKeyValue(new TKeyValue<double>("ScanTracker2D_ScanPosFixed_Y", oValue, -200.0, 200.0, 0.0));
    }
    if ( key == "PositionDifferenceTolerance")
    {
        int oValue = 0;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->getPositionDifferenceTolerance();
        }
        return SmpKeyValue(new TKeyValue<int> ( "PositionDifferenceTolerance", oValue, 1, 255, 183));
    }
    if ( key == "PositionDifferenceToleranceMillimeter")
    {
        double oValue = 0.0;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->getPositionDifferenceTolerance();
            return SmpKeyValue(new TKeyValue<double> ( "PositionDifferenceToleranceMillimeter", oValue, m_rWeldingHeadControl.getScanlab()->minPositionDifferenceToleranceInMM(), m_rWeldingHeadControl.getScanlab()->maxPositionDifferenceToleranceInMM(), m_rWeldingHeadControl.getScanlab()->defaultPositionDifferenceToleranceInMM()));
        }
        return SmpKeyValue(new TKeyValue<double> ( "PositionDifferenceToleranceMillimeter", oValue, 0, 1.0, 0.5));
    }
    if ( key == "Laser_Power_Compensation_10us")
    {
        int oValue = 0;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->laserPowerDelayCompensation();
            return SmpKeyValue(new TKeyValue<int> ("Laser_Power_Compensation_10us", oValue, -200, 200, 0));
        }
        return SmpKeyValue(new TKeyValue<int> ( "Laser_Power_Compensation_10us", oValue, -200, 200, 0));
    }
    if ( key == "DebugModePeriod")
    {
        int oValue = 0;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->GetMeasurementPeriod();
        }
        return SmpKeyValue(new TKeyValue<int>( "DebugModePeriod", oValue, 1, 500000, 1 ) );
    }
    for (std::size_t i = 0u; i < 4u; i++)
    {
        const std::string loggerSignal = std::string{"ScannerLoggerSignal"} + std::to_string(i + 1);
        if (key == loggerSignal)
        {
            int oValue = 0;
            if (m_rWeldingHeadControl.getScanlab() != nullptr)
            {
                oValue = m_rWeldingHeadControl.getScanlab()->getLoggedSignal(i);
            }
            return SmpKeyValue(new TKeyValue<int>( loggerSignal, oValue, -1, 57, -1 ) );
        }
    }
    if ( key == "StatusSignalHead1Axis1")
    {
        auto oValue = 0;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->GetStatusSignalHead1Axis1();
        }
        return SmpKeyValue(new TKeyValue<int>( "StatusSignalHead1Axis1", oValue, -1, 9, -1) );
    }
    if ( key == "StatusSignalHead1Axis2")
    {
        auto oValue = 0;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->GetStatusSignalHead1Axis2();
        }
        return SmpKeyValue(new TKeyValue<int>( "StatusSignalHead1Axis2", oValue, -1, 9, -1) );
    }
    if ( key == "ScannerCompensateHeight")
    {
        bool oValue = false;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->scannerCompensateHeight();
        }
        return SmpKeyValue(new TKeyValue<bool>( "ScannerCompensateHeight", oValue, false, true, false ) );
    }
    if ( key == "CompensationHeight")
    {
        double oValue = 0.0;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->compensationHeight();
        }
        return SmpKeyValue(new TKeyValue<double>( "CompensationHeight", oValue, -15.0, 15.0, 0.0 ) );
    }
    if ( key == "IsCompensationHeightFixed")
    {
        bool oValue = false;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->isCompensateHeightFixed();
        }
        return SmpKeyValue(new TKeyValue<bool>( "IsCompensationHeightFixed", oValue, false, true, false ) );
    }
    if (key == "CorrectionFileMode")
    {
        int oValue = 1;
        if (m_rWeldingHeadControl.getScanlab() != nullptr)
        {
            oValue = m_rWeldingHeadControl.getScanlab()->correctionFileMode();
        }
        return SmpKeyValue(new TKeyValue<bool>("CorrectionFileMode", oValue, 1, 3, 1));
    }

    // parameters regarding LWM40
    if ( key == "LWM40_No1_AmpPlasma")
    {
        return SmpKeyValue(new TKeyValue<int>("LWM40_No1_AmpPlasma", m_rWeldingHeadControl.GetLWM40_No1_AmpPlasma(), 0, 6, 0) );
    }
    if ( key == "LWM40_No1_AmpTemperature")
    {
        return SmpKeyValue(new TKeyValue<int>("LWM40_No1_AmpTemperature", m_rWeldingHeadControl.GetLWM40_No1_AmpTemperature(), 0, 6, 0) );
    }
    if ( key == "LWM40_No1_AmpBackReflection")
    {
        return SmpKeyValue(new TKeyValue<int>("LWM40_No1_AmpBackReflection", m_rWeldingHeadControl.GetLWM40_No1_AmpBackReflection(), 0, 6, 0) );
    }
    if ( key == "LWM40_No1_AmpAnalogInput")
    {
        return SmpKeyValue(new TKeyValue<int>("LWM40_No1_AmpAnalogInput", m_rWeldingHeadControl.GetLWM40_No1_AmpAnalogInput(), 0, 6, 0) );
    }

	// misc
	if ( key == "LaserPowerInputEnabled")
	{
		return SmpKeyValue(new TKeyValue<bool>( "LaserPowerInputEnabled", m_rWeldingHeadControl.isLaserPowerInputEnabled(), false, true, false ) );
	}
	if ( key == "ClearEncoderCounter1")
	{
		return SmpKeyValue(new TKeyValue<bool>( "ClearEncoderCounter1", false, false, true, false ) );
	}
	if ( key == "ClearEncoderCounter2")
	{
		return SmpKeyValue(new TKeyValue<bool>( "ClearEncoderCounter2", false, false, true, false ) );
	}

	return NULL;
}

SmpKeyValue DeviceServer::get(KeyHandle handle, int subDevice)
{
	return NULL;
}

Configuration DeviceServer::get(int subDevice)
{
	Configuration config;

	if (m_rWeldingHeadControl.isLineLaser1Enabled() )
	{
		config.push_back( SmpKeyValue( new TKeyValue<bool>( "LineLaser1OnOff", m_rWeldingHeadControl.getLineLaser1OnOff(), false, true, false) ) );
		config.push_back( SmpKeyValue( new TKeyValue<int>( "LineLaser1Intensity", m_rWeldingHeadControl.getLineLaser1Intensity(), 0, 100, 50 ) ) );
	}
	if (m_rWeldingHeadControl.isLineLaser2Enabled() )
	{
		config.push_back( SmpKeyValue( new TKeyValue<bool>( "LineLaser2OnOff", m_rWeldingHeadControl.getLineLaser2OnOff(), false, true, false) ) );
		config.push_back( SmpKeyValue( new TKeyValue<int>( "LineLaser2Intensity", m_rWeldingHeadControl.getLineLaser2Intensity(), 0, 100, 50 ) ) );
	}
	if (m_rWeldingHeadControl.isFieldLight1Enabled() )
	{
		config.push_back( SmpKeyValue( new TKeyValue<bool>( "FieldLight1OnOff", m_rWeldingHeadControl.getFieldLight1OnOff(), false, true, false) ) );
		config.push_back( SmpKeyValue( new TKeyValue<int>( "FieldLight1Intensity", m_rWeldingHeadControl.getFieldLight1Intensity(), 0, 100, 50 ) ) );
	}
	if (m_rWeldingHeadControl.isLED_IlluminationEnabled() )
	{
		config.push_back( SmpKeyValue( new TKeyValue<bool>( "LEDSendData", false, false, true, false) ) );
		config.push_back( SmpKeyValue( new TKeyValue<bool>( "LEDPanel1OnOff", m_rWeldingHeadControl.getLEDPanelOnOff(LED_PANEL_1), false, true, false) ) );
		config.push_back( SmpKeyValue( new TKeyValue<int>( "LEDPanel1Intensity", m_rWeldingHeadControl.getLEDPanelIntensity(LED_PANEL_1), 0, 100, 50 ) ) );
		config.push_back( SmpKeyValue( new TKeyValue<int>( "LEDPanel1PulseWidth", m_rWeldingHeadControl.getLEDPanelPulseWidth(LED_PANEL_1), 40, 400, 80 ) ) );
		config.push_back( SmpKeyValue( new TKeyValue<bool>( "LEDPanel2OnOff", m_rWeldingHeadControl.getLEDPanelOnOff(LED_PANEL_2), false, true, false) ) );
		config.push_back( SmpKeyValue( new TKeyValue<int>( "LEDPanel2Intensity", m_rWeldingHeadControl.getLEDPanelIntensity(LED_PANEL_2), 0, 100, 50 ) ) );
		config.push_back( SmpKeyValue( new TKeyValue<int>( "LEDPanel2PulseWidth", m_rWeldingHeadControl.getLEDPanelPulseWidth(LED_PANEL_2), 40, 400, 80 ) ) );
		config.push_back( SmpKeyValue( new TKeyValue<bool>( "LEDPanel3OnOff", m_rWeldingHeadControl.getLEDPanelOnOff(LED_PANEL_3), false, true, false) ) );
		config.push_back( SmpKeyValue( new TKeyValue<int>( "LEDPanel3Intensity", m_rWeldingHeadControl.getLEDPanelIntensity(LED_PANEL_3), 0, 100, 50 ) ) );
		config.push_back( SmpKeyValue( new TKeyValue<int>( "LEDPanel3PulseWidth", m_rWeldingHeadControl.getLEDPanelPulseWidth(LED_PANEL_3), 40, 400, 80 ) ) );
		config.push_back( SmpKeyValue( new TKeyValue<bool>( "LEDPanel4OnOff", m_rWeldingHeadControl.getLEDPanelOnOff(LED_PANEL_4), false, true, false) ) );
		config.push_back( SmpKeyValue( new TKeyValue<int>( "LEDPanel4Intensity", m_rWeldingHeadControl.getLEDPanelIntensity(LED_PANEL_4), 0, 100, 50 ) ) );
		config.push_back( SmpKeyValue( new TKeyValue<int>( "LEDPanel4PulseWidth", m_rWeldingHeadControl.getLEDPanelPulseWidth(LED_PANEL_4), 40, 400, 80 ) ) );
		config.push_back( SmpKeyValue( new TKeyValue<bool>( "LEDPanel5OnOff", m_rWeldingHeadControl.getLEDPanelOnOff(LED_PANEL_5), false, true, false) ) );
		config.push_back( SmpKeyValue( new TKeyValue<int>( "LEDPanel5Intensity", m_rWeldingHeadControl.getLEDPanelIntensity(LED_PANEL_5), 0, 100, 50 ) ) );
		config.push_back( SmpKeyValue( new TKeyValue<int>( "LEDPanel5PulseWidth", m_rWeldingHeadControl.getLEDPanelPulseWidth(LED_PANEL_5), 40, 400, 80 ) ) );
		config.push_back( SmpKeyValue( new TKeyValue<bool>( "LEDPanel6OnOff", m_rWeldingHeadControl.getLEDPanelOnOff(LED_PANEL_6), false, true, false) ) );
		config.push_back( SmpKeyValue( new TKeyValue<int>( "LEDPanel6Intensity", m_rWeldingHeadControl.getLEDPanelIntensity(LED_PANEL_6), 0, 100, 50 ) ) );
		config.push_back( SmpKeyValue( new TKeyValue<int>( "LEDPanel6PulseWidth", m_rWeldingHeadControl.getLEDPanelPulseWidth(LED_PANEL_6), 40, 400, 80 ) ) );
		config.push_back( SmpKeyValue( new TKeyValue<bool>( "LEDPanel7OnOff", m_rWeldingHeadControl.getLEDPanelOnOff(LED_PANEL_7), false, true, false) ) );
		config.push_back( SmpKeyValue( new TKeyValue<int>( "LEDPanel7Intensity", m_rWeldingHeadControl.getLEDPanelIntensity(LED_PANEL_7), 0, 100, 50 ) ) );
		config.push_back( SmpKeyValue( new TKeyValue<int>( "LEDPanel7PulseWidth", m_rWeldingHeadControl.getLEDPanelPulseWidth(LED_PANEL_7), 40, 400, 80 ) ) );
		config.push_back( SmpKeyValue( new TKeyValue<bool>( "LEDPanel8OnOff", m_rWeldingHeadControl.getLEDPanelOnOff(LED_PANEL_8), false, true, false) ) );
		config.push_back( SmpKeyValue( new TKeyValue<int>( "LEDPanel8Intensity", m_rWeldingHeadControl.getLEDPanelIntensity(LED_PANEL_8), 0, 100, 50 ) ) );
		config.push_back( SmpKeyValue( new TKeyValue<int>( "LEDPanel8PulseWidth", m_rWeldingHeadControl.getLEDPanelPulseWidth(LED_PANEL_8), 40, 400, 80 ) ) );
		// LED_SaveIntensityPersistent soll nicht mehr in KeyValue-Liste sichtbar sein 12.1.18 EA/SB
		//config.push_back( SmpKeyValue( new TKeyValue<bool>("LED_SaveIntensityPersistent", false, false, true, false) ) );
	}
	if (m_rWeldingHeadControl.isScanTrackerEnabled() )
	{
		config.push_back( SmpKeyValue( new TKeyValue<int>( "FocalLength", m_rWeldingHeadControl.GetFocalLength(), 0, 500, 250 ) ) );
		config.push_back( SmpKeyValue( new TKeyValue<bool>( "TrackerDriverOnOff", m_rWeldingHeadControl.GetTrackerDriverOnOff(), false, true, false) ) );
		config.push_back( SmpKeyValue( new TKeyValue<bool>( "ScanTrackerExpertMode", m_rWeldingHeadControl.GetTrackerExpertMode(), false, true, false) ) );
		config.push_back( SmpKeyValue( new TKeyValue<int>( "ScanTrackerFrequency", static_cast<int>(m_rWeldingHeadControl.GetTrackerFrequencyStep()), eFreq30, eFreq500, eFreq250 ) ) );
		config.push_back( SmpKeyValue( new TKeyValue<int>( "ScanTrackerFrequencyContinuously", static_cast<int>(m_rWeldingHeadControl.GetTrackerFrequencyCont()), 2, 500, 100 ) ) );
		config.push_back( SmpKeyValue( new TKeyValue<bool>( "ScanWidthOutOfGapWidth", m_rWeldingHeadControl.GetScanWidthOutOfGapWidth(), false, true, false) ) );
		config.push_back( SmpKeyValue( new TKeyValue<int>( "ScanWidthFixed", m_rWeldingHeadControl.GetScanWidthFixed(), 0, 32800, 0 ) ) );
		config.push_back( SmpKeyValue( new TKeyValue<int>( "GapWidthToScanWidthOffset", m_rWeldingHeadControl.GetGapWidthToScanWidthOffset(), 0, 1000, 0 ) ) );
		config.push_back( SmpKeyValue( new TKeyValue<int>( "GapWidthToScanWidthGradient", m_rWeldingHeadControl.GetGapWidthToScanWidthGradient(), 0, 1000, 1 ) ) );
		config.push_back( SmpKeyValue( new TKeyValue<bool>( "ScanPosOutOfGapPos", m_rWeldingHeadControl.GetScanPosOutOfGapPos(), false, true, true) ) );
		config.push_back( SmpKeyValue( new TKeyValue<int>( "ScanPosFixed", m_rWeldingHeadControl.GetScanPosFixed(), -12000, 12000, 0 ) ) );
		config.push_back( SmpKeyValue( new TKeyValue<int>( "TrackerMaxAmplitude", m_rWeldingHeadControl.GetTrackerMaxAmplitude(), 0, 20000, 0 ) ) );
		config.push_back( SmpKeyValue( new TKeyValue<bool>( "ScanTrackerAskStatus", false, false, true, false) ) );
		config.push_back( SmpKeyValue( new TKeyValue<bool>( "ScanTrackerAskRevisions", false, false, true, false) ) );
		config.push_back( SmpKeyValue( new TKeyValue<bool>( "ScanTrackerAskSerialNumbers", false, false, true, false) ) );
		//config.push_back( SmpKeyValue( new TKeyValue<bool>( "TestFunctionTracker4", false, false, true, false) ) );
		//config.push_back( SmpKeyValue( new TKeyValue<bool>( "TestFunctionTracker5", false, false, true, false) ) );
	}
	if (m_rWeldingHeadControl.isLaserControlEnabled() )
	{
		if (m_rWeldingHeadControl.getDataToLaserControl() != NULL)
		{
			int value;
			config.push_back( SmpKeyValue( new TKeyValue<bool>("LC_Start_Processing", m_rWeldingHeadControl.GetLCStartSignal(), false, true, false) ) );
			config.push_back( SmpKeyValue( new TKeyValue<bool>("LC_Send_Data", false, false, true, false) ) );
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx1);
			config.push_back( SmpKeyValue( new TKeyValue<int>("LC_Parameter_No1", value, 0, 10, 0) ) );
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx2);
			config.push_back( SmpKeyValue( new TKeyValue<int>("LC_Parameter_No2", value, -999, 999, 0) ) );
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx3);
			config.push_back( SmpKeyValue( new TKeyValue<int>("LC_Parameter_No3", value, 0, 100, 0) ) );
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx4);
			config.push_back( SmpKeyValue( new TKeyValue<int>("LC_Parameter_No4", value, 0, 100, 0) ) );
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx5);
			config.push_back( SmpKeyValue( new TKeyValue<int>("LC_Parameter_No5", value, 0, 100, 0) ) );
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx6);
			config.push_back( SmpKeyValue( new TKeyValue<int>("LC_Parameter_No6", value, 0, 100, 0) ) );
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx7);
			config.push_back( SmpKeyValue( new TKeyValue<int>("LC_Parameter_No7", value, -25, 25, 0) ) );
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx8);
			config.push_back( SmpKeyValue( new TKeyValue<int>("LC_Parameter_No8", value, -25, 25, 0) ) );
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx9);
			config.push_back( SmpKeyValue( new TKeyValue<int>("LC_Parameter_No9", value, -25, 25, 0) ) );
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx10);
			config.push_back( SmpKeyValue( new TKeyValue<int>("LC_Parameter_No10", value, -25, 25, 0) ) );
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx11);
			config.push_back( SmpKeyValue( new TKeyValue<int>("LC_Parameter_No11", value, 0, 100, 0) ) );
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx12);
			config.push_back( SmpKeyValue( new TKeyValue<int>("LC_Parameter_No12", value, 0, 100, 0) ) );
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx13);
			config.push_back( SmpKeyValue( new TKeyValue<int>("LC_Parameter_No13", value, 0, 100, 0) ) );
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx14);
			config.push_back( SmpKeyValue( new TKeyValue<int>("LC_Parameter_No14", value, 0, 100, 0) ) );
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx15);
			config.push_back( SmpKeyValue( new TKeyValue<int>("LC_Parameter_No15", value, -100, 100, 0) ) );
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx16);
			config.push_back( SmpKeyValue( new TKeyValue<int>("LC_Parameter_No16", value, 0, 100, 0) ) );
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx17);
			config.push_back( SmpKeyValue( new TKeyValue<int>("LC_Parameter_No17", value, 0, 100, 0) ) );
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx18);
			config.push_back( SmpKeyValue( new TKeyValue<int>("LC_Parameter_No18", value, 0, 100, 0) ) );
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx19);
			config.push_back( SmpKeyValue( new TKeyValue<int>("LC_Parameter_No19", value, 0, 100, 0) ) );
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx20);
			config.push_back( SmpKeyValue( new TKeyValue<int>("LC_Parameter_No20", value, -25, 25, 0) ) );
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx21);
			config.push_back( SmpKeyValue( new TKeyValue<int>("LC_Parameter_No21", value, -25, 25, 0) ) );
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx22);
			config.push_back( SmpKeyValue( new TKeyValue<int>("LC_Parameter_No22", value, -25, 25, 0) ) );
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx23);
			config.push_back( SmpKeyValue( new TKeyValue<int>("LC_Parameter_No23", value, -25, 25, 0) ) );
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx24);
			config.push_back( SmpKeyValue( new TKeyValue<int>("LC_Parameter_No24", value, 0, 100, 0) ) );
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx25);
			config.push_back( SmpKeyValue( new TKeyValue<int>("LC_Parameter_No25", value, 0, 100, 0) ) );
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx26);
			config.push_back( SmpKeyValue( new TKeyValue<int>("LC_Parameter_No26", value, 0, 100, 0) ) );
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx27);
			config.push_back( SmpKeyValue( new TKeyValue<int>("LC_Parameter_No27", value, 0, 100, 0) ) );
			value = m_rWeldingHeadControl.getDataToLaserControl()->getDataVariable(eLCVarIdx28);
			config.push_back( SmpKeyValue( new TKeyValue<int>("LC_Parameter_No28", value, -100, 100, 0) ) );
		}
	}
	if (m_rWeldingHeadControl.isZCollimatorEnabled() )
	{
		config.push_back( SmpKeyValue( new TKeyValue<double>( "Z_Collimator_SystemOffset", m_rWeldingHeadControl.GetZCSystemOffset(), -50.0, 50.0, 0, 2) ) ); // 2 digits after decimal point
		config.push_back( SmpKeyValue( new TKeyValue<double>( "Z_Collimator_LensToFocusRatio", m_rWeldingHeadControl.GetZCLensToFocusRatio(), 0.0, 15.0, 3.73, 2) ) ); // 2 digits after decimal point
		config.push_back( SmpKeyValue( new TKeyValue<bool>( "Z_Collimator_Homing", false, false, true, false) ) );
		config.push_back( SmpKeyValue( new TKeyValue<bool>( "Z_Collimator_CenterPosition", false, false, true, false) ) );
        if (m_rWeldingHeadControl.getZCollimatorType() == eZColliAnalog)
        {
            config.push_back( SmpKeyValue( new TKeyValue<int>( "Z_Collimator_PositionAbsolute", m_rWeldingHeadControl.GetZCActPosition(), 0, 16000, 0) ) );
        }
        else if (m_rWeldingHeadControl.getZCollimatorType() == eZColliDigitalV1)
        {
            config.push_back( SmpKeyValue( new TKeyValue<int>( "Z_Collimator_PositionAbsolute", m_rWeldingHeadControl.GetZCActPosition(), 925, 16785, 8854) ) );
            config.push_back( SmpKeyValue( new TKeyValue<int>( "Z_Collimator_ActualPosition_um", m_rWeldingHeadControl.GetZCActualPositionV2(), 925, 16785, 8854) ) );
        }
	}
    if (m_rWeldingHeadControl.isLiquidLensControllerEnabled())
    {
        config.push_back(SmpKeyValue(new TKeyValue<double>("LiquidLensPosition", m_rWeldingHeadControl.getLiquidLensPosition(), -2.0, 3.0, 0.00, 2))); // 2 digits after decimal point
    }
    if (m_rWeldingHeadControl.getScanlab() != NULL)
    {
        if (m_rWeldingHeadControl.getScanlab()->isScanlabScannerEnabled() )
        {
            config.push_back( SmpKeyValue( new TKeyValue<bool>( "Scanner_DriveToZero", false, false, true, false ) ) );
            config.push_back( SmpKeyValue( new TKeyValue<bool>( "Scanner_DriveToPosition", false, false, true, false ) ) );
            if (m_rWeldingHeadControl.getScanlab()->isOCT_with_reference_arms())
            {
                config.push_back( SmpKeyValue( new TKeyValue<bool>( "Scanner_DriveWithOCTReference", false, false, true, false ) ) );
                config.push_back( SmpKeyValue( new TKeyValue<bool>( "Scanner_SetOCTReference", false, false, true, false ) ) );
            }
            config.push_back( SmpKeyValue( new TKeyValue<bool>( "Scanner_StartWeldingPreview", false, false, true, false ) ) );
            config.push_back( SmpKeyValue( new TKeyValue<bool>( "Scanner_StopWeldingPreview", false, false, true, false ) ) );
            //config.push_back( SmpKeyValue( new TKeyValue<bool>( "Scanner_TestFunction1", false, false, true, false ) ) );
            //config.push_back( SmpKeyValue( new TKeyValue<bool>( "Scanner_TestFunction2", false, false, true, false ) ) );
            config.push_back( SmpKeyValue( new TKeyValue<bool>( "SaveLoggedScannerData", false, false, true, false ) ) );

            config.push_back(SmpKeyValue(new TKeyValue<bool>("ScanTracker2D_CustomFigure", m_rWeldingHeadControl.getScanlab()->getScanTracker2DCustomFigure(), false, true, false)));
            config.push_back(SmpKeyValue(new TKeyValue<bool>("Generate_ScanTracker2D_Figure", false, false, true, false)));
            double oValue = m_rWeldingHeadControl.getScanlab()->getScanTracker2DAngle();
            config.push_back(SmpKeyValue(new TKeyValue<double>("ScanTracker2D_Angle", oValue, -360.0, 360.0, 0.0)));
            config.push_back(SmpKeyValue(new TKeyValue<int>("ScanTracker2D_LaserDelay", m_rWeldingHeadControl.getScanlab()->getScanTracker2DLaserDelay(), -100, 100, 0)));

            config.push_back(SmpKeyValue(new TKeyValue<bool>("ScanWidthOutOfGapWidth", m_rWeldingHeadControl.GetScanWidthOutOfGapWidth(), false, true, false)));
            config.push_back(SmpKeyValue(new TKeyValue<double>("ScanTracker2D_ScanWidthFixed_X", m_rWeldingHeadControl.getScanlab()->getScanTracker2DScanWidthFixedX(), 0.0, 20.0, 1.0)));
            config.push_back(SmpKeyValue(new TKeyValue<double>("ScanTracker2D_ScanWidthFixed_Y", m_rWeldingHeadControl.getScanlab()->getScanTracker2DScanWidthFixedY(), 0.0, 20.0, 1.0)));
            config.push_back(SmpKeyValue(new TKeyValue<bool>("ScanPosOutOfGapPos", m_rWeldingHeadControl.GetScanPosOutOfGapPos(), false, true, true)));
            config.push_back(SmpKeyValue(new TKeyValue<double>("ScanTracker2D_ScanPosFixed_X", m_rWeldingHeadControl.getScanlab()->getScanTracker2DScanPosFixedX(), -200.0, 200.0, 0.0)));
            config.push_back(SmpKeyValue(new TKeyValue<double>("ScanTracker2D_ScanPosFixed_Y", m_rWeldingHeadControl.getScanlab()->getScanTracker2DScanPosFixedY(), -200.0, 200.0, 0.0)));

            oValue = m_rWeldingHeadControl.getScanlab()->GetScannerNewXPosition();
            config.push_back( SmpKeyValue( new TKeyValue<double>( "Scanner_New_X_Position", oValue, -500.0, 500.0, 0.0 ) ) );
            oValue = m_rWeldingHeadControl.getScanlab()->GetScannerNewYPosition();
            config.push_back( SmpKeyValue( new TKeyValue<double>( "Scanner_New_Y_Position", oValue, -500.0, 500.0, 0.0 ) ) );
            oValue = m_rWeldingHeadControl.getScanlab()->GetScannerActXPosition();
            auto oKeyValue = SmpKeyValue( new TKeyValue<double>( "Scanner_Actual_X_Position", oValue, -500.0, 500.0, 0.0 ) );
            oKeyValue->setReadOnly(true);
            config.push_back( oKeyValue );
            oValue = m_rWeldingHeadControl.getScanlab()->GetScannerActYPosition();
            oKeyValue =  SmpKeyValue( new TKeyValue<double>( "Scanner_Actual_Y_Position", oValue, -500.0, 500.0, 0.0 ) );
            oKeyValue->setReadOnly(true);
            config.push_back( oKeyValue );
            int oValueInt{};
            if (m_rWeldingHeadControl.getScanlab()->isOCT_with_reference_arms())
            {
                oValueInt = m_rWeldingHeadControl.getScanlab()->GetOCTReferenceArm();
                config.push_back( SmpKeyValue( new TKeyValue<int>( "OCT_Reference_Arm", oValueInt, 1, 4, 1 ) ) );
            }
            oValue = m_rWeldingHeadControl.getScanlab()->GetScannerJumpSpeed();
            oKeyValue = SmpKeyValue( new TKeyValue<double>( "Scanner_Jump_Speed", oValue, 0.01, 200.0, 0.01 ) );
            oKeyValue->setPrecision(2);
            config.push_back( oKeyValue );
            oValue = m_rWeldingHeadControl.getScanlab()->GetScannerMarkSpeed();
            oKeyValue = SmpKeyValue( new TKeyValue<double>( "Scanner_Mark_Speed", oValue, 0.01, 200.0, 0.01 ) );
            oKeyValue->setPrecision(2);
            config.push_back( oKeyValue );
            oValue = m_rWeldingHeadControl.getScanlab()->GetScannerWobbleXSize();
            config.push_back( SmpKeyValue( new TKeyValue<double>("Scanner_Wobble_X_Size", oValue, 0.0, 20.0, 0.0) ) );
            oValue = m_rWeldingHeadControl.getScanlab()->GetScannerWobbleYSize();
            config.push_back( SmpKeyValue( new TKeyValue<double>("Scanner_Wobble_Y_Size", oValue, 0.0, 20.0, 0.0) ) );
            oValue = m_rWeldingHeadControl.getScanlab()->GetScannerWobbleRadius();
            config.push_back( SmpKeyValue( new TKeyValue<double>( "Scanner_Wobble_Radius", oValue, 0.0, 10.0, 0.3 ) ) );
            oValueInt = m_rWeldingHeadControl.getScanlab()->GetScannerLaserOnDelay();
            config.push_back( SmpKeyValue( new TKeyValue<int>( "Scanner_LaserOnDelay_us", oValueInt, -100000, 100000, 0 ) ) );
            oValueInt = m_rWeldingHeadControl.getScanlab()->GetScannerLaserOffDelay();
            config.push_back( SmpKeyValue( new TKeyValue<int>( "Scanner_LaserOffDelay_us", oValueInt, -100000, 100000, 0 ) ) );
            oValueInt = m_rWeldingHeadControl.getScanlab()->GetLaserPowerStatic();
            config.push_back( SmpKeyValue( new TKeyValue<int>( "Scanner_LaserPowerStatic", oValueInt, 0, 100, 0 ) ) );
            oValueInt = m_rWeldingHeadControl.getScanlab()->GetLaserPowerStaticRing();
            config.push_back( SmpKeyValue( new TKeyValue<int>( "Scanner_LaserPowerStaticRing", oValueInt, 0, 100, 0 ) ) );
            for(int i = eLaserPowerPara1;i <= eLaserPowerPara30;i ++)
            {
                oValueInt = m_rWeldingHeadControl.getScanlab()->GetLaserPowerParameter(static_cast<LaserPowerParameter>(i));
                std::string oParaName = "Scanner_LaserPower_Parameter" + std::to_string(i + 1);
                config.push_back( SmpKeyValue( new TKeyValue<int>( oParaName, oValueInt, 0, 100, 10 ) ) );
            }
            oValueInt = m_rWeldingHeadControl.getScanlab()->GetScannerWobbleFrequency();
            config.push_back( SmpKeyValue( new TKeyValue<int>( "Scanner_Wobble_Frequency", oValueInt, -6000, 6000, 0 ) ) );
            oValueInt = m_rWeldingHeadControl.getScanlab()->GetWobbleMode();
            config.push_back( SmpKeyValue( new TKeyValue<int>( "Scanner_Wobble_Mode", oValueInt, -2, 2, -2 ) ) );
            oValueInt = m_rWeldingHeadControl.getScanlab()->GetWeldingFigureNumber();
            config.push_back( SmpKeyValue(new TKeyValue<int>( "Scanner_WeldingFigureNumber", oValueInt, -1, 10, -1 ) ));
            oValueInt = m_rWeldingHeadControl.getScanlab()->GetWobbelFigureNumber();
            config.push_back( SmpKeyValue(new TKeyValue<int>( "Scanner_FileNumber", oValueInt, -1, 20, -1 ) ));
            oValueInt = m_rWeldingHeadControl.getScanlab()->GetMeasurementPeriod();
            config.push_back( SmpKeyValue(new TKeyValue<int>( "DebugModePeriod", oValueInt, 1, 500000, 1 ) ));
            for (std::size_t i = 0u; i < 4u; i++)
            {
                oValueInt = m_rWeldingHeadControl.getScanlab()->getLoggedSignal(i);
                config.push_back( SmpKeyValue(new TKeyValue<int>( "ScannerLoggerSignal" + std::to_string(i + 1), oValueInt, -1, 57, -1 ) ));
            }
            oValueInt = m_rWeldingHeadControl.getScanlab()->GetStatusSignalHead1Axis1();
            config.push_back( SmpKeyValue(new TKeyValue<int>( "StatusSignalHead1Axis1", oValueInt, -1, 9, -1 ) ));
            oValueInt = m_rWeldingHeadControl.getScanlab()->GetStatusSignalHead1Axis2();
            config.push_back( SmpKeyValue(new TKeyValue<int>( "StatusSignalHead1Axis2", oValueInt, -1, 9, -1 ) ));
            oValueInt = m_rWeldingHeadControl.getScanlab()->getPositionDifferenceToleranceInBits();
            config.push_back( SmpKeyValue(new TKeyValue<int>( "PositionDifferenceTolerance", oValueInt, 1, 255, 183) ));
            oValue = m_rWeldingHeadControl.getScanlab()->getPositionDifferenceTolerance();
            config.push_back( SmpKeyValue(new TKeyValue<double>( "PositionDifferenceToleranceMillimeter", oValue, m_rWeldingHeadControl.getScanlab()->minPositionDifferenceToleranceInMM(), m_rWeldingHeadControl.getScanlab()->maxPositionDifferenceToleranceInMM(), m_rWeldingHeadControl.getScanlab()->defaultPositionDifferenceToleranceInMM()) ));
            oValue = m_rWeldingHeadControl.getScanlab()->GetLaserDelay();
            config.push_back( SmpKeyValue(new TKeyValue<double>( "LaserDelay", oValue, 0.0, 25.0, 0.0 ) ));
            auto oValueBool = m_rWeldingHeadControl.getScanlab()->GetLaserPowerIsDigital();
            config.push_back( SmpKeyValue(new TKeyValue<bool>( "IsLaserPowerDigital", oValueBool, false, true, false ) ));
            oValueBool = m_rWeldingHeadControl.getScanlab()->GetEnableDebugMode();
            config.push_back( SmpKeyValue(new TKeyValue<bool> ( "ScannerDebugMode", oValueBool, false, true, false ) ));
            oValueBool = m_rWeldingHeadControl.getScanlab()->scannerCompensateHeight();
            config.push_back( SmpKeyValue(new TKeyValue<bool> ( "ScannerCompensateHeight", oValueBool, false, true, false ) ));
            oValueBool = m_rWeldingHeadControl.getScanlab()->isCompensateHeightFixed();
            config.push_back( SmpKeyValue(new TKeyValue<bool> ( "IsCompensationHeightFixed", oValueBool, false, true, false ) ));
            oValueInt = m_rWeldingHeadControl.getScanlab()->laserPowerDelayCompensation();
            config.push_back( SmpKeyValue( new TKeyValue<int>("Laser_Power_Compensation_10us", oValueInt, -200, 200, 0) ));
            oValue = m_rWeldingHeadControl.getScanlab()->compensationHeight();
            config.push_back( SmpKeyValue( new TKeyValue<int>("CompensationHeight", oValue, -15.0, 15.0, 0.0) ));
            oValueInt = m_rWeldingHeadControl.getScanlab()->correctionFileMode();
            config.push_back(SmpKeyValue(new TKeyValue<int>("CorrectionFileMode", oValueInt, 1, 3, 1)));
        }
    }
	if (m_rWeldingHeadControl.isAxisXEnabled() )
	{
		if (m_rWeldingHeadControl.getStatesHeadX() != NULL)
		{
			config.push_back( SmpKeyValue( new TKeyValue<int>( "X_Axis_AbsolutePosition", m_rWeldingHeadControl.getRequiredPositionX(), -100000, 100000, 1000 ) ) );
			config.push_back( SmpKeyValue( new TKeyValue<bool>( "X_Axis_SoftLimitsOnOff", m_rWeldingHeadControl.getStatesHeadX()->GetSoftLimitsActive(), false, true, true) ) );
			config.push_back( SmpKeyValue( new TKeyValue<int>( "X_Axis_SetUpperLimit", m_rWeldingHeadControl.getStatesHeadX()->GetUpperLimit(), -100000, 100000, 30000 ) ) );
			config.push_back( SmpKeyValue( new TKeyValue<int>( "X_Axis_SetLowerLimit", m_rWeldingHeadControl.getStatesHeadX()->GetLowerLimit(), -100000, 100000, 1000 ) ) );
			config.push_back( SmpKeyValue( new TKeyValue<int>( "X_Axis_Velocity", m_rWeldingHeadControl.getStatesHeadX()->GetAxisVelocity(), 1, 100, 100 ) ) );
			config.push_back( SmpKeyValue( new TKeyValue<int>( "X_Axis_Acceleration", m_rWeldingHeadControl.getStatesHeadX()->GetAxisAcceleration(), 1, 100, 100 ) ) );
			config.push_back( SmpKeyValue( new TKeyValue<bool>("X_Axis_HomingDirectionPositive", m_rWeldingHeadControl.getStatesHeadX()->GetAxisHomingDirPos(), false, true, true) ) );
		}
	}
	if (m_rWeldingHeadControl.isAxisYEnabled() )
	{
		if (m_rWeldingHeadControl.getStatesHeadY() != NULL)
		{
			config.push_back( SmpKeyValue( new TKeyValue<int>( "Y_Axis_AbsolutePosition", m_rWeldingHeadControl.getRequiredPositionY(), -100000, 100000, 1000 ) ) );
			config.push_back( SmpKeyValue( new TKeyValue<bool>( "Y_Axis_SoftLimitsOnOff", m_rWeldingHeadControl.getStatesHeadY()->GetSoftLimitsActive(), false, true, true) ) );
			config.push_back( SmpKeyValue( new TKeyValue<int>( "Y_Axis_SetUpperLimit", m_rWeldingHeadControl.getStatesHeadY()->GetUpperLimit(), -100000, 100000, 30000 ) ) );
			config.push_back( SmpKeyValue( new TKeyValue<int>( "Y_Axis_SetLowerLimit", m_rWeldingHeadControl.getStatesHeadY()->GetLowerLimit(), -100000, 100000, 1000 ) ) );
			config.push_back( SmpKeyValue( new TKeyValue<int>( "Y_Axis_Velocity", m_rWeldingHeadControl.getStatesHeadY()->GetAxisVelocity(), 1, 100, 100 ) ) );
			config.push_back( SmpKeyValue( new TKeyValue<int>( "Y_Axis_Acceleration", m_rWeldingHeadControl.getStatesHeadY()->GetAxisAcceleration(), 1, 100, 100 ) ) );
			config.push_back( SmpKeyValue( new TKeyValue<bool>("Y_Axis_HomingDirectionPositive", m_rWeldingHeadControl.getStatesHeadY()->GetAxisHomingDirPos(), false, true, true) ) );
            config.push_back( SmpKeyValue( new TKeyValue<bool>( "TestFunctionAxisY_1", false, false, true, false) ) );
            config.push_back( SmpKeyValue( new TKeyValue<bool>( "TestFunctionAxisY_2", false, false, true, false) ) );
            config.push_back( SmpKeyValue( new TKeyValue<bool>( "TestFunctionAxisY_3", false, false, true, false) ) );
            config.push_back( SmpKeyValue( new TKeyValue<bool>( "TestFunctionAxisY_4", false, false, true, false) ) );
            config.push_back( SmpKeyValue( new TKeyValue<bool>( "TestFunctionAxisY_5", false, false, true, false) ) );
		}
	}
	if (m_rWeldingHeadControl.isAxisZEnabled() )
	{
		if (m_rWeldingHeadControl.getStatesHeadZ() != NULL)
		{
			config.push_back( SmpKeyValue( new TKeyValue<int>( "Z_Axis_AbsolutePosition", m_rWeldingHeadControl.getRequiredPositionZ(), -100000, 100000, 1000 ) ) );
			config.push_back( SmpKeyValue( new TKeyValue<bool>( "Z_Axis_SoftLimitsOnOff", m_rWeldingHeadControl.getStatesHeadZ()->GetSoftLimitsActive(), false, true, true) ) );
			config.push_back( SmpKeyValue( new TKeyValue<int>( "Z_Axis_SetUpperLimit", m_rWeldingHeadControl.getStatesHeadZ()->GetUpperLimit(), -100000, 100000, 30000 ) ) );
			config.push_back( SmpKeyValue( new TKeyValue<int>( "Z_Axis_SetLowerLimit", m_rWeldingHeadControl.getStatesHeadZ()->GetLowerLimit(), -100000, 100000, 1000 ) ) );
			config.push_back( SmpKeyValue( new TKeyValue<int>( "Z_Axis_Velocity", m_rWeldingHeadControl.getStatesHeadZ()->GetAxisVelocity(), 1, 100, 100 ) ) );
			config.push_back( SmpKeyValue( new TKeyValue<int>( "Z_Axis_Acceleration", m_rWeldingHeadControl.getStatesHeadZ()->GetAxisAcceleration(), 1, 100, 100 ) ) );
			config.push_back( SmpKeyValue( new TKeyValue<bool>("Z_Axis_HomingDirectionPositive", m_rWeldingHeadControl.getStatesHeadZ()->GetAxisHomingDirPos(), false, true, true) ) );
		}
	}
    config.push_back( SmpKeyValue( new TKeyValue<bool>( "DebugInfo_AxisController", m_rWeldingHeadControl.getDebugInfo_AxisController(), false, true, false ) ) );
	if (m_rWeldingHeadControl.isEncoderInput1Enabled() )
	{
		config.push_back( SmpKeyValue( new TKeyValue<bool>( "ClearEncoderCounter1", false, false, true, false) ) );
	}
	if (m_rWeldingHeadControl.isEncoderInput2Enabled() )
	{
		config.push_back( SmpKeyValue( new TKeyValue<bool>( "ClearEncoderCounter2", false, false, true, false) ) );
	}

    if (m_rWeldingHeadControl.isLWM40_No1_Enabled() )
    {
        config.push_back( SmpKeyValue( new TKeyValue<int>( "LWM40_No1_AmpPlasma", m_rWeldingHeadControl.GetLWM40_No1_AmpPlasma(), 0, 6, 0) ) );
        config.push_back( SmpKeyValue( new TKeyValue<int>( "LWM40_No1_AmpTemperature", m_rWeldingHeadControl.GetLWM40_No1_AmpTemperature(), 0, 6, 0) ) );
        config.push_back( SmpKeyValue( new TKeyValue<int>( "LWM40_No1_AmpBackReflection", m_rWeldingHeadControl.GetLWM40_No1_AmpBackReflection(), 0, 6, 0) ) );
        config.push_back( SmpKeyValue( new TKeyValue<int>( "LWM40_No1_AmpAnalogInput", m_rWeldingHeadControl.GetLWM40_No1_AmpAnalogInput(), 0, 6, 0) ) );
    }

std::cout << config << std::endl;

	return config;
}

} // namespace ethercat

} // namespace precitec


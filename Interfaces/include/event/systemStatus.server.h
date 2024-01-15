/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			AL, SB
 *  @date			2010
 *  @brief			Transmit system status			
 */

#ifndef SYSTEM_STATUS_SERVER_H_
#define SYSTEM_STATUS_SERVER_H_

#include "event/systemStatus.interface.h"

#include <string>

/*
* Hier werden die abstrakten Basisklassen, sowie die
* Messages definiert. Dieser Header wird sowohl fuer
* den lokalen (Implementation, MsgHandler) sowie den
* remote-Teil (Proxyer) benoetigt.
*/
namespace precitec
{
	namespace interface
	{

		//----------------------------------------------------------
		// Abstrakte Basis Klassen  = Interface
		// Die <Implementation> und <Proxy> Spezialisierungen leiten von diesen
		// Basisklassen ab.

		/**
		* Signaler zeigt auf primitive Weise den Systemzustand an.
		* Der State-Enum bietet drei Zustaende an. Verschiedene
		* Handler koennen diese Zustaende unterschiedlich darstellen.
		*/

		typedef image::BImage imageBImage;

		template<>
		class TSystemStatus<EventServer> : public TSystemStatus<AbstractInterface>
		{
		public:
			TSystemStatus() {}
			virtual ~TSystemStatus() {}
		public:
			virtual void signalSystemError(ErrorState errorState)
			{
				if (errorState==NoError)
				{
					wmLog(eDebug, "SystemError : None\n");
				} else if (errorState&Error1) {
					wmLog(eDebug, "SystemError : Error 1\n");
				} else if (errorState&Error2) {
					wmLog(eDebug, "SystemError : Error 2\n");
				} else if (errorState&Error3) {
					wmLog(eDebug, "SystemError : Error 3\n");
				};
			}

			virtual void signalHardwareError(Hardware hardware)
			{
				wmLog(eDebug, "Hardeware error in system\n");
				switch (hardware)
				{
				case SiSoGrabber:
					wmLog(eDebug, "Silicon Software Framegrabber\n");
					break;
				case PFCamera:
					wmLog(eDebug, "Photon Focus Camera\n");
					break;
				case DigIO:
					wmLog(eDebug, "Digital-IO Board\n");
					break;
				default:
					wmLog(eDebug, "unknown Hardware\n");
					break;
				};
			}
			virtual void acknowledgeError(ErrorState errorState)
			{
				wmLog(eDebug, "Resetting system error\n");
				switch (errorState)
				{
				case Error1:
					wmLog(eDebug, "error1\n");
					break;
				case Error2:
					wmLog(eDebug, "error2\n");
					break;
				case Error3:
					wmLog(eDebug, "error3\n");
					break;
				default:
					wmLog(eDebug, "unknown error\n");
					break;
				};
			}
			virtual void signalState(ReadyState state)
			{
				switch (state)
				{
				case SystemUp:
					wmLog(eDebug, "SystemState: GREEN\n");
					break;
				case SystemDown:
					wmLog(eDebug, "SystemState: RED\n");
					break;
				case SystemBusy:
					wmLog(eDebug, "SystemState: YELLOW\n");
					break;
				default:
					break;
				};
			}
			virtual void mark(ErrorType errorType, int position)
			{
				wmLog(eDebug, "There is an error \n");
				switch (errorType)
				{
				case Hole:
					wmLog(eDebug, "hole\n");
					break;
				case Pore:
					wmLog(eDebug, "pore\n");
					break;
				case Splatter:
					wmLog(eDebug, "splatter\n");
					break;
				case Geometry:
					wmLog(eDebug, "bad geometry\n");
					break;
				default:
					break;
				};
				wmLog(eDebug, " at position %i\n", position);
			}

			virtual void operationState(OperationState state)
			{
				switch (state)
				{
				case NormalMode:
					wmLog(eDebug, "OperationState: NormalMode\n");
					break;
				case LiveMode:
					wmLog(eDebug, "OperationState: LiveMode\n");
					break;
				case AutomaticMode:
					wmLog(eDebug, "OperationState: AutomaticMode\n");
					break;
				default:
					break;
				};
			}
			
			virtual void upsState(UpsState state)
			{
                 switch (state)
				{
				case Online:
					wmLog(eDebug, "UpsState: Online\n");
					break;
				case OnBattery:
					wmLog(eDebug, "UpsState: OnBattery\n");
					break;
				case LowBattery:
					wmLog(eDebug, "UpsState: LowBattery\n");
					break;
				case ReplaceBattery:
					wmLog(eDebug, "UpsState: ReplaceBattery\n");
					break;
				case NoConnection:
					wmLog(eDebug, "UpsState: NoConnection\n");
					break;
				default:
					break;
				};
			}
			
			virtual void workingState(WorkingState state)
			{
				switch (state)
				{
				case WaitForTrigger:
					wmLog(eDebug, "WorkingState: WaitForTrigger\n");
					break;
				case Triggered:
					wmLog(eDebug, "WorkingState: Triggered\n");
					break;
				case SeamSeriesChanged:
					wmLog(eDebug, "WorkingState: SeamSeriesChanged\n");
					break;
				case Stopped:
					wmLog(eDebug, "WorkingState: Stopped\n");
					break;
				default:
					break;
				};
			}

			virtual void signalProductInfo(ProductInfo productInfo)
			{}
		};



	} // namespace interface
} // namespace precitec

#endif /*SYSTEM_STATUS_SERVER_H_*/

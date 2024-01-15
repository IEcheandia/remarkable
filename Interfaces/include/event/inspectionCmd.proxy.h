#ifndef INSPECTIONCMD_PROXY_H_
#define INSPECTIONCMD_PROXY_H_


#include "server/eventProxy.h"
#include "module/interfaces.h"
#include "event/inspectionCmd.interface.h"



namespace precitec
{
namespace interface
{
	template <>
	class TInspectionCmd<EventProxy> : public Server<EventProxy>, public TInspectionCmd<AbstractInterface>, public TInspectionCmdMessageDefinition
	{
	public:
		/// Der CTor braucht die Klasse, die die Arbeit erledigt, also den eigentlichen Server
		TInspectionCmd() : EVENT_PROXY_CTOR(TInspectionCmd), TInspectionCmd<AbstractInterface>()
		{
			//std::cout << "remote CTor::TRegistrar<Proxy> ohne Protokoll" << std::endl;
		}

		virtual ~TInspectionCmd() {}

	public:

		virtual void signalNotReady(int relatedException)
		{
			INIT_EVENT(SignalNotReady);
			//signaler().initMessage(Msg::index);
			signaler().marshal(relatedException);
			signaler().send();
		}

		virtual void requestCalibration(int type)
		{
			INIT_EVENT(RequestCalibration);
			//signaler().initMessage(Msg::index);
			signaler().marshal(type);
			signaler().send();
		}

		virtual void startLivemode(const Poco::UUID& productId, int seamseries, int seam)
		{
			INIT_EVENT(StartLivemode);
			signaler().marshal(productId);
			signaler().marshal(seamseries);
			signaler().marshal(seam);
			signaler().send();
		}

		virtual void stopLivemode()
		{
			INIT_EVENT(StopLivemode);
			//signaler().initMessage(Msg::index);
			signaler().send();
		}

		virtual void startSimulation(const Poco::UUID& productId, int mode)
		{
			INIT_EVENT(StartSimulation);
			//signaler().initMessage(Msg::index);
			signaler().marshal(productId);
			signaler().marshal(mode);
			signaler().send();
		}

		virtual void stopSimulation()
		{
			INIT_EVENT(StopSimulation);
			//signaler().initMessage(Msg::index);
			signaler().send();
		}

		virtual void startProductTeachInMode()
		{
			INIT_EVENT(StartProductTeachInMode);
			//signaler().initMessage(Msg::index);
			signaler().send();
		}

		virtual void abortProductTeachInMode()
		{
			INIT_EVENT(AbortProductTeachInMode);
			//signaler().initMessage(Msg::index);
			signaler().send();
		}

		virtual void quitSystemFault()
		{
			INIT_EVENT(QuitSystemFault);
			//signaler().initMessage(Msg::index);
			signaler().send();
		}

		virtual void resetSumErrors()
		{
			INIT_EVENT(ResetSumErrors);
			//signaler().initMessage(Msg::index);
			signaler().send();
		}

		virtual void emergencyStop()
		{
			INIT_EVENT(EmergencyStop);
			//signaler().initMessage(Msg::index);
			signaler().send();
		}

		virtual void resetEmergencyStop()
		{
			INIT_EVENT(ResetEmergencyStop);
			//signaler().initMessage(Msg::index);
			signaler().send();
		}


        void signalReady(int relatedException) override
        {
            INIT_EVENT(SignalReady);
            signaler().marshal(relatedException);
            signaler().send();
        }

	};

} // interface
} // precitec


#endif /*INSPECTIONCMD_PROXY_H_*/

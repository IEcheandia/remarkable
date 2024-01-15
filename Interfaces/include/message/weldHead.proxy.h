#ifndef WELDHEAD_PROXY_H_
#define WELDHEAD_PROXY_H_

#include  "message/weldHead.interface.h"
#include  "server/proxy.h"


namespace precitec
{
namespace interface
{
	/**
	 *	AbstrakteBasisklasse des Module Servers
	 * 	Der Modul-Server kommuniziert mit dem Module-Manager,
	 * 		d.h. er wartet auf Befehler dessselben
	 */
	using namespace  message;
	using namespace  system;

	// ?? using system::message::Sender;


	/**
	 *  die Remote-Call-Spezialisierung ist recht simpel
	 * Fuer jede Member-Funktion des Basis-Servers gibt es eine
	 * Funktion mit gleicher Signatur.
	 * Nach initMessage wird jeder Parameter gemarshalled (in die Message verpackt)
	 * und dann mit send abgeschickt. Fuer nicht-void-funktionen wird der Returnparameter
	 * demarshalled (aus der Message entpackt) unr zurueckgegeben.
	 * Dass dies alles von Hand passiert hat den Vorteil, dass dieser Code
	 * ggf. Optimierungen bzw. Spezialcode fuer In-Out-Parameter enthalten kann.
	 * Dies den Kompiler automatisch machen zu lassen, ist vermutlich moeglich, wuerde aber
	 * sehr viel undurchsichtigen Template-Macro-Metaprogrammier-Code enthalten.
	 */
	template <>
	class TWeldHeadMsg<MsgProxy> : public Server<MsgProxy>, public TWeldHeadMsg<AbstractInterface>, public TWeldHeadMsgDefinition
	{
	public:
		// die Ctoren sind Standard und muessen nur umbenannt werden (inkl ResultArgsRangeServer<Messages>)
		/// beide Basisklassen muessen geeignet initialisiert werden
		TWeldHeadMsg() : PROXY_CTOR(TWeldHeadMsg), TWeldHeadMsg<AbstractInterface>()
		{
			//std::cout << "remote CTor::TDevice<Proxy> ohne Protokoll" << std::endl;
		}

		/// normalerweise wird das Protokoll gleich mitgeliefert
		TWeldHeadMsg(SmpProtocolInfo &p) : PROXY_CTOR1(TWeldHeadMsg,  p), TWeldHeadMsg<AbstractInterface>()
		{
			//std::cout << "remote CTor::TDevice<Proxy> " << std::endl;
		}
		/// der DTor muss virtuell sein
		virtual ~TWeldHeadMsg() {}
	public:
		// das gesamte virtuelle Interfae muss hier nachgebildet werden

		virtual bool setHeadPos(HeadAxisID axis, int value)
		{
			INIT_MESSAGE(SetHeadPos);
			sender().marshal(axis);
			sender().marshal(value);
			sender().send();
			bool ret = false;
            sender().deMarshal(ret);
			return ret;
		}

		virtual bool setHeadMode(HeadAxisID axis, MotionMode mode, bool bHome)
		{
			INIT_MESSAGE(SetHeadMode);
			sender().marshal(axis);
			sender().marshal(mode);
			sender().marshal(bHome);
			sender().send();
			bool ret = false;
            sender().deMarshal(ret);
			return ret;
		}

		virtual int getHeadPosition(HeadAxisID axis)
		{
			INIT_MESSAGE(GetHeadPosition);
			sender().marshal(axis);
			sender().send();
			int ret = 0;
			sender().deMarshal(ret);
			return ret;
		}

		virtual int getLowerLimit(HeadAxisID axis)
		{
			INIT_MESSAGE(GetLowerLimit);
			sender().marshal(axis);
			sender().send();
			int ret = 0;
			sender().deMarshal(ret);
			return ret;
		}

		virtual int getUpperLimit(HeadAxisID axis)
		{
			INIT_MESSAGE(GetUpperLimit);
			sender().marshal(axis);
			sender().send();
			int ret = 0;
			sender().deMarshal(ret);
			return ret;
		}

		virtual bool doZCollHoming(void)
		{
			INIT_MESSAGE(DoZCollHoming);
			sender().send();
			bool ret = false;
			sender().deMarshal(ret);
			return ret;
		}

		virtual bool getLEDEnable(LEDPanelNo ledPanel)
        {
			INIT_MESSAGE(GetLEDEnable);
            sender().marshal(ledPanel);
			sender().send();
			bool ret = false;
			sender().deMarshal(ret);
            return ret;
        }

		virtual bool setLEDEnable(LEDPanelNo ledPanel, bool enabled)
        {
			INIT_MESSAGE(SetLEDEnable);
            sender().marshal(ledPanel);
            sender().marshal(enabled);
			sender().send();
			bool ret = false;
			sender().deMarshal(ret);
            return ret;
        }

        bool reloadFiberSwitchCalibration(void) override
        {
            INIT_MESSAGE(ReloadFiberSwitchCalibration);
            sender().send();
            bool ret = false;
            sender().deMarshal(ret);
            return ret;
        }

        bool weldForScannerCalibration(const std::vector<geo2d::DPoint>& points, double laserPowerInPct, double durationInMs, double jumpSpeedInMmPerSec) override
        {
            INIT_MESSAGE(WeldForScannerCalibration);
            sender().marshal(jumpSpeedInMmPerSec);
            sender().marshal(durationInMs);
            sender().marshal(laserPowerInPct);
            sender().marshal(points);
            sender().send();
            bool ret = false;
            sender().deMarshal(ret);
            return ret;
        }

        bool doZCollDrivingRelative(int value) override
        {
            INIT_MESSAGE(DoZCollDrivingRelative);
            sender().marshal(value);
            sender().send();
            bool ret = false;
            sender().deMarshal(ret);
            return ret;
        }
	};

} // namespace interface
} // namespace precitec

#endif /*WELDHEAD_PROXY_H_*/

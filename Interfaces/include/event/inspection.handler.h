#ifndef INSPECTION_HANDLER_H_
#define INSPECTION_HANDLER_H_


#include "event/inspection.h"
#include "event/inspection.interface.h"
#include "server/eventHandler.h"

namespace precitec
{
namespace interface
{
	using namespace  message;

	template <>
	class TInspection<EventHandler> : public Server<EventHandler>, public TInspectionMessageDefinition
	{
	public:
		EVENT_HANDLER( TInspection );
	public:
		void registerCallbacks()
		{
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!
			REGISTER_EVENT(StartAutomaticmode, startAutomaticmode);
			REGISTER_EVENT(StopAutomaticmode, stopAutomaticmode);
			REGISTER_EVENT(Start, start);
			REGISTER_EVENT(End, end);
			REGISTER_EVENT(Info, info);
			REGISTER_EVENT(Linelaser, linelaser);
			REGISTER_EVENT(StartCalibration, startCalibration);
			REGISTER_EVENT(StopCalibration, stopCalibration);
			REGISTER_EVENT(SeamPreStart, seamPreStart);
		}

		void startAutomaticmode(Receiver &receiver)
		{
			uint32_t producttype; receiver.deMarshal(producttype);
			uint32_t productnumber; receiver.deMarshal(productnumber);
			std::string oExtendedProductInfo; receiver.deMarshal(oExtendedProductInfo);

			getServer()->startAutomaticmode(producttype, productnumber, oExtendedProductInfo);
		}

		void stopAutomaticmode(Receiver &receiver)
		{
			getServer()->stopAutomaticmode();
		}

		void start(Receiver &receiver)
		{
			int seam; receiver.deMarshal(seam);
			getServer()->start(seam);
		}

		void end(Receiver &receiver)
		{
			int seam; receiver.deMarshal(seam);
			getServer()->end(seam);
		}

		void info(Receiver &receiver)
		{
			int seamsequence; receiver.deMarshal(seamsequence);
			getServer()->info(seamsequence);
		}

		void linelaser(Receiver &receiver)
		{
			int onoff; receiver.deMarshal(onoff);
			getServer()->linelaser(onoff!=0);
		}

		void startCalibration(Receiver &receiver)
		{
			getServer()->startCalibration();
		}

		void stopCalibration(Receiver &receiver)
		{
			getServer()->stopCalibration();
		}

		void seamPreStart(Receiver &receiver)
		{
			int seamnumber; receiver.deMarshal(seamnumber);
			getServer()->seamPreStart(seamnumber);
		}

	private:
		TInspection<AbstractInterface> * getServer()
		{
			return server_;
		}

	};

} // namespace interface
} // namespace precitec



#endif /*INSPECTION_HANDLER_H_*/

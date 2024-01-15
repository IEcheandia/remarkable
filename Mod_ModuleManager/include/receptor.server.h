#ifndef RECEPTOR_SERVER_H_
#define RECEPTOR_SERVER_H_

#include  "message/receptor.interface.h"
#include  "message/module.interface.h"
#include  "message/messageSender.h"
#include  "protocol/protocol.info.h"

namespace precitec
{
namespace interface
{

	typedef Poco::SharedPtr<Poco::Semaphore> SmpSemaphore;

	class MMAccessor;
	/**
	 * Hier wird nun der Registrar-Server implementiert
	 */
	template <>
	class TReceptor<MsgServer> : public TReceptor<AbstractInterface>	{
	public:
		TReceptor(MMAccessor &ca);
		virtual ~TReceptor() {}
	public:
		/// zwischen MM und RootM und WinMm
		virtual void 	registerModuleManager(int moduleHandle, SmpProtocolInfo & protocol) {}
		/// 1 Protokoll fuer Server und Client (systematisch davon ageleitet) kommt zurueck
		virtual SmpProtocolInfo registerModule(int moduleHandle, ModuleSpec spec, ModuleType type);
		/// alle Tabelleneintraege fuer das Modul sinddanach geloescht
		virtual void unregisterModule(int moduleHandle);

		//void loadModule(MessageHandlerList &mList, EventHandlerList &eList);

		friend std::ostream & operator << ( std::ostream &os, TReceptor<MsgServer>  const& r );


	private:
		/// erlaubt tThreadsicheren Zugriff auf gemeinsamee Daten des MM
		MMAccessor			&mmAccessor_;
		/// von dieser Basis aus swerden neue Protokolle generiert
		SmpProtocolInfo baseMMProtocol_;
		/// Protokollzaehler
		int				 			mmProtocolCursor_;
	}; // Registrar


} // namespace interface
} // namespace precitec

#endif /*RECEPTOR_SERVER_H_*/

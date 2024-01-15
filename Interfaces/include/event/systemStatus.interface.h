/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			AL, SB
 *  @date			2010
 *  @brief			Transmit system status			
 */

#ifndef SYSTEM_STATUS_INTERFACE_H_
#define SYSTEM_STATUS_INTERFACE_H_

#include <string>
#include <vector>
#include <utility> // for pair

#include "server/interface.h"
#include "module/interfaces.h" // wg appId
#include "protocol/protocol.info.h"
#include "event/systemStatus.h"

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
	// Hier folgen die Template-Definitionen fuer die verschiedenen
	// Spezialisierungen  <Implementation> <MsgHandler> <Proxyer> <Messages>

	template <int mode>
	class TSystemStatus;

	//----------------------------------------------------------
	// Abstrakte Basis Klassen  = Interface
	// Die <Implementation> und <Proxy> Spezialisierungen leiten von diesen
	// Basisklassen ab.

	/**
	 * Signaler zeigt auf primitive Weise den Systemzustand an.
	 * Der State-Enum bietet drei Zustaende an. Verschiedene
	 * Handler koennen diese Zustaende unterschiedlich darstellen.
	 */
	template<>
	class TSystemStatus<AbstractInterface>
	{
	public:
		TSystemStatus() {}
		virtual ~TSystemStatus() {}
	public:
		virtual void signalSystemError(ErrorState errorState) = 0;
		virtual void signalHardwareError(Hardware hardware) = 0;
		virtual void acknowledgeError(ErrorState errorState) = 0;
		virtual void signalState(ReadyState state) = 0;
		virtual void mark(ErrorType errorType, int position) = 0;
		virtual void operationState(OperationState state) = 0;
		virtual void upsState(UpsState state) = 0;
		virtual void workingState(WorkingState state) = 0;
		virtual void signalProductInfo(ProductInfo productInfo) = 0;
        /**
         * Emitted whenever the InspectManager updated the @p productId
         **/
        virtual void productUpdated(Poco::UUID productId) = 0;
        /**
         * Emitted whenever the InspectManager updated filter parameters for @p instanceFilterId of @p measureTaskId
         **/
        virtual void filterParameterUpdated(Poco::UUID measureTaskId, Poco::UUID instanceFilterId) = 0;
	};

    struct TSystemStatusMessageDefinition
    {
		EVENT_MESSAGE(Mark, int, int);
		EVENT_MESSAGE(SignalSystemError, int);
		EVENT_MESSAGE(SignalHardwareError, int);
		EVENT_MESSAGE(AcknowledgeError, int);
		EVENT_MESSAGE(SignalState, int);
		EVENT_MESSAGE(OperationStateMsg, int);
		EVENT_MESSAGE(UpsStateMsg, UpsState);
		EVENT_MESSAGE(WorkingStateMsg, int);
		EVENT_MESSAGE(SignalProductInfo, ProductInfo);
        EVENT_MESSAGE(ProductUpdated, Poco::UUID);
        EVENT_MESSAGE(FilterParameterUpdated, Poco::UUID, Poco::UUID);

		MESSAGE_LIST(
			SignalSystemError,
			SignalHardwareError,
			AcknowledgeError,
			SignalState,
			Mark,
			OperationStateMsg,
			UpsStateMsg,
			WorkingStateMsg,
			SignalProductInfo,
            ProductUpdated,
            FilterParameterUpdated
		);
    };

	//----------------------------------------------------------
	template <>
	class TSystemStatus<Messages> : public Server<Messages>, public TSystemStatusMessageDefinition
	{
	public:
		TSystemStatus<Messages>() : info(system::module::SystemStatus, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
		MessageInfo info;
	private:
		/// Konstanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		// Werte werden so festgelegt, dass alle Parameter und Ergebnisse Platz finden
		enum { sendBufLen  = (1*KBytes), replyBufLen = 100*Bytes, NumBuffers=32 };
	};


} // namespace interface
} // namespace precitec

#endif /*SYSTEM_STATUS_INTERFACE_H_*/

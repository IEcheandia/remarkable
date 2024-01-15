#ifndef RESULTS_HANDLER_H_
#define RESULTS_HANDLER_H_



#include "server/eventHandler.h"
#include "event/results.interface.h"

#include "event/results.h"


/// **************************
/// Der konkrete Handler wird von TResults<CallDirect> abgeleitet
/// Folgende Beispielklasse zeigt wie Resultate verarbeitet werden.
/// **************************

//	class MyResultHandler : TResults<CallDirect>
//	{
//		void MyResultHandler()
//		{
//			resultHandlerList.set(GapWidth, result<GapWidth, AResultArgsRange>);
//			resultHandlerList.set(Missmatch, result<Missmatch, ResultArgRange>);
//		}
//
//		template<>
//		void result<GapWidth, AResultArgsRange>(AResultArgsRange length)
//		{
//			resultHandlerList[Missmatch](value);
//			// gapWidth - Code
//		}
//
//		template<>
//		void result<Missmatch, ResultArgRange>(ResultArgRange length)
//		{
//			// gapWidth - Code
//		}
//	}


namespace precitec
{
namespace interface
{
	using namespace  system::message;

	template<>
	class TResults<EventServer> : public TResults<AbstractInterface>
	{
	protected:
		virtual void result(ResultIntArray		const& value) 	{	}
		virtual void result(ResultDoubleArray	const& value) 	{	}
		virtual void result(ResultRangeArray	const& value) 	{	}
		virtual void result(ResultRange1dArray	const& value)	{	}
		virtual void result(ResultPointArray	const& value) 	{	}
		virtual void nio(NIOResult const& result) {  }
		virtual void inspectionStarted(
			Poco::UUID productID,
			Poco::UUID instanceProductID,
			uint32_t productNr,
			MeasureTaskIDs measureTaskIDs,
			IntRange range,
			int p_oSeamNo,
			int triggerDeltaInMicrons,
			Poco::UUID seamId,
			const std::string &seamLabel) { }
		virtual void inspectionEnded(MeasureTaskIDs measureTaskIDs) { }
		virtual void inspectionAutomaticStart([[maybe_unused]] Poco::UUID productID, [[maybe_unused]] Poco::UUID instanceProductID, [[maybe_unused]] const std::string &extendedProductInfo) override { }
		virtual void inspectionAutomaticStop(Poco::UUID productID, Poco::UUID instanceProductID) { }

	};

	template <>
	class TResults<EventHandler> : public Server<EventHandler>, public TResultsMessageDefinition
	{
	public:
		EVENT_HANDLER( TResults );

	private:
			#define getServer() server_


	public:
		void registerCallbacks()
		{
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!!
            REGISTER_EVENT(ResultInt, result<ResultIntArray>);
            REGISTER_EVENT(ResultDouble, result<ResultDoubleArray>);
            REGISTER_EVENT(ResultRange, result<ResultRangeArray>);
            REGISTER_EVENT(ResultRange1d, result<ResultRange1dArray>);
            REGISTER_EVENT(ResultPoint, result<ResultPointArray>);
            REGISTER_EVENT(CombinedResults, combinedResults);

			REGISTER_EVENT(Nio, nio);
			REGISTER_EVENT(InspectionStarted, inspectionStarted);
			REGISTER_EVENT(InspectionEnded, inspectionEnded);
			REGISTER_EVENT(InspectionAutomaticStart, inspectionAutomaticStart);
			REGISTER_EVENT(InspectionAutomaticStop, inspectionAutomaticStop);
		}

        template <typename T>
        void result(Receiver &receiver)
        {
            T arg1;
            receiver.deMarshal(arg1);
            getServer()->result(arg1);
        }

        void combinedResults(Receiver &receiver)
        {
            std::vector<ResultDoubleArray> results;
            receiver.deMarshal(results);
            getServer()->result(results);
        }

		void nio(Receiver &receiver)
		{
			NIOResult arg1; receiver.deMarshal(arg1);
			getServer()->nio(arg1);
		}

		void inspectionStarted(Receiver &receiver)
		{
			Poco::UUID productID; receiver.deMarshal(productID);
			Poco::UUID instanceProductID; receiver.deMarshal(instanceProductID);
			uint32_t productNr; receiver.deMarshal(productNr);
			MeasureTaskIDs measureTaskIDs; receiver.deMarshal(measureTaskIDs);
			IntRange range; receiver.deMarshal(range);
			int p_oSeamNo; receiver.deMarshal(p_oSeamNo);
			int triggerDeltaInMicrons; receiver.deMarshal(triggerDeltaInMicrons);
			Poco::UUID seamId; receiver.deMarshal(seamId);
			std::string label; receiver.deMarshal(label);
			getServer()->inspectionStarted(productID,instanceProductID,productNr,measureTaskIDs,range,p_oSeamNo,triggerDeltaInMicrons, seamId, label);
		}

		void inspectionEnded(Receiver &receiver)
		{
			MeasureTaskIDs measureTaskIDs; receiver.deMarshal(measureTaskIDs);
			getServer()->inspectionEnded(measureTaskIDs);
		}

		void inspectionAutomaticStart(Receiver &receiver){

			Poco::UUID productID; receiver.deMarshal(productID);
			Poco::UUID instanceProductID; receiver.deMarshal(instanceProductID);
			std::string extendedProductInfo; receiver.deMarshal(extendedProductInfo);
			getServer()->inspectionAutomaticStart(productID,instanceProductID, extendedProductInfo);
		}

		void inspectionAutomaticStop(Receiver &receiver){

			Poco::UUID productID; receiver.deMarshal(productID);
			Poco::UUID instanceProductID; receiver.deMarshal(instanceProductID);
			getServer()->inspectionAutomaticStop(productID,instanceProductID);
		}


	};


} // namespace interface
} // namespace precitec

#ifdef getServer
#undef getServer
#endif


#endif /*RESULTS_HANDLER_H_*/

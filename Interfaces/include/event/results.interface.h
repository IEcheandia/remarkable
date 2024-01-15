#ifndef RESULTS_INTERFACE_H_
#define RESULTS_INTERFACE_H_

#include "event/results.h"
#include "server/interface.h"
#include "module/interfaces.h"
#include "message/serializer.h"
#include "Poco/UUID.h"

#include "geo/point.h"
#include "geo/geo.h" // Geo-Context




/*
 * Hier werden die abstrakten Baisklassen, sowie die
 * Messages definiert. Dieser Header wird sowohl fuer
 * den lokalen (Implementation, MsgHandler) sowie den
 * remote-Teil (Proxyer) benoetigt.
 */
namespace precitec
{
	using namespace  system::message;
	using system::module::Results;

namespace interface
{
	/// Vorwaertsdeklaration des generellen Templates das hier Spezialisiert wird
	template <int mode>
	class TResults;


	// Schnittstelle Result Event
	template <>
	class TResults<AbstractInterface> {
	public:
		TResults() {}
		virtual ~TResults() {}
		virtual void result(ResultIntArray const& value) = 0;
		virtual void result(ResultDoubleArray const& value) = 0;
		virtual void result(ResultRangeArray const& value) = 0;
		virtual void result(ResultRange1dArray const& value) = 0;
		virtual void result(ResultPointArray const& value) = 0;
		virtual void nio(NIOResult const& result) = 0; // TODO: nio array
		virtual void inspectionStarted(Poco::UUID productID, Poco::UUID instanceProductID, uint32_t productNr, MeasureTaskIDs measureTaskIDs, IntRange range, int p_oSeamNo, int triggerDeltaInMicrons, Poco::UUID seamId, const std::string &seamLabel) = 0;
		virtual void inspectionEnded(MeasureTaskIDs measureTaskIDs) = 0;
		virtual void inspectionAutomaticStart(Poco::UUID productID, Poco::UUID instanceProductID, const std::string &extendedProductInfo) = 0;
		virtual void inspectionAutomaticStop(Poco::UUID productID, Poco::UUID instanceProductID) = 0;

        virtual void result(const std::vector<ResultDoubleArray> &results)
        {
            for (auto r : results)
            {
                result(r);
            }
        }
	};

    struct TResultsMessageDefinition
    {
		EVENT_MESSAGE(ResultInt, ResultIntArray);
		EVENT_MESSAGE(ResultDouble, ResultDoubleArray);
		EVENT_MESSAGE(ResultRange, ResultRangeArray);
		EVENT_MESSAGE(ResultRange1d, ResultRange1dArray);
		EVENT_MESSAGE(ResultPoint, ResultPointArray);
		EVENT_MESSAGE(Nio, NIOResult);
		EVENT_MESSAGE(InspectionStarted, Poco::UUID, Poco::UUID, uint32_t, MeasureTaskIDs, IntRange, int, int, Poco::UUID, std::string);
		EVENT_MESSAGE(InspectionEnded, MeasureTaskIDs);
		EVENT_MESSAGE(InspectionAutomaticStart, Poco::UUID, Poco::UUID, std::string);
		EVENT_MESSAGE(InspectionAutomaticStop, Poco::UUID, Poco::UUID);
        EVENT_MESSAGE(CombinedResults, std::vector<ResultDoubleArray>);

        MESSAGE_LIST(
			ResultInt,
			ResultDouble,
			ResultRange,
			ResultRange1d,
			ResultPoint,
			Nio,
			InspectionStarted,
			InspectionEnded,
			InspectionAutomaticStart,
			InspectionAutomaticStop,
            CombinedResults
		);
    };

	//----------------------------------------------------------
	template <>
	class TResults<Messages> : public Server<Messages>, public TResultsMessageDefinition
	{
	private:
		typedef Poco::UUID PocoUUID; // for macros

		/// Konstanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024}; /// todo: anpassen
		enum { sendBufLen  = 512*KBytes, replyBufLen = 100*Bytes, NumBuffers=512 };

	public:
		TResults() : info(Results, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
		MessageInfo info;

	};


} // namespace interface
} // namespace precitec


#endif /*RESULTS_INTERFACE_H_*/

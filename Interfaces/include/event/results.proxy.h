#ifndef RESULTS_PROXY_H_
#define RESULTS_PROXY_H_

#include <string> // std::string

#include "server/eventProxy.h"
#include "event/results.interface.h"


namespace precitec
{
namespace interface
{

	template <>
	class TResults<EventProxy> : public Server<EventProxy>, public TResults<AbstractInterface>, public TResultsMessageDefinition
	{

	public:
		TResults() : EVENT_PROXY_CTOR(TResults), TResults<AbstractInterface>()
		{
			//std::cout << "remote CTor::TRegistrar<Proxy> ohne Protokoll" << std::endl;
		}
	public:
		void result(ResultIntArray const& value) override
		{
			INIT_EVENT(ResultInt);
			signaler().marshal(value);
			signaler().send();
		}
		void result(ResultDoubleArray const& value) override
		{
			INIT_EVENT(ResultDouble);
			signaler().marshal(value);
			signaler().send();
		}
		void result(ResultRangeArray const& value) override
		{
			INIT_EVENT(ResultRange);
			signaler().marshal(value);
			signaler().send();
		}
		void result(ResultRange1dArray const& value) override
		{
			INIT_EVENT(ResultRange1d);
			signaler().marshal(value);
			signaler().send();
		}
		void result(ResultPointArray const& value) override
		{
			INIT_EVENT(ResultPoint);
			signaler().marshal(value);
			signaler().send();
		}

		void nio(NIOResult const& value) override
		{
			INIT_EVENT(Nio);
			signaler().marshal(value);
			signaler().send();
		}

		void inspectionStarted(Poco::UUID productID, Poco::UUID instanceProductID, uint32_t productNr, MeasureTaskIDs measureTaskIDs, IntRange range, int p_oSeamNo, int triggerDeltaInMicrons, Poco::UUID seamId, const std::string &seamLabel) override
		{
			INIT_EVENT(InspectionStarted);
			signaler().marshal(productID);
			signaler().marshal(instanceProductID);
			signaler().marshal(productNr);
			signaler().marshal(measureTaskIDs);
			signaler().marshal(range);
			signaler().marshal(p_oSeamNo);
			signaler().marshal(triggerDeltaInMicrons);
			signaler().marshal(seamId);
			signaler().marshal(seamLabel);
			signaler().send();
		}
		void inspectionEnded(MeasureTaskIDs measureTaskIDs) override
		{
			INIT_EVENT(InspectionEnded);
			signaler().marshal(measureTaskIDs);
			signaler().send();
		}
		void inspectionAutomaticStart(Poco::UUID productID, Poco::UUID instanceProductID, const std::string &extendedProductInfo) override
		{
			INIT_EVENT(InspectionAutomaticStart);
			signaler().marshal(productID);
			signaler().marshal(instanceProductID);
			signaler().marshal(extendedProductInfo);
			signaler().send();
		}
		void inspectionAutomaticStop(Poco::UUID productID, Poco::UUID instanceProductID) override
		{
			INIT_EVENT(InspectionAutomaticStop);
			signaler().marshal(productID);
			signaler().marshal(instanceProductID);
			signaler().send();
		}

        void result(const std::vector<ResultDoubleArray> &results) override
        {
            INIT_EVENT(CombinedResults);
            signaler().marshal(results);
            signaler().send();
        }
	};

} // namespace interface
} // namespace precitec


#endif /*RESULTS_PROXY_H_*/

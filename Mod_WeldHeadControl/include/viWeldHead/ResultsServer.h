/*
 * ResultsServer.h
 *
 *  Created on: 20.06.2019
 *      Author: a. egger
 */

#pragma once

#include "event/results.handler.h"

#include "event/viWeldHeadSubscribe.interface.h"

namespace precitec
{

namespace viCommunicator
{

    using namespace interface;

/**
 * ResultServer, hier schlagen die Resultete auf.
 * Steuert die HW ueber die empfangenen Resultate an
 *
 * @attention Hier schlagen ALLE abgesendeten Reultate auf (parallel zum Windows Host) -> PERFORMANCE?
 *
 **/
class ResultsServer : public TResults<AbstractInterface>
{
public:

    ResultsServer(TviWeldHeadSubscribe<AbstractInterface>& p_rWeldHeadSubscribeProxy);
    virtual ~ResultsServer();

    void setResultsProxy(const std::shared_ptr<interface::TResults<interface::AbstractInterface>> &proxy)
    {
        m_resultsProxy = proxy;
    }

private:

    void result(interface::ResultIntArray const&) override;
    void result(interface::ResultDoubleArray const&) override;
    void result(interface::ResultRangeArray const&) override;
    void result(interface::ResultRange1dArray const&) override;
    void result(interface::ResultPointArray const&) override;
    void nio(interface::NIOResult const&) override;
    void result(const std::vector<ResultDoubleArray> &results) override;

    void inspectionAutomaticStart(Poco::UUID productID, Poco::UUID instanceProductID, const std::string &extendedProductInfo) override;
    void inspectionAutomaticStop(Poco::UUID productID, Poco::UUID instanceProductID) override;

    void inspectionStarted(Poco::UUID productID, Poco::UUID instanceProductID, uint32_t productNr, MeasureTaskIDs measureTaskIDs, IntRange range, int p_oSeamNo , int triggerDeltaInMicrons, Poco::UUID seamId, const std::string &seamLabel) override;
    void inspectionEnded(MeasureTaskIDs measureTaskIDs) override;

    TviWeldHeadSubscribe<AbstractInterface>& m_rWeldHeadSubscribeProxy;
    std::shared_ptr<interface::TResults<interface::AbstractInterface>> m_resultsProxy;

};

} // namespece precitec
} // namespace viCommunicator


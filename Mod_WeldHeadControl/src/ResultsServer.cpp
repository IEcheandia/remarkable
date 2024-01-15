/*
 * ResultsServer.cpp
 *
 *  Created on: 20.06.2019
 *      Author: a. egger
 */

#include <cstdio>
#include <string>
#include <system/timer.h>

#include <module/moduleLogger.h>
#include "common/systemConfiguration.h"
#include "common/defines.h"

#include "viWeldHead/ResultsServer.h"

using namespace precitec::interface;

namespace precitec
{
    using namespace interface;
    using precitec::filter::ValueRankType;

namespace viCommunicator
{

#define S6K_RESULT_TEST 0

ResultsServer::ResultsServer(TviWeldHeadSubscribe<AbstractInterface>& p_rWeldHeadSubscribeProxy):
        m_rWeldHeadSubscribeProxy(p_rWeldHeadSubscribeProxy)
{
}

ResultsServer::~ResultsServer()
{
}

void ResultsServer::result(interface::ResultIntArray const& rValue)
{
#if !defined(NDEBUG)
    wmLog(eDebug, "ResultIntArray: %d\n", rValue.resultType() );
#endif
}

void ResultsServer::result(interface::ResultDoubleArray const& rValue)
{
    system::Timer oTimer; oTimer.start();

#if !defined(NDEBUG)
    if (rValue.isValid())
    {
        wmLog(eDebug, "ResultDoubleArray: %d , %f\n", rValue.resultType(), rValue.value().front());
    }
#endif

    if(rValue.isValid() && rValue.resultType() == LD50_Y_AbsolutePosition && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rWeldHeadSubscribeProxy.SetHeadValue(interface::eAxisY, rValue.value().front(), Position_Absolute);
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent LD50_Y_AbsolutePosition(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
    }
    if(rValue.isValid() && rValue.resultType() == LD50_Y_RelativePosition && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rWeldHeadSubscribeProxy.SetHeadValue(interface::eAxisY, rValue.value().front(), Position_Relative);
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent LD50_Y_RelativePosition(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
    }
    if(rValue.isValid() && rValue.resultType() == LD50_Z_AbsolutePosition && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rWeldHeadSubscribeProxy.SetHeadValue(interface::eAxisZ, rValue.value().front(), Position_Absolute);
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent LD50_Z_AbsolutePosition(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
    }
    if(rValue.isValid() && rValue.resultType() == LD50_Z_RelativePosition && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rWeldHeadSubscribeProxy.SetHeadValue(interface::eAxisZ, rValue.value().front(), Position_Relative);
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent LD50_Z_RelativePosition(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
    }

    if(rValue.isValid() && rValue.resultType() == ScanTracker_Amplitude && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rWeldHeadSubscribeProxy.SetTrackerScanWidthControlled( (rValue.value().front() * 1000) ); // in um weiterleiten
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent ScanTracker_Amplitude(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
    }
    if(rValue.isValid() && rValue.resultType() == ScanTracker_Offset && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rWeldHeadSubscribeProxy.SetTrackerScanPosControlled( (rValue.value().front() * 1000) ); // in um weiterleiten (fuer Positionsregelung Scantracker)
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent ScanTracker_Offset(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
    }
    if(rValue.isValid() && rValue.resultType() == LaserControl_PowerOffset && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rWeldHeadSubscribeProxy.SetLCPowerOffset( (rValue.value().front() * 1000) ); // in um weiterleiten
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent LaserControl_PowerOffset(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
    }
    if(rValue.isValid() && rValue.resultType() == Z_Collimator_Position && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rWeldHeadSubscribeProxy.doZCollDriving( eRelativeDriving, (rValue.value().front() * 1000) ); // in um weiterleiten (fuer Positionsregelung Z-Collimator)
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent Z_Collimator_Position(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
    }
    if(rValue.isValid() && rValue.resultType() == GeneralPurpose_AnalogOut1 && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rWeldHeadSubscribeProxy.SetGenPurposeAnaOut( eOutput1, (rValue.value().front() * 1000) ); // Double Wert (-10.0 bis +10.0) als Int Wert weiterleiten
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent GeneralPurpose_AnalogOut1(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
    }
    if(rValue.isValid() && rValue.resultType() == GeneralPurpose_AnalogOut2 && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rWeldHeadSubscribeProxy.SetGenPurposeAnaOut( eOutput2, (rValue.value().front() * 1000) ); // Double Wert (-10.0 bis +10.0) als Int Wert weiterleiten
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent GeneralPurpose_AnalogOut2(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
    }
    if(rValue.isValid() && rValue.resultType() == GeneralPurpose_AnalogOut3 && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rWeldHeadSubscribeProxy.SetGenPurposeAnaOut( eOutput3, (rValue.value().front() * 1000) ); // Double Wert (-10.0 bis +10.0) als Int Wert weiterleiten
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent GeneralPurpose_AnalogOut3(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
    }
    if(rValue.isValid() && rValue.resultType() == GeneralPurpose_AnalogOut4 && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rWeldHeadSubscribeProxy.SetGenPurposeAnaOut( eOutput4, (rValue.value().front() * 1000) ); // Double Wert (-10.0 bis +10.0) als Int Wert weiterleiten
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent GeneralPurpose_AnalogOut4(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
    }
    if(rValue.isValid() && rValue.resultType() == GeneralPurpose_AnalogOut5 && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rWeldHeadSubscribeProxy.SetGenPurposeAnaOut( eOutput5, (rValue.value().front() * 1000) ); // Double Wert (-10.0 bis +10.0) als Int Wert weiterleiten
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent GeneralPurpose_AnalogOut5(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
    }
    if(rValue.isValid() && rValue.resultType() == GeneralPurpose_AnalogOut6 && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rWeldHeadSubscribeProxy.SetGenPurposeAnaOut( eOutput6, (rValue.value().front() * 1000) ); // Double Wert (-10.0 bis +10.0) als Int Wert weiterleiten
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent GeneralPurpose_AnalogOut6(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
    }
    if(rValue.isValid() && rValue.resultType() == GeneralPurpose_AnalogOut7 && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rWeldHeadSubscribeProxy.SetGenPurposeAnaOut( eOutput7, (rValue.value().front() * 1000) ); // Double Wert (-10.0 bis +10.0) als Int Wert weiterleiten
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent GeneralPurpose_AnalogOut7(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
    }
    if(rValue.isValid() && rValue.resultType() == GeneralPurpose_AnalogOut8 && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rWeldHeadSubscribeProxy.SetGenPurposeAnaOut( eOutput8, (rValue.value().front() * 1000) ); // Double Wert (-10.0 bis +10.0) als Int Wert weiterleiten
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent GeneralPurpose_AnalogOut8(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
    }
    if(rValue.isValid() && (rValue.resultType() == ScanmasterSeamWelding || rValue.resultType() == ScanmasterSeamWeldingAndEndOfSeamMarker) && rValue.rank().front() == ValueRankType::eRankMax)
    {
#if 1

        wmLog(eDebug, "11 resultType: ScanmasterSeamWelding, imageNumber: %d, position: %d, size: %d, value[0]: %f\n",
                rValue.context().imageNumber(), rValue.context().position(), rValue.value().size(), rValue.value()[0]);
        m_rWeldHeadSubscribeProxy.ScanmasterResult( rValue.resultType() == ScanmasterSeamWelding ? eScanmasterSeamWelding : eScanmasterSeamWeldingAndEndOfSeamMarker, rValue );
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent ScanmasterSeamWelding(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
#else
        if (rValue.context().imageNumber() == 0)
        {
            wmLog(eDebug, "11 resultType: ScanmasterSeamWelding, imageNumber: %d, position: %d, size: %d, value[0]: %f\n",
                rValue.context().imageNumber(), rValue.context().position(), rValue.value().size(), rValue.value()[0]);
            // neu erstellen
            precitec::geo2d::TArray<double> oValues;
            oValues.resize(12);
            oValues.getData().data()[0] =  -50.0; // x1
            oValues.getData().data()[1] =   10.0; // y1
            oValues.getData().data()[2] =    0.0; // laser power/programm mode
            oValues.getData().data()[3] =   80.0; // velocity
            oValues.getData().data()[4] =    0.0; // x2
            oValues.getData().data()[5] =   10.0; // y2
            oValues.getData().data()[6] =    0.0; // laser power/programm mode
            oValues.getData().data()[7] =   80.0; // velocity
            oValues.getData().data()[8] =   50.0; // x3
            oValues.getData().data()[9] =   10.0; // y3
            oValues.getData().data()[10] =   0.0; // laser power/programm mode
            oValues.getData().data()[11] =  80.0; // velocity
            GeoDoublearray oGeoValueOut = GeoDoublearray {rValue.context(), oValues};
            geo2d::Range1d m_oMinMaxRange( -1000000.0, 1000000.0 );
            interface::ResultDoubleArray oNewRValue(
                rValue.filterId(),
                rValue.resultType(),
                rValue.nioType(),
                rValue.context(),
                oGeoValueOut,
                m_oMinMaxRange,
                rValue.isNio()
            );
            // neu erstelltes senden
            m_rWeldHeadSubscribeProxy.ScanmasterResult(eScanmasterSeamWelding, oNewRValue);
            oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent ScanmasterSeamWelding(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
        }
#endif
    }
    if(rValue.isValid() && rValue.resultType() == ScanmasterScannerMoving && rValue.rank().front() == ValueRankType::eRankMax)
    {
#if 1
        wmLog(eDebug, "11 resultType: ScanmasterScannerMoving, imageNumber: %d, position: %d, size: %d, value[0]: %f\n",
              rValue.context().imageNumber(), rValue.context().position(), rValue.value().size(), rValue.value()[0]);
        m_rWeldHeadSubscribeProxy.ScanmasterResult( eScanmasterScannerMoving, rValue );
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent ScanmasterScannerMoving(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
#else
        wmLog(eDebug, "11 resultType: ScanmasterScannerMoving, imageNumber: %d, position: %d, size: %d, value[0]: %f\n",
            rValue.context().imageNumber(), rValue.context().position(), rValue.value().size(), rValue.value()[0]);
        // neu erstellen
        precitec::geo2d::TArray<double> oValues;
        oValues.resize(2);
        oValues.getData().data()[0] = -1.0 - rValue.context().imageNumber(); // x
        oValues.getData().data()[1] = 1.0 + rValue.context().imageNumber(); // y
        GeoDoublearray oGeoValueOut = GeoDoublearray {rValue.context(), oValues};
        geo2d::Range1d m_oMinMaxRange( -1000000.0, 1000000.0 );
        interface::ResultDoubleArray oNewRValue(
            rValue.filterId(),
            rValue.resultType(),
            rValue.nioType(),
            rValue.context(),
            oGeoValueOut,
            m_oMinMaxRange,
            rValue.isNio()
        );
        // neu erstelltes senden
        m_rWeldHeadSubscribeProxy.ScanmasterResult(eScanmasterScannerMoving, oNewRValue);
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent ScanmasterScannerMoving(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
#endif
    }

    if(rValue.isValid() && rValue.resultType() == ScanmasterSpotWelding && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rWeldHeadSubscribeProxy.ScanmasterResult(eScanmasterSpotWelding, rValue);
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent ScanmasterScannerSpotWelding(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
    }
    if (rValue.isValid() && rValue.resultType() == PrepareContour && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rWeldHeadSubscribeProxy.ScanmasterResult(eScanmasterPrepareContour, rValue);
        oTimer.elapsed();
        if (oTimer.ms() > 0)
        {
            wmLog(eDebug, "Sent prepare contour(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms());
        }
    }
    if(rValue.isValid() && rValue.resultType() == WeldPreparedContour && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rWeldHeadSubscribeProxy.ScanmasterResult(eScanmasterWeldPreparedContour, rValue);
        oTimer.elapsed();
        if (oTimer.ms() > 0)
        {
            wmLog(eDebug, "Sent weld prepared contour(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() );
        }
    }
    if(rValue.isValid() && rValue.resultType() == ScanmasterHeight && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rWeldHeadSubscribeProxy.ScanmasterHeight(rValue.value().front());
        oTimer.elapsed();
        if (oTimer.ms() > 0)
        {
            wmLog(eDebug, "Sent ScanmasterHeight(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() );
        }
    }

    oTimer.elapsed(); if ( oTimer.ms() > 5 ) { wmLog(eWarning, "Warning sending of result %d - blocked for %d ms\n", rValue.resultType(), oTimer.ms() ); }
}

void ResultsServer::result(const std::vector<ResultDoubleArray> &results)
{
    std::vector<ResultDoubleArray> delayed;
    for (auto r : results)
    {
        if (r.resultType() == ScanmasterSeamWelding || r.resultType() == ScanmasterScannerMoving || r.resultType() == ScanmasterSeamWeldingAndEndOfSeamMarker)
        {
            delayed.push_back(r);
            continue;
        }
        result(r);
    }
    for (auto r : delayed)
    {
        result(r);
    }
}

void ResultsServer::result(interface::ResultRangeArray const& rValue)
{
#if !defined(NDEBUG)
    wmLog(eDebug, "ResultRangeArray: %d\n", rValue.resultType() );
#endif
}

void ResultsServer::result(interface::ResultRange1dArray const& rValue)
{
#if !defined(NDEBUG)
    wmLog(eDebug, "ResultRange1dArray: %d\n", rValue.resultType() );
#endif
}

void ResultsServer::result(interface::ResultPointArray const& rValue)
{
#if !defined(NDEBUG)
    wmLog(eDebug, "ResultPointArray: %d\n", rValue.resultType() );
#endif
}

void ResultsServer::nio(interface::NIOResult const& rError)
{
}

void ResultsServer::inspectionAutomaticStart([[maybe_unused]] Poco::UUID productID, [[maybe_unused]] Poco::UUID instanceProductID, [[maybe_unused]] const std::string &extendedProductInfo)
{
}

void ResultsServer::inspectionAutomaticStop(Poco::UUID productID, Poco::UUID instanceProductID)
{
}

void ResultsServer::inspectionStarted(Poco::UUID productID, Poco::UUID instanceProductID, uint32_t productNr,
                                      MeasureTaskIDs measureTaskIDs, IntRange range, int p_oSeamNo, int triggerDeltaInMicrons, Poco::UUID seamId, UNUSED const std::string &seamLabel )
{
}

void ResultsServer::inspectionEnded(MeasureTaskIDs measureTaskIDs)
{
}

} // namespace viCommunicator
} // namespace precitec


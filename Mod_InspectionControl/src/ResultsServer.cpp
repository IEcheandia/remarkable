/*
 * ResultsServer.cpp
 *
 *  Created on: 03.04.2017
 *      Author: a.egger
 */

#include <module/moduleLogger.h>
#include "common/defines.h"

#include "viInspectionControl/ResultsServer.h"

namespace precitec
{
	using namespace interface;
    using precitec::filter::ValueRankType;

namespace ethercat
{

#define  BIT0  0x00000001
#define  BIT1  0x00000002
#define  BIT2  0x00000004
#define  BIT3  0x00000008
#define  BIT4  0x00000010
#define  BIT5  0x00000020
#define  BIT6  0x00000040
#define  BIT7  0x00000080
#define  BIT8  0x00000100
#define  BIT9  0x00000200
#define  BIT10 0x00000400
#define  BIT11 0x00000800
#define  BIT12 0x00001000
#define  BIT13 0x00002000
#define  BIT14 0x00004000
#define  BIT15 0x00008000
#define  BIT16 0x00010000
#define  BIT17 0x00020000
#define  BIT18 0x00040000
#define  BIT19 0x00080000
#define  BIT20 0x00100000
#define  BIT21 0x00200000
#define  BIT22 0x00400000
#define  BIT23 0x00800000
#define  BIT24 0x01000000
#define  BIT25 0x02000000
#define  BIT26 0x04000000
#define  BIT27 0x08000000
#define  BIT28 0x10000000
#define  BIT29 0x20000000
#define  BIT30 0x40000000
#define  BIT31 0x80000000

ResultsServer::ResultsServer(VI_InspectionControl& p_rVI_InspectionControl):
		m_rVI_InspectionControl(p_rVI_InspectionControl),
		m_oResultCounter(0),
		m_oOK(false),
        m_oSeamNoIndex(0),
        m_oQualityErrorAccu(0x0),
        m_oQualityErrorCat1Accu(0x0),
        m_oQualityErrorCat2Accu(0x0),
        m_oFirstErrorPosition(-1),
        m_oElementWidthSum(0),
        m_oElementWidthCounter(0),
        m_oHeightDiffSum(0),
        m_oHeightDiffCounter(0),
        m_oConcavitySum(0),
        m_oConcavityCounter(0),
        m_oConvexitySum(0),
        m_oConvexityCounter(0),
        m_oFirstPoreWidth(-1),
        m_oS6K_CS_ActualResultBlock(0),
        m_oS6K_CS_MaxResultCountInBlock(CS_MEASURE_COUNT_PER_TCP_BLOCK),
        m_oS6K_CS_CurrentNoResult {},
        m_oS6K_CS_ResultCountPerImage {},
        m_oS6K_CS_BlockNo(0),
        m_oS6K_CS_MeasuresPerResult(1),
        m_oS6K_CS_SeamStartFlag(false)
{
    // SystemConfig Switches for SOUVIS6000 application and functions
    m_oIsSOUVIS6000_Application = SystemConfiguration::instance().getBool("SOUVIS6000_Application", false);
    wmLog(eDebug, "m_oIsSOUVIS6000_Application (bool): %d\n", m_oIsSOUVIS6000_Application);
    m_oSOUVIS6000_Is_PreInspection = SystemConfiguration::instance().getBool("SOUVIS6000_Is_PreInspection", false);
    wmLog(eDebug, "m_oSOUVIS6000_Is_PreInspection (bool): %d\n", m_oSOUVIS6000_Is_PreInspection);
    m_oSOUVIS6000_Is_PostInspection_Top = SystemConfiguration::instance().getBool("SOUVIS6000_Is_PostInspection_Top", false);
    wmLog(eDebug, "m_oSOUVIS6000_Is_PostInspection_Top (bool): %d\n", m_oSOUVIS6000_Is_PostInspection_Top);
    m_oSOUVIS6000_Is_PostInspection_Bottom = SystemConfiguration::instance().getBool("SOUVIS6000_Is_PostInspection_Bottom", false);
    wmLog(eDebug, "m_oSOUVIS6000_Is_PostInspection_Bottom (bool): %d\n", m_oSOUVIS6000_Is_PostInspection_Bottom);
    m_oSOUVIS6000_CrossSectionMeasurementEnable = SystemConfiguration::instance().getBool("SOUVIS6000_CrossSectionMeasurementEnable", false);
    wmLog(eDebug, "m_oSOUVIS6000_CrossSectionMeasurementEnable (bool): %d\n", m_oSOUVIS6000_CrossSectionMeasurementEnable);
    m_oSOUVIS6000_CrossSection_Leading_System = SystemConfiguration::instance().getBool("SOUVIS6000_CrossSection_Leading_System", false);
    wmLog(eDebug, "m_oSOUVIS6000_CrossSection_Leading_System (bool): %d\n", m_oSOUVIS6000_CrossSection_Leading_System);

    for(int i = 0;i < CS_MAX_RESULT_BLOCKS;i++)
    {
        for(int j = 0;j < CS_MEASURE_COUNT_PER_TCP_BLOCK;j++)
        {
            for(int k = 0;k < CS_VALUES_PER_MEASURE;k++)
            {
                m_oS6K_CS_ResultBlock[i][j][k] = 0;
            }
        }
        m_oS6K_CS_FirstImageInBlock[i] = 0;
    }
}

ResultsServer::~ResultsServer()
{
}

void ResultsServer::result(interface::ResultIntArray const& rValue)
{
	m_oResultCounter++;
	if (m_oResultCounter >= NUMBER_RESULTS_FOR_OK)
	{
		if (m_oOK == false) m_rVI_InspectionControl.ResultsReceivedFlag(true);
		m_oOK = true;
	}
}

void ResultsServer::result(interface::ResultDoubleArray const& rValue)
{
    system::Timer oTimer; oTimer.start();

	m_oResultCounter++;
	if (m_oResultCounter >= NUMBER_RESULTS_FOR_OK)
	{
		if (m_oOK == false) m_rVI_InspectionControl.ResultsReceivedFlag(true);
		m_oOK = true;
	}

    if (m_oS6K_CS_SeamStartFlag)
    {
        m_oS6K_CS_StartTime = std::chrono::steady_clock::now();
        m_oS6K_CS_SeamStartFlag = false;
    }

    if(rValue.isValid() && rValue.resultType() == Fieldbus_PositionResult && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rVI_InspectionControl.setPositionResultsField( static_cast<int>(rValue.value().front()) );
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent Fieldbus_PositionResult(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
    }
    if(rValue.isValid() && rValue.resultType() == GeneralPurpose_DigitalOut1 && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rVI_InspectionControl.setGenPurposeDigOut( eOutput1, static_cast<int>(rValue.value().front()) );
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent setGenPurposeDigOut1(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
    }
    if(rValue.isValid() && rValue.resultType() == GeneralPurpose_DigitalOut2 && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rVI_InspectionControl.setGenPurposeDigOut( eOutput2, static_cast<int>(rValue.value().front()) );
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent setGenPurposeDigOut2(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
    }
    if(rValue.isValid() && rValue.resultType() == GeneralPurpose_DigitalOut3 && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rVI_InspectionControl.setGenPurposeDigOut( eOutput3, static_cast<int>(rValue.value().front()) );
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent setGenPurposeDigOut3(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
    }
    if(rValue.isValid() && rValue.resultType() == GeneralPurpose_DigitalOut4 && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rVI_InspectionControl.setGenPurposeDigOut( eOutput4, static_cast<int>(rValue.value().front()) );
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent setGenPurposeDigOut4(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
    }
    if(rValue.isValid() && rValue.resultType() == GeneralPurpose_DigitalOut5 && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rVI_InspectionControl.setGenPurposeDigOut( eOutput5, static_cast<int>(rValue.value().front()) );
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent setGenPurposeDigOut5(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
    }
    if(rValue.isValid() && rValue.resultType() == GeneralPurpose_DigitalOut6 && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rVI_InspectionControl.setGenPurposeDigOut( eOutput6, static_cast<int>(rValue.value().front()) );
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent setGenPurposeDigOut6(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
    }
    if(rValue.isValid() && rValue.resultType() == GeneralPurpose_DigitalOut7 && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rVI_InspectionControl.setGenPurposeDigOut( eOutput7, static_cast<int>(rValue.value().front()) );
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent setGenPurposeDigOut7(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
    }
    if(rValue.isValid() && rValue.resultType() == GeneralPurpose_DigitalOut8 && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rVI_InspectionControl.setGenPurposeDigOut( eOutput8, static_cast<int>(rValue.value().front()) );
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent setGenPurposeDigOut8(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
    }
    if(rValue.isValid() && rValue.resultType() == EndOfSeamMarker && rValue.rank().front() == ValueRankType::eRankMax)
    {
        m_rVI_InspectionControl.SM_SetEndOfSeamMarker(rValue.context().taskContext().measureTask()->seam());
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent SM_SetEndOfSeamMarker(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
    }
     if(rValue.isValid() && rValue.resultType() == ScanmasterSpotWelding && rValue.rank().front() == ValueRankType::eRankMax)
     {
        m_rVI_InspectionControl.SM_SetSpotWelding();
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent SM_SetSpotWelding(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
     }
    // in case of SOUVIS6000 tracking results there should be no check for valid rank. inspection graph can not guarantee a good rank for this results.
    if(rValue.isValid() && rValue.resultType() == S6K_EdgePosition)
    {
#if !S6K_RESULT_TEST
        // this is the normal procedure if value is valid
        m_rVI_InspectionControl.setS6K_EdgePosition( (rValue.value().front() * 1000),  rValue.context().imageNumber(), rValue.context().position()); // in um weiterleiten (fuer Ausgabe ueber Feldbus)
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent setS6K_EdgePosition(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
#else
        // this is the procedure if S6K_RESULT_TEST is active
        static int oValue = 115;
        if (rValue.context().imageNumber() == 0)
        {
            oValue = 115;
        }
        m_rVI_InspectionControl.setS6K_EdgePosition( oValue,  rValue.context().imageNumber(), rValue.context().position()); // in um weiterleiten (fuer Ausgabe ueber Feldbus)
        oValue += 1;
#endif
    }
    // in case of SOUVIS6000 tracking results there should be no check for valid rank. inspection graph can not guarantee a good rank for this results.
    if(rValue.isValid() && rValue.resultType() == S6K_GapWidth)
    {
#if !S6K_RESULT_TEST
        // this is the normal procedure if value is valid
        m_rVI_InspectionControl.setS6K_GapWidth( (rValue.value().front() * 1000),  rValue.context().imageNumber(), rValue.context().position()); // in um weiterleiten (fuer Ausgabe ueber Feldbus)
        m_oElementWidthSum += rValue.value().front() * 1000;
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent setS6K_GapWidth(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
#else
        // this is the procedure if S6K_RESULT_TEST is active
        static int oValue = 42;
        if (rValue.context().imageNumber() == 0)
        {
            oValue = 42;
        }
        m_rVI_InspectionControl.setS6K_GapWidth( oValue,  rValue.context().imageNumber(), rValue.context().position()); // in um weiterleiten (fuer Ausgabe ueber Feldbus)
        m_oElementWidthSum += oValue;
        oValue += 1;
#endif
        m_oElementWidthCounter++;
    }
    if(rValue.isValid() && rValue.resultType() == S6K_SeamWidth && rValue.rank().front() == ValueRankType::eRankMax)
    {
#if !S6K_RESULT_TEST
        // this is the normal procedure if value is valid and rank is eRankMax
        m_oElementWidthSum += rValue.value().front() * 1000;
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "S6K inspection result(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
#else
        // this is the procedure if S6K_RESULT_TEST is active
        static int oValue = 55;
        if (rValue.context().imageNumber() == 0)
        {
            oValue = 55;
        }
        m_oElementWidthSum += oValue;
        oValue += 1;
#endif
        m_oElementWidthCounter++;
    }
    if(rValue.isValid() && rValue.resultType() == S6K_HeightDifference && rValue.rank().front() == ValueRankType::eRankMax)
    {
#if !S6K_RESULT_TEST
        // this is the normal procedure if value is valid and rank is eRankMax
        m_oHeightDiffSum += rValue.value().front() * 1000;
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent S6K inspection result(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
#else
        // this is the procedure if S6K_RESULT_TEST is active
        static int oValue = 30 + ((m_oSeamNoIndex + 1) * 100);
        if (rValue.context().imageNumber() == 0)
        {
            oValue = 30 + ((m_oSeamNoIndex + 1) * 100);
        }
        m_oHeightDiffSum += oValue;
#endif
        m_oHeightDiffCounter++;
    }
    if(rValue.isValid() && rValue.resultType() == S6K_Concavity && rValue.rank().front() == ValueRankType::eRankMax)
    {
#if !S6K_RESULT_TEST
        // this is the normal procedure if value is valid and rank is eRankMax
        m_oConcavitySum += rValue.value().front() * 1000;
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent S6K inspection result(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
#else
        // this is the procedure if S6K_RESULT_TEST is active
        static int oValue = 20 + ((m_oSeamNoIndex + 1) * 100);
        if (rValue.context().imageNumber() == 0)
        {
            oValue = 20 + ((m_oSeamNoIndex + 1) * 100);
        }
        m_oConcavitySum += oValue;
#endif
        m_oConcavityCounter++;
    }
    if(rValue.isValid() && rValue.resultType() == S6K_Convexity && rValue.rank().front() == ValueRankType::eRankMax)
    {
#if !S6K_RESULT_TEST
        // this is the normal procedure if value is valid and rank is eRankMax
        m_oConvexitySum += rValue.value().front() * 1000;
        oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent S6K inspection result(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
#else
        // this is the procedure if S6K_RESULT_TEST is active
        static int oValue = 50;
        if (rValue.context().imageNumber() == 0)
        {
            oValue = 50;
        }
        m_oConvexitySum += oValue;
        oValue += 1;
#endif
        m_oConvexityCounter++;
    }
    if(rValue.isValid() && rValue.resultType() == S6K_PoreWidth && rValue.rank().front() == ValueRankType::eRankMax)
    {
        if (m_oFirstPoreWidth == -1)
        {
#if !S6K_RESULT_TEST
            // this is the normal procedure if value is valid and rank is eRankMax
            m_oFirstPoreWidth = rValue.value().front() * 1000;
            oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent S6K inspection result(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
#else
            // this is the procedure if S6K_RESULT_TEST is active
            m_oFirstPoreWidth = 147;
#endif
        }
    }

    if(rValue.resultType() == S6K_CrossSection_Result1)
    {
        if(rValue.isValid() && rValue.rank().front() == ValueRankType::eRankMax)
        {
#if !S6K_RESULT_TEST
            // this is the normal procedure if value is valid and rank is eRankMax
            if ( isSOUVIS6000_CrossSectionMeasurementEnable() && isSOUVIS6000_CrossSection_Leading_System() )
            {
                size_t oSize = 1;
                if (rValue.value().size() > 160)
                {
                    if (rValue.context().imageNumber() == 0) // send error message only with first image in seam
                    {
                        wmLogTr(eError, "QnxMsg.VI.CSTooManyMeas", "There are too many measurements in one result, only 160 possible !\n");
                    }
                    oSize = 160;
                }
                else
                {
                    oSize = rValue.value().size();
                }

                for(size_t i = 0;i < oSize;i++)
                {
                    int oIndex = m_oS6K_CS_CurrentNoResult[CS_IDX_RESULT1] - m_oS6K_CS_FirstImageInBlock[m_oS6K_CS_ActualResultBlock];
                    if(oIndex < m_oS6K_CS_MaxResultCountInBlock)
                    {
                        m_oS6K_CS_ResultBlock[m_oS6K_CS_ActualResultBlock][oIndex][CS_IDX_RESULT1] = static_cast<int16_t>(rValue.value().data()[i] * 1000);
                        m_oS6K_CS_ResultCountPerImage[m_oS6K_CS_ActualResultBlock][oIndex]++;
                    }
                    else
                    {
                        int oTempActualBlock = ((m_oS6K_CS_ActualResultBlock + 1) % CS_MAX_RESULT_BLOCKS);
                        int oTempIndex = oIndex - m_oS6K_CS_MaxResultCountInBlock;
                        m_oS6K_CS_ResultBlock[oTempActualBlock][oTempIndex][CS_IDX_RESULT1] = static_cast<int16_t>(rValue.value().data()[i] * 1000);
                        m_oS6K_CS_ResultCountPerImage[oTempActualBlock][oTempIndex]++;
                    }
                    m_oS6K_CS_CurrentNoResult[CS_IDX_RESULT1]++;
                }
                m_oS6K_CS_MeasuresPerResult = oSize;
            }
            if ( isSOUVIS6000_CrossSectionMeasurementEnable() && !isSOUVIS6000_CrossSection_Leading_System() )
            {
                if (rValue.context().imageNumber() == 0)
                {
                    m_rVI_InspectionControl.setS6K_CS_MeasuresPerResult(rValue.value().size());
                }
            }
#else
            // this is the procedure if S6K_RESULT_TEST is active
            static int oValue = 40 + ((m_oSeamNoIndex + 1) * 100);
            if (rValue.context().imageNumber() == 0)
            {
                oValue = 40 + ((m_oSeamNoIndex + 1) * 100);
            }

            // neu erstellen
            precitec::geo2d::TArray<double> oValues;
            oValues.resize(1);
            for(int i = 0;i < 1;i++)
            {
                oValues.getData().data()[i] = oValue + i;
            }
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

            for(size_t i = 0;i < oNewRValue.value().size();i++)
            {
                int oIndex = m_oS6K_CS_CurrentNoResult[CS_IDX_RESULT1] - m_oS6K_CS_FirstImageInBlock[m_oS6K_CS_ActualResultBlock];
                if(oIndex < m_oS6K_CS_MaxResultCountInBlock)
                {
                    m_oS6K_CS_ResultBlock[m_oS6K_CS_ActualResultBlock][oIndex][CS_IDX_RESULT1] = static_cast<int16_t>(oNewRValue.value().data()[i]);
                    m_oS6K_CS_ResultCountPerImage[m_oS6K_CS_ActualResultBlock][oIndex]++;
                }
                else
                {
                    int oTempActualBlock = ((m_oS6K_CS_ActualResultBlock + 1) % CS_MAX_RESULT_BLOCKS);
                    int oTempIndex = oIndex - m_oS6K_CS_MaxResultCountInBlock;
                    m_oS6K_CS_ResultBlock[oTempActualBlock][oTempIndex][CS_IDX_RESULT1] = static_cast<int16_t>(oNewRValue.value().data()[i]);
                    m_oS6K_CS_ResultCountPerImage[oTempActualBlock][oTempIndex]++;
                }
                m_oS6K_CS_CurrentNoResult[CS_IDX_RESULT1]++;
            }
            oValue += oNewRValue.value().size();
            m_oS6K_CS_MeasuresPerResult = oNewRValue.value().size();
#endif
            oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent S6K inspection result(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
        } // if(rValue.isValid() && rValue.rank().front() == ValueRankType::eRankMax)
        else
        {
            // this is the procedure if value is not valid or rank is not eRankMax
            if ( isSOUVIS6000_CrossSectionMeasurementEnable() && isSOUVIS6000_CrossSection_Leading_System() )
            {
                size_t oSize = 1;
                if (rValue.value().size() > 160)
                {
                    if (rValue.context().imageNumber() == 0) // send error message only with first image in seam
                    {
                        wmLogTr(eError, "QnxMsg.VI.CSTooManyMeas", "There are too many measurements in one result, only 160 possible !\n");
                    }
                    oSize = 160;
                }
                else
                {
                    oSize = rValue.value().size();
                }

                for(size_t i = 0;i < oSize;i++)
                {
                    int oIndex = m_oS6K_CS_CurrentNoResult[CS_IDX_RESULT1] - m_oS6K_CS_FirstImageInBlock[m_oS6K_CS_ActualResultBlock];
                    if(oIndex < m_oS6K_CS_MaxResultCountInBlock)
                    {
                        m_oS6K_CS_ResultBlock[m_oS6K_CS_ActualResultBlock][oIndex][CS_IDX_RESULT1] = static_cast<int16_t>(0);
                        m_oS6K_CS_ResultCountPerImage[m_oS6K_CS_ActualResultBlock][oIndex]++;
                    }
                    else
                    {
                        int oTempActualBlock = ((m_oS6K_CS_ActualResultBlock + 1) % CS_MAX_RESULT_BLOCKS);
                        int oTempIndex = oIndex - m_oS6K_CS_MaxResultCountInBlock;
                        m_oS6K_CS_ResultBlock[oTempActualBlock][oTempIndex][CS_IDX_RESULT1] = static_cast<int16_t>(0);
                        m_oS6K_CS_ResultCountPerImage[oTempActualBlock][oTempIndex]++;
                    }
                    m_oS6K_CS_CurrentNoResult[CS_IDX_RESULT1]++;
                }
                m_oS6K_CS_MeasuresPerResult = oSize;
            }
            oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent S6K inspection result(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
        } // else of:  // if (rValue.isValid() && rValue.rank().front() == ValueRankType::eRankMax)
    } // if(rValue.resultType() == S6K_CrossSection_Result1)

    if(rValue.resultType() == S6K_CrossSection_Result2)
    {
        if(rValue.isValid() && rValue.rank().front() == ValueRankType::eRankMax)
        {
#if !S6K_RESULT_TEST
            // this is the normal procedure if value is valid and rank is eRankMax
            if ( isSOUVIS6000_CrossSectionMeasurementEnable() && isSOUVIS6000_CrossSection_Leading_System() )
            {
                size_t oSize = 1;
                if (rValue.value().size() > 160)
                {
                    if (rValue.context().imageNumber() == 0) // send error message only with first image in seam
                    {
                        wmLogTr(eError, "QnxMsg.VI.CSTooManyMeas", "There are too many measurements in one result, only 160 possible !\n");
                    }
                    oSize = 160;
                }
                else
                {
                    oSize = rValue.value().size();
                }

                for(size_t i = 0;i < oSize;i++)
                {
                    int oIndex = m_oS6K_CS_CurrentNoResult[CS_IDX_RESULT2] - m_oS6K_CS_FirstImageInBlock[m_oS6K_CS_ActualResultBlock];
                    if(oIndex < m_oS6K_CS_MaxResultCountInBlock)
                    {
                        m_oS6K_CS_ResultBlock[m_oS6K_CS_ActualResultBlock][oIndex][CS_IDX_RESULT2] = static_cast<int16_t>(rValue.value().data()[i] * 1000);
                        m_oS6K_CS_ResultCountPerImage[m_oS6K_CS_ActualResultBlock][oIndex]++;
                    }
                    else
                    {
                        int oTempActualBlock = ((m_oS6K_CS_ActualResultBlock + 1) % CS_MAX_RESULT_BLOCKS);
                        int oTempIndex = oIndex - m_oS6K_CS_MaxResultCountInBlock;
                        m_oS6K_CS_ResultBlock[oTempActualBlock][oTempIndex][CS_IDX_RESULT2] = static_cast<int16_t>(rValue.value().data()[i] * 1000);
                        m_oS6K_CS_ResultCountPerImage[oTempActualBlock][oTempIndex]++;
                    }
                    m_oS6K_CS_CurrentNoResult[CS_IDX_RESULT2]++;
                }
                m_oS6K_CS_MeasuresPerResult = oSize;
            }
#else
            // this is the procedure if S6K_RESULT_TEST is active
            static int oValue = 40 + ((m_oSeamNoIndex + 1) * 100);
            if (rValue.context().imageNumber() == 0)
            {
                oValue = 40 + ((m_oSeamNoIndex + 1) * 100);
            }

            // neu erstellen
            precitec::geo2d::TArray<double> oValues;
            oValues.resize(1);
            for(int i = 0;i < 1;i++)
            {
                oValues.getData().data()[i] = oValue + i;
            }
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

            for(size_t i = 0;i < oNewRValue.value().size();i++)
            {
                int oIndex = m_oS6K_CS_CurrentNoResult[CS_IDX_RESULT2] - m_oS6K_CS_FirstImageInBlock[m_oS6K_CS_ActualResultBlock];
                if(oIndex < m_oS6K_CS_MaxResultCountInBlock)
                {
                    m_oS6K_CS_ResultBlock[m_oS6K_CS_ActualResultBlock][oIndex][CS_IDX_RESULT2] = static_cast<int16_t>(oNewRValue.value().data()[i]);
                    m_oS6K_CS_ResultCountPerImage[m_oS6K_CS_ActualResultBlock][oIndex]++;
                }
                else
                {
                    int oTempActualBlock = ((m_oS6K_CS_ActualResultBlock + 1) % CS_MAX_RESULT_BLOCKS);
                    int oTempIndex = oIndex - m_oS6K_CS_MaxResultCountInBlock;
                    m_oS6K_CS_ResultBlock[oTempActualBlock][oTempIndex][CS_IDX_RESULT2] = static_cast<int16_t>(oNewRValue.value().data()[i]);
                    m_oS6K_CS_ResultCountPerImage[oTempActualBlock][oTempIndex]++;
                }
                m_oS6K_CS_CurrentNoResult[CS_IDX_RESULT2]++;
            }
            oValue += oNewRValue.value().size();
            m_oS6K_CS_MeasuresPerResult = oNewRValue.value().size();
#endif
            oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent S6K inspection result(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
        } // if(rValue.isValid() && rValue.rank().front() == ValueRankType::eRankMax)
        else
        {
            // this is the procedure if value is not valid or rank is not eRankMax
            if ( isSOUVIS6000_CrossSectionMeasurementEnable() && isSOUVIS6000_CrossSection_Leading_System() )
            {
                size_t oSize = 1;
                if (rValue.value().size() > 160)
                {
                    if (rValue.context().imageNumber() == 0) // send error message only with first image in seam
                    {
                        wmLogTr(eError, "QnxMsg.VI.CSTooManyMeas", "There are too many measurements in one result, only 160 possible !\n");
                    }
                    oSize = 160;
                }
                else
                {
                    oSize = rValue.value().size();
                }

                for(size_t i = 0;i < oSize;i++)
                {
                    int oIndex = m_oS6K_CS_CurrentNoResult[CS_IDX_RESULT2] - m_oS6K_CS_FirstImageInBlock[m_oS6K_CS_ActualResultBlock];
                    if(oIndex < m_oS6K_CS_MaxResultCountInBlock)
                    {
                        m_oS6K_CS_ResultBlock[m_oS6K_CS_ActualResultBlock][oIndex][CS_IDX_RESULT2] = static_cast<int16_t>(0);
                        m_oS6K_CS_ResultCountPerImage[m_oS6K_CS_ActualResultBlock][oIndex]++;
                    }
                    else
                    {
                        int oTempActualBlock = ((m_oS6K_CS_ActualResultBlock + 1) % CS_MAX_RESULT_BLOCKS);
                        int oTempIndex = oIndex - m_oS6K_CS_MaxResultCountInBlock;
                        m_oS6K_CS_ResultBlock[oTempActualBlock][oTempIndex][CS_IDX_RESULT2] = static_cast<int16_t>(0);
                        m_oS6K_CS_ResultCountPerImage[oTempActualBlock][oTempIndex]++;
                    }
                    m_oS6K_CS_CurrentNoResult[CS_IDX_RESULT2]++;
                }
                m_oS6K_CS_MeasuresPerResult = oSize;
            }
            oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent S6K inspection result(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
        } // else of:  // if (rValue.isValid() && rValue.rank().front() == ValueRankType::eRankMax)
    } // if(rValue.resultType() == S6K_CrossSection_Result2)

    if(rValue.resultType() == S6K_CrossSection_Result3)
    {
        if(rValue.isValid() && rValue.rank().front() == ValueRankType::eRankMax)
        {
#if !S6K_RESULT_TEST
            // this is the normal procedure if value is valid and rank is eRankMax
            if ( isSOUVIS6000_CrossSectionMeasurementEnable() && isSOUVIS6000_CrossSection_Leading_System() )
            {
                size_t oSize = 1;
                if (rValue.value().size() > 160)
                {
                    if (rValue.context().imageNumber() == 0) // send error message only with first image in seam
                    {
                        wmLogTr(eError, "QnxMsg.VI.CSTooManyMeas", "There are too many measurements in one result, only 160 possible !\n");
                    }
                    oSize = 160;
                }
                else
                {
                    oSize = rValue.value().size();
                }

                for(size_t i = 0;i < oSize;i++)
                {
                    int oIndex = m_oS6K_CS_CurrentNoResult[CS_IDX_RESULT3] - m_oS6K_CS_FirstImageInBlock[m_oS6K_CS_ActualResultBlock];
                    if(oIndex < m_oS6K_CS_MaxResultCountInBlock)
                    {
                        m_oS6K_CS_ResultBlock[m_oS6K_CS_ActualResultBlock][oIndex][CS_IDX_RESULT3] = static_cast<int16_t>(rValue.value().data()[i] * 1000);
                        m_oS6K_CS_ResultCountPerImage[m_oS6K_CS_ActualResultBlock][oIndex]++;
                    }
                    else
                    {
                        int oTempActualBlock = ((m_oS6K_CS_ActualResultBlock + 1) % CS_MAX_RESULT_BLOCKS);
                        int oTempIndex = oIndex - m_oS6K_CS_MaxResultCountInBlock;
                        m_oS6K_CS_ResultBlock[oTempActualBlock][oTempIndex][CS_IDX_RESULT3] = static_cast<int16_t>(rValue.value().data()[i] * 1000);
                        m_oS6K_CS_ResultCountPerImage[oTempActualBlock][oTempIndex]++;
                    }
                    m_oS6K_CS_CurrentNoResult[CS_IDX_RESULT3]++;
                }
                m_oS6K_CS_MeasuresPerResult = oSize;
            }
#else
            // this is the procedure if S6K_RESULT_TEST is active
            static int oValue = 40 + ((m_oSeamNoIndex + 1) * 100);
            if (rValue.context().imageNumber() == 0)
            {
                oValue = 40 + ((m_oSeamNoIndex + 1) * 100);
            }

            // neu erstellen
            precitec::geo2d::TArray<double> oValues;
            oValues.resize(1);
            for(int i = 0;i < 1;i++)
            {
                oValues.getData().data()[i] = oValue + i;
            }
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

            for(size_t i = 0;i < oNewRValue.value().size();i++)
            {
                int oIndex = m_oS6K_CS_CurrentNoResult[CS_IDX_RESULT3] - m_oS6K_CS_FirstImageInBlock[m_oS6K_CS_ActualResultBlock];
                if(oIndex < m_oS6K_CS_MaxResultCountInBlock)
                {
                    m_oS6K_CS_ResultBlock[m_oS6K_CS_ActualResultBlock][oIndex][CS_IDX_RESULT3] = static_cast<int16_t>(oNewRValue.value().data()[i]);
                    m_oS6K_CS_ResultCountPerImage[m_oS6K_CS_ActualResultBlock][oIndex]++;
                }
                else
                {
                    int oTempActualBlock = ((m_oS6K_CS_ActualResultBlock + 1) % CS_MAX_RESULT_BLOCKS);
                    int oTempIndex = oIndex - m_oS6K_CS_MaxResultCountInBlock;
                    m_oS6K_CS_ResultBlock[oTempActualBlock][oTempIndex][CS_IDX_RESULT3] = static_cast<int16_t>(oNewRValue.value().data()[i]);
                    m_oS6K_CS_ResultCountPerImage[oTempActualBlock][oTempIndex]++;
                }
                m_oS6K_CS_CurrentNoResult[CS_IDX_RESULT3]++;
            }
            oValue += oNewRValue.value().size();
            m_oS6K_CS_MeasuresPerResult = oNewRValue.value().size();
#endif
            oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent S6K inspection result(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
        } // if(rValue.isValid() && rValue.rank().front() == ValueRankType::eRankMax)
        else
        {
            // this is the procedure if value is not valid or rank is not eRankMax
            if ( isSOUVIS6000_CrossSectionMeasurementEnable() && isSOUVIS6000_CrossSection_Leading_System() )
            {
                size_t oSize = 1;
                if (rValue.value().size() > 160)
                {
                    if (rValue.context().imageNumber() == 0) // send error message only with first image in seam
                    {
                        wmLogTr(eError, "QnxMsg.VI.CSTooManyMeas", "There are too many measurements in one result, only 160 possible !\n");
                    }
                    oSize = 160;
                }
                else
                {
                    oSize = rValue.value().size();
                }

                for(size_t i = 0;i < oSize;i++)
                {
                    int oIndex = m_oS6K_CS_CurrentNoResult[CS_IDX_RESULT3] - m_oS6K_CS_FirstImageInBlock[m_oS6K_CS_ActualResultBlock];
                    if(oIndex < m_oS6K_CS_MaxResultCountInBlock)
                    {
                        m_oS6K_CS_ResultBlock[m_oS6K_CS_ActualResultBlock][oIndex][CS_IDX_RESULT3] = static_cast<int16_t>(0);
                        m_oS6K_CS_ResultCountPerImage[m_oS6K_CS_ActualResultBlock][oIndex]++;
                    }
                    else
                    {
                        int oTempActualBlock = ((m_oS6K_CS_ActualResultBlock + 1) % CS_MAX_RESULT_BLOCKS);
                        int oTempIndex = oIndex - m_oS6K_CS_MaxResultCountInBlock;
                        m_oS6K_CS_ResultBlock[oTempActualBlock][oTempIndex][CS_IDX_RESULT3] = static_cast<int16_t>(0);
                        m_oS6K_CS_ResultCountPerImage[oTempActualBlock][oTempIndex]++;
                    }
                    m_oS6K_CS_CurrentNoResult[CS_IDX_RESULT3]++;
                }
                m_oS6K_CS_MeasuresPerResult = oSize;
            }
            oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent S6K inspection result(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
        } // else of:  // if (rValue.isValid() && rValue.rank().front() == ValueRankType::eRankMax)
    } // if(rValue.resultType() == S6K_CrossSection_Result3)

    if(rValue.resultType() == S6K_CrossSection_Result4)
    {
        if(rValue.isValid() && rValue.rank().front() == ValueRankType::eRankMax)
        {
#if !S6K_RESULT_TEST
            // this is the normal procedure if value is valid and rank is eRankMax
            if ( isSOUVIS6000_CrossSectionMeasurementEnable() && isSOUVIS6000_CrossSection_Leading_System() )
            {
                size_t oSize = 1;
                if (rValue.value().size() > 160)
                {
                    if (rValue.context().imageNumber() == 0) // send error message only with first image in seam
                    {
                        wmLogTr(eError, "QnxMsg.VI.CSTooManyMeas", "There are too many measurements in one result, only 160 possible !\n");
                    }
                    oSize = 160;
                }
                else
                {
                    oSize = rValue.value().size();
                }

                for(size_t i = 0;i < oSize;i++)
                {
                    int oIndex = m_oS6K_CS_CurrentNoResult[CS_IDX_RESULT4] - m_oS6K_CS_FirstImageInBlock[m_oS6K_CS_ActualResultBlock];
                    if(oIndex < m_oS6K_CS_MaxResultCountInBlock)
                    {
                        m_oS6K_CS_ResultBlock[m_oS6K_CS_ActualResultBlock][oIndex][CS_IDX_RESULT4] = static_cast<int16_t>(rValue.value().data()[i] * 1000);
                        m_oS6K_CS_ResultCountPerImage[m_oS6K_CS_ActualResultBlock][oIndex]++;
                    }
                    else
                    {
                        int oTempActualBlock = ((m_oS6K_CS_ActualResultBlock + 1) % CS_MAX_RESULT_BLOCKS);
                        int oTempIndex = oIndex - m_oS6K_CS_MaxResultCountInBlock;
                        m_oS6K_CS_ResultBlock[oTempActualBlock][oTempIndex][CS_IDX_RESULT4] = static_cast<int16_t>(rValue.value().data()[i] * 1000);
                        m_oS6K_CS_ResultCountPerImage[oTempActualBlock][oTempIndex]++;
                    }
                    m_oS6K_CS_CurrentNoResult[CS_IDX_RESULT4]++;
                }
                m_oS6K_CS_MeasuresPerResult = oSize;
            }
#else
            // this is the procedure if S6K_RESULT_TEST is active
            static int oValue = 40 + ((m_oSeamNoIndex + 1) * 100);
            if (rValue.context().imageNumber() == 0)
            {
                oValue = 40 + ((m_oSeamNoIndex + 1) * 100);
            }

            // neu erstellen
            precitec::geo2d::TArray<double> oValues;
            oValues.resize(1);
            for(int i = 0;i < 1;i++)
            {
                oValues.getData().data()[i] = oValue + i;
            }
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

            for(size_t i = 0;i < oNewRValue.value().size();i++)
            {
                int oIndex = m_oS6K_CS_CurrentNoResult[CS_IDX_RESULT4] - m_oS6K_CS_FirstImageInBlock[m_oS6K_CS_ActualResultBlock];
                if(oIndex < m_oS6K_CS_MaxResultCountInBlock)
                {
                    m_oS6K_CS_ResultBlock[m_oS6K_CS_ActualResultBlock][oIndex][CS_IDX_RESULT4] = static_cast<int16_t>(oNewRValue.value().data()[i]);
                    m_oS6K_CS_ResultCountPerImage[m_oS6K_CS_ActualResultBlock][oIndex]++;
                }
                else
                {
                    int oTempActualBlock = ((m_oS6K_CS_ActualResultBlock + 1) % CS_MAX_RESULT_BLOCKS);
                    int oTempIndex = oIndex - m_oS6K_CS_MaxResultCountInBlock;
                    m_oS6K_CS_ResultBlock[oTempActualBlock][oTempIndex][CS_IDX_RESULT4] = static_cast<int16_t>(oNewRValue.value().data()[i]);
                    m_oS6K_CS_ResultCountPerImage[oTempActualBlock][oTempIndex]++;
                }
                m_oS6K_CS_CurrentNoResult[CS_IDX_RESULT4]++;
            }
            oValue += oNewRValue.value().size();
            m_oS6K_CS_MeasuresPerResult = oNewRValue.value().size();
#endif
            oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent S6K inspection result(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
        } // if(rValue.isValid() && rValue.rank().front() == ValueRankType::eRankMax)
        else
        {
            // this is the procedure if value is not valid or rank is not eRankMax
            if ( isSOUVIS6000_CrossSectionMeasurementEnable() && isSOUVIS6000_CrossSection_Leading_System() )
            {
                size_t oSize = 1;
                if (rValue.value().size() > 160)
                {
                    if (rValue.context().imageNumber() == 0) // send error message only with first image in seam
                    {
                        wmLogTr(eError, "QnxMsg.VI.CSTooManyMeas", "There are too many measurements in one result, only 160 possible !\n");
                    }
                    oSize = 160;
                }
                else
                {
                    oSize = rValue.value().size();
                }

                for(size_t i = 0;i < oSize;i++)
                {
                    int oIndex = m_oS6K_CS_CurrentNoResult[CS_IDX_RESULT4] - m_oS6K_CS_FirstImageInBlock[m_oS6K_CS_ActualResultBlock];
                    if(oIndex < m_oS6K_CS_MaxResultCountInBlock)
                    {
                        m_oS6K_CS_ResultBlock[m_oS6K_CS_ActualResultBlock][oIndex][CS_IDX_RESULT4] = static_cast<int16_t>(0);
                        m_oS6K_CS_ResultCountPerImage[m_oS6K_CS_ActualResultBlock][oIndex]++;
                    }
                    else
                    {
                        int oTempActualBlock = ((m_oS6K_CS_ActualResultBlock + 1) % CS_MAX_RESULT_BLOCKS);
                        int oTempIndex = oIndex - m_oS6K_CS_MaxResultCountInBlock;
                        m_oS6K_CS_ResultBlock[oTempActualBlock][oTempIndex][CS_IDX_RESULT4] = static_cast<int16_t>(0);
                        m_oS6K_CS_ResultCountPerImage[oTempActualBlock][oTempIndex]++;
                    }
                    m_oS6K_CS_CurrentNoResult[CS_IDX_RESULT4]++;
                }
                m_oS6K_CS_MeasuresPerResult = oSize;
            }
            oTimer.elapsed(); if ( oTimer.ms() > 0 ) { wmLog(eDebug, "Sent S6K inspection result(%d) - %f - %d ms\n", rValue.resultType(), rValue.value().front(), oTimer.ms() ); }
        } // else of:  // if (rValue.isValid() && rValue.rank().front() == ValueRankType::eRankMax)
    } // if(rValue.resultType() == S6K_CrossSection_Result4)

    if ( isSOUVIS6000_CrossSectionMeasurementEnable() && isSOUVIS6000_CrossSection_Leading_System() )
    {
        if (rValue.resultType() == S6K_CrossSection_Result1)
        {
            S6K_CS_CheckForTimeDuration(m_oS6K_CS_CurrentNoResult[CS_IDX_RESULT1]);
        }
        S6K_CS_CheckForBufferFull();
    }

    oTimer.elapsed(); if ( oTimer.ms() > 5 ) { wmLog(eWarning, "Warning sending of result %d - blocked for %d ms\n", rValue.resultType(), oTimer.ms() ); }
}

void ResultsServer::result(interface::ResultRangeArray const& rValue)
{
	m_oResultCounter++;
	if (m_oResultCounter >= NUMBER_RESULTS_FOR_OK)
	{
		if (m_oOK == false) m_rVI_InspectionControl.ResultsReceivedFlag(true);
		m_oOK = true;
	}
}

void ResultsServer::result(interface::ResultRange1dArray const& rValue)
{
	m_oResultCounter++;
	if (m_oResultCounter >= NUMBER_RESULTS_FOR_OK)
	{
		if (m_oOK == false) m_rVI_InspectionControl.ResultsReceivedFlag(true);
		m_oOK = true;
	}
}

void ResultsServer::result(interface::ResultPointArray const& rValue)
{
	m_oResultCounter++;
	if (m_oResultCounter >= NUMBER_RESULTS_FOR_OK)
	{
		if (m_oOK == false) m_rVI_InspectionControl.ResultsReceivedFlag(true);
		m_oOK = true;
	}
}

void ResultsServer::nio(interface::NIOResult const& rError)
{
    m_rVI_InspectionControl.NIOReceived(rError.taskContext().measureTask()->seamseries(),
                                        rError.taskContext().measureTask()->seam(),
                                        rError.taskContext().measureTask()->seaminterval());

    system::Timer oTimer; oTimer.start();

    wmLog(eDebug, "ResultsServer::handleNio() resultType <%d> nioType <%d> imageNumber <%d>\n", rError.resultType(), rError.nioType(), rError.context().imageNumber() );
    switch ( rError.nioType() )
    {
        case XCoordOutOfLimits:
            m_rVI_InspectionControl.setSumErrorLatched(true); // Setzen des Fehlerausgangs
            break;
        case YCoordOutOfLimits:
            m_rVI_InspectionControl.setSumErrorLatched(true); // Setzen des Fehlerausgangs
            break;
        case ZCoordOutOfLimits:
            m_rVI_InspectionControl.setSumErrorLatched(true); // Setzen des Fehlerausgangs
            break;
        case ValueOutOfLimits:
            m_rVI_InspectionControl.setSumErrorLatched(true); // Setzen des Fehlerausgangs
            break;
        case RankViolation:
            m_rVI_InspectionControl.setSumErrorLatched(true); // Setzen des Fehlerausgangs
            break;
        case GapPositionError:
            m_rVI_InspectionControl.setSumErrorLatched(true); // Setzen des Fehlerausgangs
            break;
        case LaserPowerOutOfLimits:
            m_rVI_InspectionControl.setSumErrorLatched(true); // Setzen des Fehlerausgangs
            break;
        case SensorOutOfLimits:
            m_rVI_InspectionControl.setSumErrorLatched(true); // Setzen des Fehlerausgangs
            break;

        case QualityFaultTypeA:
            m_rVI_InspectionControl.setQualityErrorField(BIT0); // Bit fuer Fehlerursache setzen
            m_rVI_InspectionControl.setSumErrorLatched(true); // Setzen des Fehlerausgangs
            m_oQualityErrorAccu |= BIT0;
            m_oQualityErrorCat1Accu |= BIT0;
            break;
        case QualityFaultTypeB:
            m_rVI_InspectionControl.setQualityErrorField(BIT1); // Bit fuer Fehlerursache setzen
            m_rVI_InspectionControl.setSumErrorLatched(true); // Setzen des Fehlerausgangs
            m_oQualityErrorAccu |= BIT1;
            m_oQualityErrorCat1Accu |= BIT1;
            break;
        case QualityFaultTypeC:
            m_rVI_InspectionControl.setQualityErrorField(BIT2); // Bit fuer Fehlerursache setzen
            m_rVI_InspectionControl.setSumErrorLatched(true); // Setzen des Fehlerausgangs
            m_oQualityErrorAccu |= BIT2;
            m_oQualityErrorCat1Accu |= BIT2;
            break;
        case QualityFaultTypeD:
            m_rVI_InspectionControl.setQualityErrorField(BIT3); // Bit fuer Fehlerursache setzen
            m_rVI_InspectionControl.setSumErrorLatched(true); // Setzen des Fehlerausgangs
            m_oQualityErrorAccu |= BIT3;
            m_oQualityErrorCat1Accu |= BIT3;
            break;
        case QualityFaultTypeE:
            m_rVI_InspectionControl.setQualityErrorField(BIT4); // Bit fuer Fehlerursache setzen
            m_rVI_InspectionControl.setSumErrorLatched(true); // Setzen des Fehlerausgangs
            m_oQualityErrorAccu |= BIT4;
            m_oQualityErrorCat1Accu |= BIT4;
            break;
        case QualityFaultTypeF:
            m_rVI_InspectionControl.setQualityErrorField(BIT5); // Bit fuer Fehlerursache setzen
            m_rVI_InspectionControl.setSumErrorLatched(true); // Setzen des Fehlerausgangs
            m_oQualityErrorAccu |= BIT5;
            m_oQualityErrorCat1Accu |= BIT5;
            break;
        case QualityFaultTypeG:
            m_rVI_InspectionControl.setQualityErrorField(BIT6); // Bit fuer Fehlerursache setzen
            m_rVI_InspectionControl.setSumErrorLatched(true); // Setzen des Fehlerausgangs
            m_oQualityErrorAccu |= BIT6;
            m_oQualityErrorCat1Accu |= BIT6;
            break;
        case QualityFaultTypeH:
            m_rVI_InspectionControl.setQualityErrorField(BIT7); // Bit fuer Fehlerursache setzen
            m_rVI_InspectionControl.setSumErrorLatched(true); // Setzen des Fehlerausgangs
            m_oQualityErrorAccu |= BIT7;
            m_oQualityErrorCat1Accu |= BIT7;
            break;
        case QualityFaultTypeI:
            m_rVI_InspectionControl.setQualityErrorField(BIT8); // Bit fuer Fehlerursache setzen
            m_rVI_InspectionControl.setSumErrorLatched(true); // Setzen des Fehlerausgangs
            m_oQualityErrorAccu |= BIT8;
            m_oQualityErrorCat1Accu |= BIT8;
            break;
        case QualityFaultTypeJ:
            m_rVI_InspectionControl.setQualityErrorField(BIT9); // Bit fuer Fehlerursache setzen
            m_rVI_InspectionControl.setSumErrorLatched(true); // Setzen des Fehlerausgangs
            m_oQualityErrorAccu |= BIT9;
            m_oQualityErrorCat1Accu |= BIT9;
            break;
        case QualityFaultTypeK:
            m_rVI_InspectionControl.setQualityErrorField(BIT10); // Bit fuer Fehlerursache setzen
            m_rVI_InspectionControl.setSumErrorLatched(true); // Setzen des Fehlerausgangs
            m_oQualityErrorAccu |= BIT10;
            m_oQualityErrorCat1Accu |= BIT10;
            break;
        case QualityFaultTypeL:
            m_rVI_InspectionControl.setQualityErrorField(BIT11); // Bit fuer Fehlerursache setzen
            m_rVI_InspectionControl.setSumErrorLatched(true); // Setzen des Fehlerausgangs
            m_oQualityErrorAccu |= BIT11;
            m_oQualityErrorCat1Accu |= BIT11;
            break;
        case QualityFaultTypeM:
            m_rVI_InspectionControl.setQualityErrorField(BIT12); // Bit fuer Fehlerursache setzen
            m_rVI_InspectionControl.setSumErrorLatched(true); // Setzen des Fehlerausgangs
            m_oQualityErrorAccu |= BIT12;
            m_oQualityErrorCat1Accu |= BIT12;
            break;
        case QualityFaultTypeN:
            m_rVI_InspectionControl.setQualityErrorField(BIT13); // Bit fuer Fehlerursache setzen
            m_rVI_InspectionControl.setSumErrorLatched(true); // Setzen des Fehlerausgangs
            m_oQualityErrorAccu |= BIT13;
            m_oQualityErrorCat1Accu |= BIT13;
            break;
        case QualityFaultTypeO:
            m_rVI_InspectionControl.setQualityErrorField(BIT14); // Bit fuer Fehlerursache setzen
            m_rVI_InspectionControl.setSumErrorLatched(true); // Setzen des Fehlerausgangs
            m_oQualityErrorAccu |= BIT14;
            m_oQualityErrorCat1Accu |= BIT14;
            break;
        case QualityFaultTypeP:
            m_rVI_InspectionControl.setQualityErrorField(BIT15); // Bit fuer Fehlerursache setzen
            m_rVI_InspectionControl.setSumErrorLatched(true); // Setzen des Fehlerausgangs
            m_oQualityErrorAccu |= BIT15;
            m_oQualityErrorCat1Accu |= BIT15;
            break;
        case QualityFaultTypeQ:
            m_oQualityErrorAccu |= BIT24;
            m_oQualityErrorCat1Accu |= BIT24;
            break;
        case QualityFaultTypeR:
            m_oQualityErrorAccu |= BIT25;
            m_oQualityErrorCat1Accu |= BIT25;
            break;
        case QualityFaultTypeS:
            m_oQualityErrorAccu |= BIT26;
            m_oQualityErrorCat1Accu |= BIT26;
            break;
        case QualityFaultTypeT:
            m_oQualityErrorAccu |= BIT27;
            m_oQualityErrorCat1Accu |= BIT27;
            break;
        case QualityFaultTypeU:
            m_oQualityErrorAccu |= BIT28;
            m_oQualityErrorCat1Accu |= BIT28;
            break;
        case QualityFaultTypeV:
            m_oQualityErrorAccu |= BIT29;
            m_oQualityErrorCat1Accu |= BIT29;
            break;
        case QualityFaultTypeW:
            m_oQualityErrorAccu |= BIT30;
            m_oQualityErrorCat1Accu |= BIT30;
            break;
        case QualityFaultTypeX:
            m_oQualityErrorAccu |= BIT31;
            m_oQualityErrorCat1Accu |= BIT31;
            break;

        case QualityFaultTypeA_Cat2:
            m_oQualityErrorAccu |= BIT0;
            m_oQualityErrorCat2Accu |= BIT0;
            break;
        case QualityFaultTypeB_Cat2:
            m_oQualityErrorAccu |= BIT1;
            m_oQualityErrorCat2Accu |= BIT1;
            break;
        case QualityFaultTypeC_Cat2:
            m_oQualityErrorAccu |= BIT2;
            m_oQualityErrorCat2Accu |= BIT2;
            break;
        case QualityFaultTypeD_Cat2:
            m_oQualityErrorAccu |= BIT3;
            m_oQualityErrorCat2Accu |= BIT3;
            break;
        case QualityFaultTypeE_Cat2:
            m_oQualityErrorAccu |= BIT4;
            m_oQualityErrorCat2Accu |= BIT4;
            break;
        case QualityFaultTypeF_Cat2:
            m_oQualityErrorAccu |= BIT5;
            m_oQualityErrorCat2Accu |= BIT5;
            break;
        case QualityFaultTypeG_Cat2:
            m_oQualityErrorAccu |= BIT6;
            m_oQualityErrorCat2Accu |= BIT6;
            break;
        case QualityFaultTypeH_Cat2:
            m_oQualityErrorAccu |= BIT7;
            m_oQualityErrorCat2Accu |= BIT7;
            break;
        case QualityFaultTypeI_Cat2:
            m_oQualityErrorAccu |= BIT8;
            m_oQualityErrorCat2Accu |= BIT8;
            break;
        case QualityFaultTypeJ_Cat2:
            m_oQualityErrorAccu |= BIT9;
            m_oQualityErrorCat2Accu |= BIT9;
            break;
        case QualityFaultTypeK_Cat2:
            m_oQualityErrorAccu |= BIT10;
            m_oQualityErrorCat2Accu |= BIT10;
            break;
        case QualityFaultTypeL_Cat2:
            m_oQualityErrorAccu |= BIT11;
            m_oQualityErrorCat2Accu |= BIT11;
            break;
        case QualityFaultTypeM_Cat2:
            m_oQualityErrorAccu |= BIT12;
            m_oQualityErrorCat2Accu |= BIT12;
            break;
        case QualityFaultTypeN_Cat2:
            m_oQualityErrorAccu |= BIT13;
            m_oQualityErrorCat2Accu |= BIT13;
            break;
        case QualityFaultTypeO_Cat2:
            m_oQualityErrorAccu |= BIT14;
            m_oQualityErrorCat2Accu |= BIT14;
            break;
        case QualityFaultTypeP_Cat2:
            m_oQualityErrorAccu |= BIT15;
            m_oQualityErrorCat2Accu |= BIT15;
            break;
        case QualityFaultTypeQ_Cat2:
            m_oQualityErrorAccu |= BIT24;
            m_oQualityErrorCat2Accu |= BIT24;
            break;
        case QualityFaultTypeR_Cat2:
            m_oQualityErrorAccu |= BIT25;
            m_oQualityErrorCat2Accu |= BIT25;
            break;
        case QualityFaultTypeS_Cat2:
            m_oQualityErrorAccu |= BIT26;
            m_oQualityErrorCat2Accu |= BIT26;
            break;
        case QualityFaultTypeT_Cat2:
            m_oQualityErrorAccu |= BIT27;
            m_oQualityErrorCat2Accu |= BIT27;
            break;
        case QualityFaultTypeU_Cat2:
            m_oQualityErrorAccu |= BIT28;
            m_oQualityErrorCat2Accu |= BIT28;
            break;
        case QualityFaultTypeV_Cat2:
            m_oQualityErrorAccu |= BIT29;
            m_oQualityErrorCat2Accu |= BIT29;
            break;
        case QualityFaultTypeW_Cat2:
            m_oQualityErrorAccu |= BIT30;
            m_oQualityErrorCat2Accu |= BIT30;
            break;
        case QualityFaultTypeX_Cat2:
            m_oQualityErrorAccu |= BIT31;
            m_oQualityErrorCat2Accu |= BIT31;
            break;
        case FastStop_DoubleBlank:
            m_rVI_InspectionControl.setFastStopDoubleBlank();
            break;
        default:
            m_rVI_InspectionControl.setSumErrorLatched(true); // Setzen des Fehlerausgangs
            break;
    }

    oTimer.elapsed(); if ( oTimer.ms() > 5 ) { wmLog(eWarning, "Warning sending of nio blocked for %d ms\n", oTimer.ms() ); }
    if (m_oFirstErrorPosition == -1)
    {
        m_oFirstErrorPosition = rError.context().position() / 1000; // first error position in millimeter
    }
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
	m_oResultCounter = 0;
	m_oOK = false;

    printf("ResultsServer::inspectionStarted\n");
    m_oSeamNoIndex = p_oSeamNo;
    m_oQualityErrorAccu = 0x0;
    m_oQualityErrorCat1Accu = 0x0;
    m_oQualityErrorCat2Accu = 0x0;
    m_oFirstErrorPosition = -1;
    m_oElementWidthSum = 0;
    m_oElementWidthCounter = 0;
    m_oHeightDiffSum = 0;
    m_oHeightDiffCounter = 0;
    m_oConcavitySum = 0;
    m_oConcavityCounter = 0;
    m_oConvexitySum = 0;
    m_oConvexityCounter = 0;
    m_oFirstPoreWidth = -1;

    m_oS6K_CS_ActualResultBlock = 0;
    m_oS6K_CS_MaxResultCountInBlock = CS_MEASURE_COUNT_PER_TCP_BLOCK;
    for(int i = 0;i < CS_MAX_RESULT_BLOCKS;i++)
    {
        m_oS6K_CS_FirstImageInBlock[i] = 0;

        for(int j = 0;j < CS_MEASURE_COUNT_PER_TCP_BLOCK;j++)
        {
            for(int k = 0;k < CS_VALUES_PER_MEASURE;k++)
            {
                m_oS6K_CS_ResultBlock[i][j][k] = 0;
            }

            m_oS6K_CS_ResultCountPerImage[i][j] = 0;
        }
    }
    for(int i = 0;i < CS_VALUES_PER_MEASURE;i++)
    {
        m_oS6K_CS_CurrentNoResult[i] = 0;
    }
    m_oS6K_CS_BlockNo = 0;
    if ( isSOUVIS6000_CrossSectionMeasurementEnable() && isSOUVIS6000_CrossSection_Leading_System() )
    {
        m_rVI_InspectionControl.setS6K_CS_DataBlock(eCS_TCP_OPEN_CONNECTION, m_oSeamNoIndex + 1, 1, 1, 1, 1, 1, m_oS6K_CS_ResultBlock[m_oS6K_CS_ActualResultBlock]);
    }
    m_oS6K_CS_SeamStartFlag = true;
}

void ResultsServer::inspectionEnded(MeasureTaskIDs measureTaskIDs)
{
	m_rVI_InspectionControl.ResultsReceivedNumber(m_oResultCounter);


printf("ResultsServer::inspectionEnded\n");
printf("m_oQualityErrorAccu: %08X\n", m_oQualityErrorAccu);
printf("m_oQualityErrorCat1Accu: %08X\n", m_oQualityErrorCat1Accu);
printf("m_oQualityErrorCat2Accu: %08X\n", m_oQualityErrorCat2Accu);
printf("m_oFirstErrorPosition: %d\n", m_oFirstErrorPosition);
    uint32_t oElementWidthAvg = 0;
    if (m_oElementWidthCounter > 0)
    {
        oElementWidthAvg = static_cast<uint32_t>(m_oElementWidthSum / m_oElementWidthCounter);
    }
    uint32_t oHeightDiffAvg = 0;
    if (m_oHeightDiffCounter > 0)
    {
        oHeightDiffAvg = static_cast<uint32_t>(m_oHeightDiffSum / m_oHeightDiffCounter);
    }
    uint32_t oConcavityAvg = 0;
    if (m_oConcavityCounter > 0)
    {
        oConcavityAvg = static_cast<uint32_t>(m_oConcavitySum / m_oConcavityCounter);
    }
    uint32_t oConvexityAvg = 0;
    if (m_oConvexityCounter > 0)
    {
        oConvexityAvg = static_cast<uint32_t>(m_oConvexitySum / m_oConvexityCounter);
    }

    S6K_QualityData_S1S2 oQualityData;
    oQualityData.m_oQualityError = m_oQualityErrorAccu;
    oQualityData.m_oQualityErrorCat1 = m_oQualityErrorCat1Accu;
    oQualityData.m_oQualityErrorCat2 = m_oQualityErrorCat2Accu;
    oQualityData.m_oExceptions = 0x00;
    oQualityData.m_oFirstErrorPos = m_oFirstErrorPosition;
    oQualityData.m_oAvgElementWidth = oElementWidthAvg;
    oQualityData.m_oAvgHeightDiff = oHeightDiffAvg;
    oQualityData.m_oAvgConcavity =oConcavityAvg;
    oQualityData.m_oAvgConvexity = oConvexityAvg;
    oQualityData.m_oFirstPoreWidth = m_oFirstPoreWidth;
    m_rVI_InspectionControl.setS6K_QualityResults(m_oSeamNoIndex, oQualityData);

    if ( isSOUVIS6000_CrossSectionMeasurementEnable() && isSOUVIS6000_CrossSection_Leading_System() )
    {
        S6K_CS_CheckForLastBuffer();
        usleep(5*1000); // 5ms
        m_rVI_InspectionControl.setS6K_CS_DataBlock(eCS_TCP_CLOSE_CONNECTION, m_oSeamNoIndex + 1, 1, 1, 1, 1, 1, m_oS6K_CS_ResultBlock[m_oS6K_CS_ActualResultBlock]);
    }
}

void ResultsServer::S6K_CS_CheckForTimeDuration(int p_oImageNumber)
{
    m_oS6K_CS_CurrentTime = std::chrono::steady_clock::now(); // take current time
    auto oDuration = m_oS6K_CS_CurrentTime - m_oS6K_CS_StartTime; // time duration since start time

    typedef std::chrono::duration<long double,std::ratio<1,1000>> DurationTypeMilliSec;
    DurationTypeMilliSec oDurationMilliSec(oDuration); // time duration in unit milliseconds
//std::cout << std::round(oDurationMilliSec.count()) << " ms" << " imgNo: " << p_oImageNumber << " MaxRes: " << m_oS6K_CS_MaxResultCountInBlock << " First: " << m_oS6K_CS_FirstImageInBlock[m_oS6K_CS_ActualResultBlock] << std::endl;
    if (std::round(oDurationMilliSec.count()) >= 150) // send a TCP/IP block approx. every 150ms
    {
        m_oS6K_CS_StartTime = std::chrono::steady_clock::now(); // take a new start time
        if ((p_oImageNumber - m_oS6K_CS_FirstImageInBlock[m_oS6K_CS_ActualResultBlock] + 2) <= 165)
        {
            m_oS6K_CS_MaxResultCountInBlock = p_oImageNumber - m_oS6K_CS_FirstImageInBlock[m_oS6K_CS_ActualResultBlock] + 2; // define end of block
        }
    }
}

void ResultsServer::S6K_CS_CheckForBufferFull(void)
{
    m_S6K_CS_resultsPerImage = 1;
    for (int j = 0; j < m_oS6K_CS_MaxResultCountInBlock; j++)
    {
        if (m_oS6K_CS_ResultCountPerImage[m_oS6K_CS_ActualResultBlock][j] > m_S6K_CS_resultsPerImage)
        {
            m_S6K_CS_resultsPerImage = m_oS6K_CS_ResultCountPerImage[m_oS6K_CS_ActualResultBlock][j];
        }
    }
    bool oBufferIsFull = true;
    for(int j = 0;j < m_oS6K_CS_MaxResultCountInBlock;j++)
    {
        if (m_oS6K_CS_ResultCountPerImage[m_oS6K_CS_ActualResultBlock][j] < m_S6K_CS_resultsPerImage)
        {
            oBufferIsFull = false;
        }
    }
    if (oBufferIsFull)
    {
        int oImageCountInBuffer = 0;
        for(int j = 0;j < CS_MEASURE_COUNT_PER_TCP_BLOCK;j++)
        {
            if (m_oS6K_CS_ResultCountPerImage[m_oS6K_CS_ActualResultBlock][j] >= m_S6K_CS_resultsPerImage)
            {
                oImageCountInBuffer++;
            }
        }
        m_oS6K_CS_BlockNo++;
        m_rVI_InspectionControl.setS6K_CS_DataBlock(eCS_TCP_SEND_DATABLOCK, m_oSeamNoIndex + 1, m_oS6K_CS_BlockNo,
                                                  m_oS6K_CS_FirstImageInBlock[m_oS6K_CS_ActualResultBlock], oImageCountInBuffer,
                                                  m_oS6K_CS_MeasuresPerResult, CS_VALUES_PER_MEASURE, m_oS6K_CS_ResultBlock[m_oS6K_CS_ActualResultBlock]);
        wmLog(eDebug, "actual buffer is transmitted to TCPClientCrossSection\n");
        int oTemp = m_oS6K_CS_FirstImageInBlock[m_oS6K_CS_ActualResultBlock];
        m_oS6K_CS_FirstImageInBlock[m_oS6K_CS_ActualResultBlock] = 0;
        for(int j = 0;j < CS_MEASURE_COUNT_PER_TCP_BLOCK;j++)
        {
            for(int k = 0;k < CS_VALUES_PER_MEASURE;k++)
            {
                m_oS6K_CS_ResultBlock[m_oS6K_CS_ActualResultBlock][j][k] = 0;
            }
            m_oS6K_CS_ResultCountPerImage[m_oS6K_CS_ActualResultBlock][j] = 0;
        }
        m_oS6K_CS_ActualResultBlock = ((m_oS6K_CS_ActualResultBlock + 1) % CS_MAX_RESULT_BLOCKS);
        m_oS6K_CS_FirstImageInBlock[m_oS6K_CS_ActualResultBlock] = oTemp + oImageCountInBuffer;
        m_oS6K_CS_MaxResultCountInBlock = CS_MEASURE_COUNT_PER_TCP_BLOCK;
        m_oS6K_CS_StartTime = std::chrono::steady_clock::now(); // take a new start time
    }
}

void ResultsServer::S6K_CS_CheckForLastBuffer(void)
{
    int oImageCountInBuffer = 0;
    for(int j = 0;j < CS_MEASURE_COUNT_PER_TCP_BLOCK;j++)
    {
        if (m_oS6K_CS_ResultCountPerImage[m_oS6K_CS_ActualResultBlock][j] >= m_S6K_CS_resultsPerImage)
        {
            oImageCountInBuffer++;
        }
    }
    if (oImageCountInBuffer > 0)
    {
        m_oS6K_CS_BlockNo++;
        m_rVI_InspectionControl.setS6K_CS_DataBlock(eCS_TCP_SEND_DATABLOCK, m_oSeamNoIndex + 1, m_oS6K_CS_BlockNo,
                                                  m_oS6K_CS_FirstImageInBlock[m_oS6K_CS_ActualResultBlock], oImageCountInBuffer,
                                                  m_oS6K_CS_MeasuresPerResult, CS_VALUES_PER_MEASURE, m_oS6K_CS_ResultBlock[m_oS6K_CS_ActualResultBlock]);
        wmLog(eDebug, "last buffer is transmitted to TCPClientCrossSection\n");
    }
}

} // namespace ethercat
} // namespace precitec


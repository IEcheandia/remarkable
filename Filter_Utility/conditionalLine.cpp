/**
*  @file
*  @copyright  Precitec GmbH & Co. KG
*  @author     djb
*  @date       2021
*  @brief      This is a conditional line filter.
*              Inputs:  line_a, line_b, quality_a, quality_b
*              Outputs: line_out, compareResult
*              Compare Mode:
*                              if quality_a > quality_b then data_out == data_a and operationResult = 1
*                              if quality_a < quality_b then data_out == data_b and operationResult = -1
*                              if quality_a == quality_b then data_out == data_a and Result = 1
*/

// project includes
#include "conditionalLine.h"
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include <event/resultType.h>
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
namespace filter {

const std::string ConditionalLine::m_oFilterName("ConditionalLine");            ///< FilterName
const std::string ConditionalLine::m_oPipeOutDataName("LineOut");               ///< Pipe: Data out-pipe.
const std::string ConditionalLine::m_oPipeOutOperationResultName("OperationResult_out");   ///< Pipe: Data out-pipe.

ConditionalLine::ConditionalLine() :
    TransformFilter(ConditionalLine::m_oFilterName, Poco::UUID{"0986C7B1-D740-4A64-BE33-7207720833D1"}),
    m_pPipeInLineA              ( nullptr ),
    m_pPipeInLineB              ( nullptr ),
    m_pPipeInQualityA           ( nullptr ),
    m_pPipeInQualityB           ( nullptr ),
    m_oPipeOutOperationResult   ( this, ConditionalLine::m_oPipeOutOperationResultName),
    m_oPipeOutLine              ( this, m_oPipeOutDataName)
{
    setInPipeConnectors({{Poco::UUID("F237E707-39D8-4B78-94A6-5E487B1A3040"), m_pPipeInLineA, "LineA", 1, "line_a"},
        {Poco::UUID("E3C06580-5A2E-406D-9F27-0B6683441126"), m_pPipeInLineB, "LineB", 1, "line_b"},
        {Poco::UUID("8219AC89-6F58-462B-B337-C6D27279F09B"), m_pPipeInQualityA, "QualityA", 1, "quality_a"},
        {Poco::UUID("55ACCD00-70B9-492A-A2D1-4D1A0CE11C02"), m_pPipeInQualityB, "QualityB", 1, "quality_b"}});

    setOutPipeConnectors({{Poco::UUID("CD051893-C3C8-407D-9E01-79CCF1B4C2EF"), &m_oPipeOutLine, m_oPipeOutDataName, 0, "LineOut"},
        {Poco::UUID("2E485F5E-232C-4FE4-B037-5131DA27E13F"), &m_oPipeOutOperationResult, m_oPipeOutOperationResultName, 0, "operationResult_out"}});
    setVariantID(Poco::UUID("4E857EE3-D1C7-4F6D-AC59-A7FBDCE4FEA9"));

} // CTor

/*virtual*/ ConditionalLine::~ConditionalLine()
{

} // DTor



void ConditionalLine::setParameter()
{
    TransformFilter::setParameter();

} // setParameter.



bool ConditionalLine::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
    if ( p_rPipe.tag() == "line_a" )
    {
        m_pPipeInLineA = dynamic_cast<const line_pipe_t* >(&p_rPipe);
    }
    if ( p_rPipe.tag() == "line_b" )
    {
        m_pPipeInLineB = dynamic_cast<const line_pipe_t* >(&p_rPipe);
    }
    if ( p_rPipe.tag() == "quality_a" )
    {
        m_pPipeInQualityA = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
    }
    if ( p_rPipe.tag() == "quality_b" )
    {
        m_pPipeInQualityB = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
    }

    return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe

void ConditionalLine::proceedGroup(const void *p_pSender, fliplib::PipeGroupEventArgs & e)
{
    poco_assert_dbg(m_pPipeInLineA != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInLineB != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInQualityA != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInQualityB != nullptr); // to be asserted by graph editor

    ProceedCompare();

} // proceedGroup

void ConditionalLine::ProceedCompare()
{
    // data
    const auto &rLineInA = m_pPipeInLineA->read(m_oCounter);
    const auto &rLineInB = m_pPipeInLineB->read(m_oCounter);
    const auto &rGeoDoubleArrayInQualityA = m_pPipeInQualityA->read(m_oCounter);
    const auto &rGeoDoubleArrayInQualityB = m_pPipeInQualityB->read(m_oCounter);

    if(rGeoDoubleArrayInQualityA.ref().size() == 0 || rGeoDoubleArrayInQualityB.ref().size() == 0 || rLineInA.ref().size() == 0 || rLineInB.ref().size() == 0 )
    {
       wmLog(eInfo, "Lines or qualitiy values are empty. \n");

       preSignalAction();
       m_oPipeOutLine.signal(precitec::interface::GeoVecDoublearray {
            rLineInA.context(),
            precitec::geo2d::VecDoublearray(0),
            rLineInA.analysisResult(),
            0.0} );
       m_oPipeOutOperationResult.signal(interface::GeoDoublearray{rLineInA.context(), geo2d::Doublearray(0), interface::AnalysisOK, 0.0});

       return;
    }

    bool compatibleInput = (rLineInA.rank() != 0.0) == (rLineInB.rank() != 0.0);
    compatibleInput &= (rLineInA.analysisResult() == rLineInB.analysisResult());

    if (compatibleInput)
    {
        compatibleInput &= (rLineInA.context() == rLineInB.context());

        if (!compatibleInput)
        {
            wmLog(eInfo, "Conditional filter: context of inputs is not compatible, merge is not possible \n");
        }
    }
    else
    {
        wmLog(eInfo, "Conditional filter: validity of inputs is not compatible, merge is not possible \n");
    }


    auto sizeInA = rLineInA.ref().size();
    auto sizeInB = rLineInB.ref().size();

    if ( compatibleInput && ( sizeInB == sizeInA || sizeInB == 1 || sizeInA == 1))
    {
        auto oSizeOutArray = std::max(sizeInA, sizeInB);
        //input data does not have different oversamplig ratio, can be merged

        bool alwaysUseFirstInA = sizeInA != oSizeOutArray;
        bool alwaysUseFirstInB = sizeInB != oSizeOutArray;
        bool alwaysUseFirstQualityA = rGeoDoubleArrayInQualityA.ref().size() != oSizeOutArray;
        bool alwaysUseFirstQualityB = rGeoDoubleArrayInQualityB.ref().size() != oSizeOutArray;

        geo2d::Doublearray operationResult;

        precitec::geo2d::VecDoublearray	outLine(oSizeOutArray);
        operationResult.assign(oSizeOutArray, 0 , eRankMax);

        auto setOutLine = [&outLine](const precitec::interface::TGeo<std::vector<geo2d::Doublearray>> &inputArray, unsigned int index)
        {
            outLine[index] = inputArray.ref()[index];
        };

        for (unsigned int i = 0; i < oSizeOutArray; i++)
        {
            auto quality_a = rGeoDoubleArrayInQualityA.ref().getData()[alwaysUseFirstQualityA ? 0 : i];
            auto quality_b = rGeoDoubleArrayInQualityB.ref().getData()[alwaysUseFirstQualityB ? 0 : i];
            if (quality_a >= quality_b)
            {
                setOutLine(rLineInA, (unsigned int)(alwaysUseFirstInA ? 0 : i ));
                operationResult.getData()[i] = 1;
            }
            else
            {
                setOutLine(rLineInB, (unsigned int)(alwaysUseFirstInB ? 0 : i ));
                operationResult.getData()[i] = -1;
            }
        }

        auto pipeOut = precitec::interface::GeoVecDoublearray {
            rLineInA.context(),
            outLine,
            rLineInA.analysisResult(),
            std::min(rLineInA.rank(), rLineInB.rank())
        };

        const interface::GeoDoublearray oGeoDoubleOperationResultOut(rLineInA.context(), operationResult, interface::AnalysisOK, eRankMax);
        preSignalAction();
        m_oPipeOutLine.signal( pipeOut );
        m_oPipeOutOperationResult.signal(oGeoDoubleOperationResultOut);
        return;
    }

    const interface::GeoVecDoublearray* pOutLine = NULL;
    double oOperationResult = 0;

    const auto quality_a = std::get<eData>(rGeoDoubleArrayInQualityA.ref()[0]);
    const auto quality_b = std::get<eData>(rGeoDoubleArrayInQualityB.ref()[0]);

    // check quality data
    if(quality_a >= quality_b)
    {
        pOutLine = &rLineInA;
        oOperationResult = 1;
    }
    else
    {
        pOutLine = &rLineInB;
        oOperationResult = -1;
    }

    preSignalAction();

    // stuff data into pipline
    geo2d::Doublearray oOutOperationResult;
    oOutOperationResult.assign(1);
    double maxRank = eRankMax;
    oOutOperationResult[0] = std::tie(oOperationResult, maxRank);
    const interface::GeoDoublearray oGeoDoubleOperationResultOut(pOutLine->context(), oOutOperationResult, interface::AnalysisOK, eRankMax);

    m_oPipeOutOperationResult.signal(oGeoDoubleOperationResultOut);
    m_oPipeOutLine.signal(*pOutLine);
}

} // namespace filter
} // namespace precitec

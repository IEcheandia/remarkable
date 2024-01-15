/**
*  @file
*  @copyright  Precitec GmbH & Co. KG
*  @author     mm
*  @date       2022
*  @brief      This is a conditional contour filter.
*              Inputs:  contour_a, contour_b, quality_a, quality_b
*              Outputs: contour_out, compareResult
*              Compare Mode:
*                              if quality_a > quality_b then data_out == data_a and operationResult = 1
*                              if quality_a < quality_b then data_out == data_b and operationResult = -1
*                              if quality_a == quality_b then data_out == data_a and Result = 1
*/

// project includes
#include "conditionalContour.h"
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include <event/resultType.h>
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
namespace filter {

const std::string ConditionalContour::m_oFilterName("ConditionalContour");                   ///< FilterName
const std::string ConditionalContour::m_oPipeOutDataName("ContourOut");                      ///< Pipe: Data out-pipe.
const std::string ConditionalContour::m_oPipeOutOperationResultName("OperationResult_out");  ///< Pipe: Data out-pipe.

ConditionalContour::ConditionalContour() :
    TransformFilter(ConditionalContour::m_oFilterName, Poco::UUID{"06f13dae-07d1-47e1-8d73-69c9581a3157"}),
    m_pPipeInContourA           (nullptr),
    m_pPipeInContourB           (nullptr),
    m_pPipeInQualityA           (nullptr),
    m_pPipeInQualityB           (nullptr),
    m_oPipeOutOperationResult   (this, ConditionalContour::m_oPipeOutOperationResultName),
    m_oPipeOutContour           (this, m_oPipeOutDataName)
{
    setInPipeConnectors({{Poco::UUID("6919a437-3114-44e3-85b9-f534d7d33469"), m_pPipeInContourA, "ContourA", 1, "contour_a"},
        {Poco::UUID("0102ea4c-57b5-4eb1-99ca-39c8e2fd8dc5"), m_pPipeInContourB, "ContourB", 1, "contour_b"},
        {Poco::UUID("e0da376f-aa10-4d08-98ff-726133828bf0"), m_pPipeInQualityA, "QualityA", 1, "quality_a"},
        {Poco::UUID("aaa38b25-0586-4215-9e61-9de0ca6ba81b"), m_pPipeInQualityB, "QualityB", 1, "quality_b"}});

    setOutPipeConnectors({{Poco::UUID("8b30a094-a65e-4172-a282-c0763ea667d3"), &m_oPipeOutContour, m_oPipeOutDataName, 0, "ContourOut"},
        {Poco::UUID("77c04a39-d536-4d77-9ac2-283538ffb267"), &m_oPipeOutOperationResult, m_oPipeOutOperationResultName, 0, "operationResult_out"}});
    setVariantID(Poco::UUID("0e8e786d-baad-408f-b49b-298e82d9a9bd"));

}

ConditionalContour::~ConditionalContour() = default;


void ConditionalContour::setParameter()
{
    TransformFilter::setParameter();

}


bool ConditionalContour::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
    if ( p_rPipe.tag() == "contour_a" )
    {
        m_pPipeInContourA = dynamic_cast<const contour_pipe_t* >(&p_rPipe);
    }
    if ( p_rPipe.tag() == "contour_b" )
    {
        m_pPipeInContourB = dynamic_cast<const contour_pipe_t* >(&p_rPipe);
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

}

void ConditionalContour::proceedGroup(const void *p_pSender, fliplib::PipeGroupEventArgs & e)
{
    poco_assert_dbg(m_pPipeInContourA != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInContourB != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInQualityA != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInQualityB != nullptr); // to be asserted by graph editor

    proceedCompare();

}

void ConditionalContour::proceedCompare()
{
    // data
    const auto &rContourInA = m_pPipeInContourA->read(m_oCounter);
    const auto &rContourInB = m_pPipeInContourB->read(m_oCounter);
    const auto &rGeoDoubleArrayInQualityA = m_pPipeInQualityA->read(m_oCounter);
    const auto &rGeoDoubleArrayInQualityB = m_pPipeInQualityB->read(m_oCounter);

    if (rGeoDoubleArrayInQualityA.ref().size() == 0 || rGeoDoubleArrayInQualityB.ref().size() == 0 || rContourInA.ref().size() == 0 || rContourInB.ref().size() == 0)
    {
       wmLog(eInfo, "At least one qualitiy value or contour is empty. \n");

       preSignalAction();
       m_oPipeOutContour.signal(interface::GeoVecAnnotatedDPointarray {
            rContourInA.context(),
            std::vector<geo2d::AnnotatedDPointarray>{},
            rContourInA.analysisResult(),
            interface::NotPresent} );
       m_oPipeOutOperationResult.signal(interface::GeoDoublearray{rContourInA.context(), geo2d::Doublearray(0), interface::AnalysisOK, interface::NotPresent});

       return;
    }

    bool compatibleInput = (rContourInA.rank() != 0.0) == (rContourInB.rank() != 0.0);
    compatibleInput &= (rContourInA.analysisResult() == rContourInB.analysisResult());

    if (compatibleInput)
    {
        compatibleInput &= (rContourInA.context() == rContourInB.context());

        if (!compatibleInput)
        {
            wmLog(eInfo, "Conditional filter: context of inputs is not compatible, merge is not possible \n");
        }
    }
    else
    {
        wmLog(eInfo, "Conditional filter: validity of inputs is not compatible, merge is not possible \n");
    }


    auto sizeInA = rContourInA.ref().size();
    auto sizeInB = rContourInB.ref().size();

    // If all arrays have the same size, compare all qualities. Otherwise use only the first quality.
    if (compatibleInput && sizeInB == sizeInA && rGeoDoubleArrayInQualityA.ref().size() == sizeInA && rGeoDoubleArrayInQualityB.ref().size() == sizeInA)
    {
        geo2d::Doublearray operationResult;

        std::vector<geo2d::AnnotatedDPointarray> outContour(sizeInA);
        operationResult.assign(sizeInA, 0, eRankMax);

        auto setOutContour = [&outContour](const precitec::interface::TGeo<std::vector<geo2d::AnnotatedDPointarray>> &inputArray, unsigned int index)
        {
            outContour[index] = inputArray.ref()[index];
        };

        for (size_t i = 0; i < sizeInA; i++)
        {
            auto quality_a = rGeoDoubleArrayInQualityA.ref().getData()[i];
            auto quality_b = rGeoDoubleArrayInQualityB.ref().getData()[i];
            if (quality_a >= quality_b)
            {
                setOutContour(rContourInA, i);
                operationResult.getData()[i] = 1;
            }
            else
            {
                setOutContour(rContourInB, i);
                operationResult.getData()[i] = -1;
            }
        }

        auto pipeOut = precitec::interface::GeoVecAnnotatedDPointarray {
            rContourInA.context(),
            outContour,
            rContourInA.analysisResult(),
            std::min(rContourInA.rank(), rContourInB.rank())
        };

        const interface::GeoDoublearray oGeoDoubleOperationResultOut(rContourInA.context(), operationResult, interface::AnalysisOK, eRankMax);
        preSignalAction();
        m_oPipeOutContour.signal( pipeOut );
        m_oPipeOutOperationResult.signal(oGeoDoubleOperationResultOut);
        return;
    }

    const interface::GeoVecAnnotatedDPointarray* pOutContour = nullptr;
    double oOperationResult = 0;

    const auto quality_a = std::get<eData>(rGeoDoubleArrayInQualityA.ref()[0]);
    const auto quality_b = std::get<eData>(rGeoDoubleArrayInQualityB.ref()[0]);

    // check quality data
    if (quality_a >= quality_b)
    {
        pOutContour = &rContourInA;
        oOperationResult = 1;
    }
    else
    {
        pOutContour = &rContourInB;
        oOperationResult = -1;
    }

    preSignalAction();

    // stuff data into pipline
    geo2d::Doublearray oOutOperationResult;
    oOutOperationResult.assign(1);
    double maxRank = eRankMax;
    oOutOperationResult[0] = std::tie(oOperationResult, maxRank);
    const interface::GeoDoublearray oGeoDoubleOperationResultOut(pOutContour->context(), oOutOperationResult, interface::AnalysisOK, eRankMax);

    m_oPipeOutOperationResult.signal(oGeoDoubleOperationResultOut);
    m_oPipeOutContour.signal(*pOutContour);
}

}
}


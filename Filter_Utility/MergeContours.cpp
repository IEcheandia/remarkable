/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		LB
 * 	@date		2020
 * 	@brief		This filter generate a contour by merging 2 contours provided in input
 */

#include "MergeContours.h"
#include "filter/algoArray.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <fliplib/TypeToDataTypeImpl.h>
#include "filter/algoPoint.h"
#include <common/definesScanlab.h>

namespace precitec {
namespace filter {
using fliplib::Parameter;


MergeContours::MergeContours():
    TransformFilter("MergeContours", Poco::UUID{"23921ca8-842c-4d37-85b3-1eaf4ca42293"}),
    m_pPipeInData1(nullptr),
    m_pPipeInData2(nullptr),
    m_oPipeOutData(this,"Contour"),
    m_mergeType(MergeType::SimpleMerge)
    {
        parameters_.add("MergeType", fliplib::Parameter::TYPE_int, (int)(m_mergeType));

        setInPipeConnectors({{Poco::UUID("84324d17-12da-4d56-b7d4-1932dc9b5082"), m_pPipeInData1, "Contour1", 1, "Contour1"},
        {Poco::UUID("fa43199e-ed35-4c47-a901-79f67557fb58"), m_pPipeInData2, "Contour2", 1, "Contour2"}});
        setOutPipeConnectors({{Poco::UUID("529240c6-369b-4d49-8b86-4b8a882323b1"), &m_oPipeOutData, "Contour", 0, ""}});
        setVariantID(Poco::UUID("d3a0094c-25d7-46fb-bc88-1eb5f229baa1"));
    }

void MergeContours::setParameter()
{
    TransformFilter::setParameter();
    m_mergeType = static_cast<MergeType>(parameters_.getParameter("MergeType").convert<int>());

}

bool MergeContours::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) 
{
    if ( p_rPipe.tag() == "Contour1" )
    {
        m_pPipeInData1  = dynamic_cast<pipe_contour_t*>(&p_rPipe);
    }
    else if ( p_rPipe.tag() == "Contour2" ) 
    {
        m_pPipeInData2  = dynamic_cast<pipe_contour_t*>(&p_rPipe);
    }
    else
    {
        poco_assert_dbg(false); // to be asserted by graph editor
    }
    return BaseFilter::subscribe( p_rPipe, p_oGroup );
}

void MergeContours::proceedGroup( const void* p_pSender, fliplib::PipeGroupEventArgs& e )
{
    using interface::GeoDoublearray;
    
    poco_assert_dbg(m_pPipeInData1 != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInData2 != nullptr); // to be asserted by graph editor

    const auto & rGeoIn1 = m_pPipeInData1->read(m_oCounter);
    const auto & rGeoIn2 = m_pPipeInData2->read(m_oCounter);

    const auto & rOutputContext = rGeoIn1.context();
    m_oSpTrafo = rOutputContext.trafo();
    auto trafo2 = rGeoIn2.context().trafo();

    if (m_oVerbosity >= eHigh)
    {
        if (rGeoIn1.context()!= rGeoIn2.context())
        {
            std::ostringstream oMsg;
            oMsg << name() << ": Different contexts for inputs " << m_oSpTrafo->apply(geo2d::Point{0,0}) << " " << trafo2->apply(geo2d::Point{0,0}) <<" the first one will be used'\n";
            wmLog(eDebug, oMsg.str());
        }
    }

    m_oOutPoints.clear();
    interface::ResultType oGeoAnalysisResult = std::max(rGeoIn1.analysisResult(), rGeoIn2.analysisResult());
    if (rGeoIn1.ref().size() == 0 && rGeoIn2.ref().size() == 0)
    {
        const interface::GeoVecAnnotatedDPointarray oGeoOut( rOutputContext, std::vector<geo2d::AnnotatedDPointarray>{}, oGeoAnalysisResult, 1.0 );
        preSignalAction();
        m_oPipeOutData.signal(oGeoOut);

        return;
    }

    mergeContours(m_oOutPoints,
                    rGeoIn1.ref().size() != 0 ? rGeoIn1.ref().front() : geo2d::AnnotatedDPointarray{},
                  rGeoIn2.ref().size() != 0 ? rGeoIn2.ref().front() : geo2d::AnnotatedDPointarray{},
                  rGeoIn1.context(), rGeoIn2.context(), m_mergeType);

    const interface::GeoVecAnnotatedDPointarray oGeoOut( rOutputContext, std::vector<geo2d::AnnotatedDPointarray>{m_oOutPoints}, oGeoAnalysisResult, 1.0 );
    preSignalAction(); 
    m_oPipeOutData.signal(oGeoOut);

}


/*static*/ void MergeContours::mergeContours(geo2d::AnnotatedDPointarray & rMergedContour,
                                                                    const geo2d::AnnotatedDPointarray & contour1,
                                                     const geo2d::AnnotatedDPointarray & contour2,
                                                     const interface::ImageContext & rContextReference,
                                                     const interface::ImageContext & rContext2,
                                                     MergeType mergeType
                                            )
{
 
    auto transformPoint2 = [&](geo2d::DPoint p )
    {
        auto result = transformPoint(p, rContext2, rContextReference );
        return result;
    };


    bool hasDifferentContext = (rContext2 != rContextReference);

    if (contour1.size() == 0)
    {
        rMergedContour = contour2;
        if (hasDifferentContext)
        {
            for (auto && point : rMergedContour.getData())
            {
                point = transformPoint2(point);
            }
        }
        return;
    }
    if (contour2.size() == 0)
    {
        rMergedContour = contour1;
        return;
    }




    
    //check if the last point of contour1 and the first point of contour2 are overlapping
    auto firstPoint2 = transformPoint2(contour2.getData().front());
    auto lastPoint1 = contour1.getData().back();

    bool overlapping = (mergeType == MergeType::ScannerJumpBetweenContours) && geo2d::distance(lastPoint1, firstPoint2) < 1e-16;
    unsigned int outSize = contour1.size() + contour2.size() - (overlapping ? 1 : 0);

    //create references to output data and reserve the necessary number of elements
    auto & rOutData = rMergedContour.getData();
    auto & rOutRank = rMergedContour.getRank();
    std::vector<geo2d::AnnotatedDPointarray::scalarmap_t::iterator> scalarItVector;
    for (auto scalarType : contour1.getScalarDataTypes())
    {
        if (contour2.hasScalarData(scalarType))
        {
            auto insertion = rMergedContour.insertScalar(scalarType);
            scalarItVector.push_back(insertion.first);
        }
    }

    rOutData.reserve(outSize);
    rOutRank.reserve(outSize);
    for (auto & itScalar :  scalarItVector)
    {
        itScalar->second.reserve(outSize);
    }

    //copy first input to output
    rOutData.insert(rOutData.end(), contour1.getData().begin(), contour1.getData().end());
    rOutRank.insert(rOutRank.end(), contour1.getRank().begin(), contour1.getRank().end());

    for (auto & outScalar :  scalarItVector)
    {
        auto & scalarType = outScalar->first;
        auto & scalarDataOut = outScalar->second;
        auto & scalarDataIn1 = contour1.getScalarData(scalarType);
        scalarDataOut.insert(scalarDataOut.end(), scalarDataIn1.begin(), scalarDataIn1.end());
    }


    if (overlapping)
    {
        rOutRank.back() = std::max(contour1.getRank().back(), contour2.getRank().front());
    }

    //copy second input to output

    {
        auto itData2 = contour2.getData().begin();
        if (overlapping)
        {
            itData2++;
        }
        if (hasDifferentContext)
        {
            for (; itData2 != contour2.getData().end(); itData2++)
            {
                rOutData.emplace_back(transformPoint2(*itData2));
            }
        }
        else
        {
            rOutData.insert(rOutData.end(), itData2, contour2.getData().end());
        }
    }

    {
        auto itRank2 = contour2.getRank().begin();
        if (overlapping)
        {
            itRank2++;
        }
        rOutRank.insert(rOutRank.end(), itRank2, contour2.getRank().end());
    }

    {
        for (auto & outScalar :  scalarItVector)
        {
            auto & scalarType = outScalar->first;
            auto & scalarDataOut = outScalar->second;
            auto & scalarDataIn2 = contour2.getScalarData(scalarType);
            auto it2 = scalarDataIn2.begin();
            if (overlapping)
            {
                it2++;
            }
            scalarDataOut.insert(scalarDataOut.end(), it2, scalarDataIn2.end());
        }

    }



    //ensure a jump is performed between the 2 contours
    if (mergeType == MergeType::ScannerJumpBetweenContours )
    {
        int indexLastPointFirstContour = contour1.size() - 1;
        for (auto scalarType : { geo2d::AnnotatedDPointarray::Scalar::LaserPower, geo2d::AnnotatedDPointarray::Scalar::LaserPowerRing })
        {
            if (rMergedContour.hasScalarData(scalarType))
            {
                rMergedContour.getScalarData(scalarType)[indexLastPointFirstContour] = 0;
            }
        }
        //verify if the second figure starts with SCANMASTERWELDINGDATA_UNDEFINEDVALUE, which could mean static value or previous value
        int indexFirstPointSecondContour = indexLastPointFirstContour+1;
        for (auto scalarType : { geo2d::AnnotatedDPointarray::Scalar::LaserPower, geo2d::AnnotatedDPointarray::Scalar::LaserPowerRing })
        {
            if (rMergedContour.hasScalarData(scalarType))
            {
                auto & rFirstValue = rMergedContour.getScalarData(scalarType)[indexFirstPointSecondContour];
                if ( rFirstValue == SCANMASTERWELDINGDATA_UNDEFINEDVALUE)
                {
                    rFirstValue = SCANMASTERWELDINGDATA_USESTATICVALUE;
                }
            }
        }
    }



}

void MergeContours::paint()
{
    using namespace precitec::image;
    
    if (m_oVerbosity < VerbosityType::eMedium)
    {
        return;
    }
    
    if (m_oSpTrafo.isNull() || m_oOutPoints.size() == 0)
    {
        return;
    }
    
    OverlayCanvas	&rCanvas ( canvas<OverlayCanvas>(m_oCounter) );
    OverlayLayer	&rLayerPosition ( rCanvas.getLayerPosition()); 
    OverlayLayer	&rLayerContour ( rCanvas.getLayerContour()); 
    
    auto oColor = Color::Green();


    auto & rData = m_oOutPoints.getData();
    auto & rRank = m_oOutPoints.getRank();
    for (int i = 0, n = m_oOutPoints.size(); i < n; i++)
    {
        auto & rPoint = rData[i];
        auto oCanvasPoint = m_oSpTrafo->apply(geo2d::Point( (int)std::round(rPoint.x), (int)std::round(rPoint.y)));
            
        if (rRank[i] == eRankMax)
        {
            rLayerPosition.add<OverlayCross>(oCanvasPoint.x, oCanvasPoint.y, 3, oColor);
        }
        else
        {                
            rLayerPosition.add<OverlayCircle>(oCanvasPoint.x, oCanvasPoint.y, 3, oColor);
        }
    }
    
    if (m_oVerbosity < eHigh)
    {
        return;
    }
    auto oColorContour = oColor;
    oColorContour.alpha = 200;
    
    auto & firstPoint = rData[0];
    auto prevPoint = m_oSpTrafo->apply(geo2d::Point( (int)std::round(firstPoint.x), (int)std::round(firstPoint.y)));;

    for (int i = 1, n = m_oOutPoints.size(); i < n; i++)
    {
        auto & rPoint = rData[i];
        auto oCanvasPoint = m_oSpTrafo->apply(geo2d::Point( (int)std::round(rPoint.x), (int)std::round(rPoint.y)));
            
        rLayerContour.add<OverlayLine>(prevPoint, oCanvasPoint, oColorContour);

        prevPoint = oCanvasPoint;

    }
}

}
}

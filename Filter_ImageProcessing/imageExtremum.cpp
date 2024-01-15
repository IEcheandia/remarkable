#include "imageExtremum.h"

#include "common/frame.h"
#include "image/image.h"
#include "module/moduleLogger.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"

#include <algorithm> //min_element

#include <fliplib/TypeToDataTypeImpl.h>

//helper class to compute weighted average
//weighted average (x) = (1/sum(w)) * sum(w*x) = 1/sum(w)* ( w0 *sum(x) + sum( (w-w0) * x))

class WeightedAverageCalculator
{
private:
    double m_w0;
    double sum_w; //sum(w)
    double sum_x; //sum(x)
    double sum_deltaw_x; //sum( (w-w0) * x))
public:
    WeightedAverageCalculator(double w0 = 0)
        : m_w0(w0),
        sum_w(0),
        sum_x(0),
        sum_deltaw_x(0)
    {}
    double getWeightedAverage()
    {
        return ( sum_deltaw_x + m_w0*sum_x ) / sum_w;
    }
    void add(double x, double w)
    {
        sum_w += w;
        sum_x += x;
        sum_deltaw_x += (w - m_w0)*x;
    }

};

namespace precitec
{
using image::BImage;
using image::OverlayCanvas;
using image::OverlayLayer;
using image::OverlayCross;
using image::Color;

using interface::ImageFrame;
using interface::GeoDoublearray;

namespace filter
{

const std::string ImageExtremum::m_oFilterName = "ImageExtremum";
const std::string ImageExtremum::m_oPipeOutXName = "X";
const std::string ImageExtremum::m_oPipeOutYName = "Y";
const std::string ImageExtremum::m_oPipeOutValueName = "PixelValue";

ImageExtremum::ImageExtremum():
TransformFilter(ImageExtremum::m_oFilterName, Poco::UUID{"a64cf3ee-1337-4871-8e3b-640107a501b0"}),
m_pPipeInImageFrame(nullptr),
m_pipeOutX(this, ImageExtremum::m_oPipeOutXName),
m_pipeOutY(this, ImageExtremum::m_oPipeOutYName),
m_pipeOutValue(this, ImageExtremum::m_oPipeOutValueName),
m_extremumType(ImageExtremumType::eMaximum),
m_modus(FilterModusType::eSimple),
m_x(0),
m_y(0),
m_value(0),
m_rank(eRankMin)
{
    parameters_.add("ExtremumType", fliplib::Parameter::TYPE_int, m_extremumType); //in SQL the type is enum
    parameters_.add("Modus", fliplib::Parameter::TYPE_uint, m_modus);

    setInPipeConnectors({{Poco::UUID("ee4d8ac8-2796-4779-9d68-0b885cff9b03"), m_pPipeInImageFrame, "InputImage", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("0b1e9230-1fac-4d4f-9cce-0918e2528957"), &m_pipeOutX, "X", 0, ""},
    {Poco::UUID("2973f764-04b3-44a9-a4d2-4afeb442537f"), &m_pipeOutY, "Y", 0, ""},
    {Poco::UUID("8a8bceff-c6c3-4465-81fb-de31039d1185"), &m_pipeOutValue, "PixelValue", 0, ""}});
    setVariantID(Poco::UUID("03c44b53-8cad-495f-a550-4c26f758b8c1"));
}

void ImageExtremum::setParameter()
{
    TransformFilter::setParameter();
    m_extremumType = parameters_.getParameter("ExtremumType").convert<byte>();
    m_modus = parameters_.getParameter("Modus").convert<byte>();

}

bool ImageExtremum::subscribe(fliplib::BasePipe & p_rPipe, int p_oGroup)
{
    m_pPipeInImageFrame = dynamic_cast<image_pipe_t *> (&p_rPipe);
    return BaseFilter::subscribe(p_rPipe, p_oGroup);
}

void ImageExtremum::proceed(const void* sender, fliplib::PipeEventArgs& e)
{
    poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor
    const ImageFrame &rFrame = m_pPipeInImageFrame->read(m_oCounter);
    const BImage &rImage = rFrame.data();
    interface::ResultType	curAnalysisResult = rFrame.analysisResult();
    m_oSpTrafo = rFrame.context().trafo();

    if ( !rImage.isValid() )
    {
        m_rank = eRankMin;
        sendResult(rFrame.context(), curAnalysisResult);
        return;
    }

    const auto oSize = rImage.size();  //getSize is defined by ipSignal, it gives image area
    m_rank = eRankMax;

    //check whether the image can be read as a "block" image (TImage) or a roi (TLineImage)
    if ( rImage.end() - rImage.begin() == oSize.area() )
    {
        auto imgBegin = rImage.begin();
        auto imgEnd = rImage.end();
        auto imageIterator = (m_extremumType == ImageExtremumType::eMaximum) ?
            std::max_element(imgBegin, imgEnd) : std::min_element(imgBegin, imgEnd);
        m_value = *imageIterator;
        int index = imageIterator - imgBegin;
        m_y = index / rImage.width();
        m_x = index % rImage.width();
    }
    else
    {
        m_x = 0;
        m_y = 0;
        m_value = rImage.getValue(m_x, m_y);
        //the image it's a roi of a bigger image,  iterate line by line to ensure staying inside the roi borders
        if ( m_extremumType == ImageExtremumType::eMaximum)
        {   //find maximum
            for ( int j = 0, j_max = oSize.height; j < j_max; ++j )
            {
                auto imageIterator = std::max_element(rImage.rowBegin(j), rImage.rowEnd(j));
                if ( *imageIterator > m_value )
                {
                    m_value = *imageIterator;
                    m_y = j;
                    m_x = imageIterator - rImage.rowBegin(j);
                }
            }
        }
        else
        {   //find minimum
            for ( int j = 0, j_max = oSize.height; j < j_max; ++j )
            {
                auto imageIterator = std::min_element(rImage.rowBegin(j), rImage.rowEnd(j));
                if ( *imageIterator < m_value )
                {
                    m_value = *imageIterator;
                    m_y = j;
                    m_x = imageIterator - rImage.rowBegin(j);
                }
            }
        }

    }


#ifndef NDEBUG
        assert(rImage.getValue(m_x, m_y) == m_value);
        //iterate along x and x, check the validity of the extremum found

        for ( int j = 0, j_max = oSize.height; j < j_max; j++ )
        {
            for ( int i = 0, i_max = oSize.width; i < i_max; i++ )
            {
                if ( m_extremumType == ImageExtremumType::eMaximum )
                {
                    assert(rImage.getValue(i, j) <= m_value);
                }
                else
                {
                    assert(rImage.getValue(i, j) >= m_value);
                }
            }
        }
#endif

    if ( m_modus == FilterModusType::eInterpolation )
    {
        //make a weighted average of the coordinate, using normalized intensity as weight
        //maximum: weight is intensity/255
        //minimum: weight is 1 - intensity/255
        int sign = m_extremumType == ImageExtremum::eMaximum ? 1 : -1;
        int offset = m_extremumType == ImageExtremum::eMaximum ? 0 : 255;
        int radius = 1;
        WeightedAverageCalculator oWeightedX(sign*m_value/255.0 + offset);
        for ( int x = m_x - radius; x <= m_x + radius; ++x )
        {
            if ( x >= 0 && x < rImage.width() )
            {
                oWeightedX.add(x, offset + sign*rImage.getValue(x, m_y));
            }
        }

        WeightedAverageCalculator oWeightedY(offset + sign*m_value);
        for ( int y = m_y - radius; y <= m_y + radius; ++y )
        {
            if ( y >= 0 && y < rImage.height() )
            {
                oWeightedY.add(y, offset + sign*rImage.getValue(m_x, y));
            }

        }
#ifndef NDEBUG
        //some of the following assertions are valid only with radius = 1

        if(std::abs(m_x - oWeightedX.getWeightedAverage()) >= (0.5 + 1e-10)
            || std::abs(m_y - oWeightedY.getWeightedAverage()) >= (0.5 + 1e-10))
        {
            wmLog(eWarning, "Interpolation moved from %f,%f to %f,%f (%f %f)\n",
                m_x, m_y,
                oWeightedX.getWeightedAverage(),oWeightedY.getWeightedAverage(),
                m_x - oWeightedX.getWeightedAverage(), m_y - oWeightedY.getWeightedAverage()  );
            assert(false && "Wrong interpolation");
        }


        if ( m_x > 0 && m_x < rImage.width() - 1 && m_y > 0 && m_y < rImage.height() - 1 )
        {
            int directionToClosestPixelX = oWeightedX.getWeightedAverage() > m_x ? 1 : -1;
            int directionToClosestPixelY = oWeightedY.getWeightedAverage() > m_y ? 1 : -1;

            if ( m_extremumType == ImageExtremum::eMaximum )
            {
                assert(rImage.getValue(m_x + directionToClosestPixelX, m_y) >= rImage.getValue(m_x - directionToClosestPixelX, m_y));
                assert(rImage.getValue(m_x, m_y + directionToClosestPixelY) >= rImage.getValue(m_x, m_y - directionToClosestPixelY));
            }
            else
            {
                assert(rImage.getValue(m_x + directionToClosestPixelX, m_y) <= rImage.getValue(m_x - directionToClosestPixelX, m_y));
                assert(rImage.getValue(m_x, m_y + directionToClosestPixelY) <= rImage.getValue(m_x, m_y - directionToClosestPixelY));
            }
        }
#endif
        m_x = oWeightedX.getWeightedAverage();
        m_y = oWeightedY.getWeightedAverage();
    }
    sendResult(rFrame.context(), curAnalysisResult);
}

//send result to output pipe (preSignalAction is called here)
void ImageExtremum::sendResult(const interface::ImageContext & rContext, const interface::ResultType & rResultType)
{
    const double oGlobalRank = interface::Limit; //1.0

    const GeoDoublearray oGeoOutX(rContext, geo2d::TArray<double>(1, m_x, m_rank), rResultType, oGlobalRank);
    const GeoDoublearray oGeoOutY(rContext, geo2d::TArray<double>(1, m_y, m_rank), rResultType, oGlobalRank);
    const GeoDoublearray oGeoOutValue(rContext, geo2d::TArray<double>(1, m_value, m_rank), rResultType, oGlobalRank);

    preSignalAction();
    m_pipeOutX.signal(oGeoOutX);
    m_pipeOutY.signal(oGeoOutY);
    m_pipeOutValue.signal(oGeoOutValue);

}

void ImageExtremum::paint()
{
    if ( m_oSpTrafo.isNull() || m_oVerbosity < VerbosityType::eHigh )
    {
        return;
    }

    const interface::Trafo &rTrafo(*m_oSpTrafo);
    //I need to round first, because trafo just casts
    geo2d::Point oPoint = rTrafo(geo2d::Point(std::round(m_x), std::round(m_y)));
    OverlayCanvas	&rCanvas(canvas<OverlayCanvas>(m_oCounter));
    OverlayLayer	&rLayerPosition(rCanvas.getLayerPosition());

    rLayerPosition.add<OverlayCross>(oPoint, m_rank == eRankMax ? Color::Green() : Color::Red());

}




}
}

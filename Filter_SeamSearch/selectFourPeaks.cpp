/**
*  @copyright   Precitec Vision GmbH & Co. KG
*  @author      MM
*  @date        2022
*  @brief       Fliplib filter 'SelectFourPeaks' in component 'Filter_SeamSearch'. Calculates begin and and of both heat affected zones, so also start and end of seam.
*/


#include "selectFourPeaks.h"

#include "module/moduleLogger.h"

#include "overlay/overlayPrimitive.h"   ///< paint overlay
#include "image/image.h"                ///< BImage

#include "filter/algoArray.h"           ///< Doublearray algo

#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec
{
namespace filter
{

const std::string SelectFourPeaks::m_oFilterName = std::string("SelectFourPeaks");

SelectFourPeaks::SelectFourPeaks()
: TransformFilter (m_oFilterName, Poco::UUID{"1FAD3561-ED6A-4CF6-A125-C56CE993909A"})
, m_pPipeInGradientLeft (nullptr)
, m_pPipeInGradientRight (nullptr)
, m_pPipeInMaxFilterLength (nullptr)
, m_pPipeInImageSize (nullptr)
, m_oPipeOutFirstPeak (this, "FirstPeak")
, m_oPipeOutSecondPeak (this, "SecondPeak")
, m_oPipeOutThirdPeak (this, "ThirdPeak")
, m_oPipeOutFourthPeak (this, "FourthPeak")
, m_oHeatAffectedZoneLeftWidth (150)
, m_oHeatAffectedZoneRightWidth (150)
, m_oSeamWidth (220)
, m_oHeatAffectedZoneLeftVariance (10)
, m_oHeatAffectedZoneRightVariance (10)
, m_oSeamVariance (10)
, m_peakWidth (5)
{
    parameters_.add("HeatAffectedZoneLeftWidth",     fliplib::Parameter::TYPE_int, m_oHeatAffectedZoneLeftWidth);
    parameters_.add("HeatAffectedZoneRightWidth",    fliplib::Parameter::TYPE_int, m_oHeatAffectedZoneRightWidth);
    parameters_.add("SeamWidth",                     fliplib::Parameter::TYPE_int, m_oSeamWidth);
    parameters_.add("HeatAffectedZoneLeftVariance",  fliplib::Parameter::TYPE_int, m_oHeatAffectedZoneLeftVariance);
    parameters_.add("HeatAffectedZoneRightVariance", fliplib::Parameter::TYPE_int, m_oHeatAffectedZoneRightVariance);
    parameters_.add("SeamVariance",                  fliplib::Parameter::TYPE_int, m_oSeamVariance);
    parameters_.add("PeakWidth",                     fliplib::Parameter::TYPE_int, m_peakWidth);

    setInPipeConnectors({{Poco::UUID("069635C4-2C73-4794-9199-D0566EB98C28"), m_pPipeInGradientLeft,  "GradientLeft",  1, "gradientLeft"},
                         {Poco::UUID("4711F715-407B-4929-8655-962AC788555C"), m_pPipeInGradientRight, "GradientRight", 1, "gradientRight"},
                         {Poco::UUID("78C4F42E-CE25-4F32-B82C-630B6627E7DF"), m_pPipeInMaxFilterLength, "MaxFilterLength", 1, "maxFilterLength"},
                         {Poco::UUID("02420B7F-7ED0-423E-AFB0-951E7795B9DE"), m_pPipeInImageSize, "ImageSize", 1, "imageSize"}
    });
    setOutPipeConnectors({{Poco::UUID("7778FB22-AA66-4A01-BB89-DE72D809E8A3"), &m_oPipeOutFirstPeak,  "FirstPeak", 0, ""},
                          {Poco::UUID("E7679950-919E-40F7-A5F9-D46FB7103CE7"), &m_oPipeOutSecondPeak, "SecondPeak", 0, ""},
                          {Poco::UUID("EBCBF0F1-8CFA-45D1-8123-17ACD49FD7E7"), &m_oPipeOutThirdPeak,  "ThirdPeak", 0, ""},
                          {Poco::UUID("39854B07-8DDC-461F-8A22-4D547B6AC482"), &m_oPipeOutFourthPeak, "FourthPeak", 0, ""}
    });
    setVariantID(Poco::UUID("039D0E4A-20F6-480C-850D-CF798CE3791B"));
}

SelectFourPeaks::~SelectFourPeaks() = default;

void SelectFourPeaks::setParameter()
{
    TransformFilter::setParameter();

    m_oHeatAffectedZoneLeftWidth     = parameters_.getParameter("HeatAffectedZoneLeftWidth").convert<int>();
    m_oHeatAffectedZoneRightWidth    = parameters_.getParameter("HeatAffectedZoneRightWidth").convert<int>();
    m_oSeamWidth                     = parameters_.getParameter("SeamWidth").convert<int>();
    m_oHeatAffectedZoneLeftVariance  = parameters_.getParameter("HeatAffectedZoneLeftVariance").convert<int>();
    m_oHeatAffectedZoneRightVariance = parameters_.getParameter("HeatAffectedZoneRightVariance").convert<int>();
    m_oSeamVariance                  = parameters_.getParameter("SeamVariance").convert<int>();
    m_peakWidth                      = parameters_.getParameter("PeakWidth").convert<int>();
}

void SelectFourPeaks::paint()
{
    if(m_oVerbosity <= eNone || m_firstPeakOut.size() == 0 || m_smpTrafo.isNull())
    {
        return;
    }

    const int  crossRadius     (6);
    const int  deltaY          (m_imageSize.height / m_numberGradients);
    int        y               (deltaY * 0.5);

    const interface::Trafo& rTrafo (*m_smpTrafo);
    image::OverlayCanvas& rOverlayCanvas (canvas<image::OverlayCanvas>(m_oCounter));
    image::OverlayLayer& rLayerPosition (rOverlayCanvas.getLayerPosition());
    image::OverlayLayer& rLayerText (rOverlayCanvas.getLayerText());

    for (uint lineN = 0; lineN < m_firstPeakOut.size(); ++lineN)
    {
        image::Color colorResult (image::Color::Yellow());
        const geo2d::Point firstPosition (int(m_firstPeakOut.getData()[lineN]), y);
        rLayerPosition.add<image::OverlayCross>(rTrafo(firstPosition), crossRadius, colorResult);
        std::stringstream oTmp1;
        oTmp1 << m_firstPeakOut.getRank()[lineN] << " " << m_firstPeakOut.getData()[lineN];
        rLayerText.add<image::OverlayText>(oTmp1.str(), image::Font(), rTrafo(geo2d::Rect(int(m_firstPeakOut.getData()[lineN]), y, 50, 16)), colorResult);

        colorResult = image::Color::Red();
        const geo2d::Point secondPosition (int(m_secondPeakOut.getData()[lineN]), y);
        rLayerPosition.add<image::OverlayCross>(rTrafo(secondPosition), crossRadius, colorResult);
        std::stringstream oTmp2;
        oTmp2 << m_secondPeakOut.getRank()[lineN] << " " << m_secondPeakOut.getData()[lineN];
        rLayerText.add<image::OverlayText>(oTmp2.str(), image::Font(), rTrafo(geo2d::Rect(int(m_secondPeakOut.getData()[lineN]), y, 50, 16)), colorResult);

        colorResult = image::Color::Cyan();
        const geo2d::Point thirdPosition (int(m_thirdPeakOut.getData()[lineN]), y);
        rLayerPosition.add<image::OverlayCross>(rTrafo(thirdPosition), crossRadius, colorResult);
        std::stringstream oTmp3;
        oTmp3 << m_thirdPeakOut.getRank()[lineN] << " " << m_thirdPeakOut.getData()[lineN];
        rLayerText.add<image::OverlayText>(oTmp3.str(), image::Font(), rTrafo(geo2d::Rect(int(m_thirdPeakOut.getData()[lineN]), y, 50, 16)), colorResult);

        colorResult = image::Color::m_oMagenta;
        const geo2d::Point fourthPosition (int(m_fourthPeakOut.getData()[lineN]), y);
        rLayerPosition.add<image::OverlayCross>(rTrafo(fourthPosition), crossRadius, colorResult);
        std::stringstream oTmp4;
        oTmp4 << m_fourthPeakOut.getRank()[lineN] << " " << m_fourthPeakOut.getData()[lineN];
        rLayerText.add<image::OverlayText>(oTmp4.str(), image::Font(), rTrafo(geo2d::Rect(int(m_fourthPeakOut.getData()[lineN]), y, 50, 16)), colorResult);

        y += deltaY;
    }
}

bool SelectFourPeaks::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
    if (p_rPipe.tag() == "gradientLeft")
    {
        m_pPipeInGradientLeft = dynamic_cast<fliplib::SynchronePipe<interface::GeoVecDoublearray>*>(&p_rPipe);
    }
    else if (p_rPipe.tag() == "gradientRight")
    {
        m_pPipeInGradientRight = dynamic_cast<fliplib::SynchronePipe<interface::GeoVecDoublearray>*>(&p_rPipe);
    }
    else if (p_rPipe.tag() == "maxFilterLength")
    {
        m_pPipeInMaxFilterLength = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&p_rPipe);
    }
    else if (p_rPipe.tag() == "imageSize")
    {
        m_pPipeInImageSize = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&p_rPipe);
    }

    return BaseFilter::subscribe(p_rPipe, p_oGroup);
}

void SelectFourPeaks::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e)
{
    poco_assert_dbg(m_pPipeInGradientLeft != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInGradientRight != nullptr); // to be asserted by graph editor

    const interface::GeoVecDoublearray& rGradientLeftIn = m_pPipeInGradientLeft->read(m_oCounter);
    const interface::GeoVecDoublearray& rGradientRightIn = m_pPipeInGradientRight->read(m_oCounter);

    const geo2d::Doublearray& rImageSizeIn = m_pPipeInImageSize->read(m_oCounter).ref();
    const geo2d::VecDoublearray& gradientLeftIn = rGradientLeftIn.ref();
    const geo2d::VecDoublearray& gradientRightIn = rGradientRightIn.ref();

    const geo2d::Range heatAffectedZoneLeftRange (m_oHeatAffectedZoneLeftWidth - m_oHeatAffectedZoneLeftWidth * m_oHeatAffectedZoneLeftVariance * 0.01,
                                                  m_oHeatAffectedZoneLeftWidth + m_oHeatAffectedZoneLeftWidth * m_oHeatAffectedZoneLeftVariance * 0.01);
    const geo2d::Range seamRange (m_oSeamWidth - m_oSeamWidth * m_oSeamVariance * 0.01, m_oSeamWidth + m_oSeamWidth * m_oSeamVariance * 0.01);
    const geo2d::Range heatAffectedZoneRightRange (m_oHeatAffectedZoneRightWidth - m_oHeatAffectedZoneRightWidth * m_oHeatAffectedZoneRightVariance * 0.01,
                                                   m_oHeatAffectedZoneRightWidth + m_oHeatAffectedZoneRightWidth * m_oHeatAffectedZoneRightVariance * 0.01);

    // input validity check
    auto defaultWidthsTooHigh = [&]() {
        if (static_cast<uint>(heatAffectedZoneLeftRange.start() + seamRange.start() + heatAffectedZoneRightRange.start()) >= gradientLeftIn.front().size())
        {
            wmLog(eWarning, "Filter %s: Default widths are too high.\n", m_oFilterName);
            return true;
        }
        return false;
    };

    if (inputIsInvalid(rGradientLeftIn) || inputIsInvalid(rGradientRightIn) || inputIsInvalid(rImageSizeIn) || gradientLeftIn.size() != gradientRightIn.size() || defaultWidthsTooHigh())
    {
        const interface::GeoDoublearray oGeoFirstPeakOut  (rGradientLeftIn.context(),  m_firstPeakOut,  rGradientLeftIn.analysisResult(),  interface::NotPresent);
        const interface::GeoDoublearray oGeoSecondPeakOut (rGradientRightIn.context(), m_secondPeakOut, rGradientRightIn.analysisResult(), interface::NotPresent);
        const interface::GeoDoublearray oGeoThirdPeakOut  (rGradientLeftIn.context(),  m_thirdPeakOut,  rGradientLeftIn.analysisResult(),  interface::NotPresent);
        const interface::GeoDoublearray oGeoFourthPeakOut (rGradientRightIn.context(), m_fourthPeakOut, rGradientRightIn.analysisResult(), interface::NotPresent);
        preSignalAction();
        m_oPipeOutFirstPeak.signal(oGeoFirstPeakOut);
        m_oPipeOutSecondPeak.signal(oGeoSecondPeakOut);
        m_oPipeOutThirdPeak.signal(oGeoThirdPeakOut);
        m_oPipeOutFourthPeak.signal(oGeoFourthPeakOut);

        return;
    }

    m_maxFilterLength = int(m_pPipeInMaxFilterLength->read(m_oCounter).ref().getData().front());
    m_imageSize.width = (int)rImageSizeIn.getData()[0];
    m_imageSize.height = (int)rImageSizeIn.getData()[1];
    m_smpTrafo = rGradientLeftIn.context().trafo();
    m_numberGradients = gradientLeftIn.size();

    if (m_smpTrafo != rGradientRightIn.context().trafo())
    {
        wmLog(eWarning, "Filter %s: different trafos for right and left gradients.", m_oFilterName);
    }

    poco_assert_dbg(!gradientLeftIn.empty());

    // (re)initialize output based on input dimension
    m_firstPeakOut.assign (gradientLeftIn.size(), 0, eRankMin);
    m_secondPeakOut.assign(gradientLeftIn.size(), 0, eRankMin);
    m_thirdPeakOut.assign (gradientLeftIn.size(), 0, eRankMin);
    m_fourthPeakOut.assign(gradientLeftIn.size(), 0, eRankMin);

    m_maxima.resize(gradientLeftIn.size());
    const std::vector<double> globalMaxima = searchAllMaxima(gradientLeftIn, gradientRightIn);

    for (uint lineN = 0; lineN < gradientLeftIn.size(); ++lineN)
    {
        auto& maxima = m_maxima[lineN];
        peakArray peaks; ///< begin/end left heat affected zone, begin/end right heat affected zone

        // Check from left to right for every maximum to find for the given distances the other three maxima.
        // Compute for these four maxima error and notice the maxima with the least error.
        const uint numberMaxima = maxima.size();
        double leastError = 10;
        for (uint j = 0; j < numberMaxima; ++j)
        {
            peaks.fill(std::pair<int, Gradient>(0, Gradient::undefined));
            peaks[0] = maxima[j]; // Gradient is never undefined in maxima
            int peakNumber = 1;
            geo2d::Range range = heatAffectedZoneLeftRange;
            std::vector<peakArray> peakQueue;
            std::vector<uint> peakIndexQueue;
            std::vector<int> peakNumberQueue;
            int startIndex = j + 1;
            do
            {
                if (!peakQueue.empty())
                {
                    peaks = peakQueue.back();
                    peakQueue.pop_back();
                    startIndex = peakIndexQueue.back();
                    peakIndexQueue.pop_back();
                    peakNumber = peakNumberQueue.back() + 1;
                    peakNumberQueue.pop_back();
                    if (peakNumber == 2)
                    {
                        range = seamRange;
                    } else if (peakNumber == 3)
                    {
                        range = heatAffectedZoneRightRange;
                    }
                }

                // Check for four peaks with the given distance. If there are different options for these peaks, remember all in peakQueue.
                for (uint k = startIndex; k < numberMaxima && peakNumber < m_numberPeaks; ++k)
                {
                    if (maxima[k].first < peaks[peakNumber - 1].first + range.start())
                    {
                        continue;
                    } else if (maxima[k].first > peaks[peakNumber - 1].first + range.end())
                    {
                        // no maximum found in the required distance
                        if (peaks[peakNumber].first == 0)
                        {
                            break;
                        }
                        // maximum found, begin search for next (if there is a next)
                        peakNumber++;
                        if (peakNumber == 2)
                        {
                            range = seamRange;
                        } else if (peakNumber == 3)
                        {
                            range = heatAffectedZoneRightRange;
                        }
                        k--; // check again maxima[k] for the next pos
                    } else if (peaks[peakNumber].first == 0)
                    {
                        peaks[peakNumber] = maxima[k];
                    } else if (peaks[peakNumber].first > 0)
                    {
                        auto tempPeak = peaks;
                        tempPeak[peakNumber] = maxima[k];
                        peakQueue.push_back(tempPeak);
                        peakIndexQueue.push_back(k + 1);
                        peakNumberQueue.push_back(peakNumber);
                    }
                }

                // The peaks with the smallest error are the result
                if (peaks[3].first > 0)
                {
                    auto& gradientLeft = gradientLeftIn[lineN].getData();
                    auto& gradientRight = gradientRightIn[lineN].getData();
                    double error = computeError(peaks, gradientLeft, gradientRight, globalMaxima[lineN]);

                    if (error < leastError)
                    {
                        leastError = error;
                        m_firstPeakOut.getData()[lineN]  = peaks[0].first;
                        m_secondPeakOut.getData()[lineN] = peaks[1].first;
                        m_thirdPeakOut.getData()[lineN]  = peaks[2].first;
                        m_fourthPeakOut.getData()[lineN] = peaks[3].first;
                        m_firstPeakOut.getRank()[lineN]  = eRankMax;
                        m_secondPeakOut.getRank()[lineN] = eRankMax;
                        m_thirdPeakOut.getRank()[lineN]  = eRankMax;
                        m_fourthPeakOut.getRank()[lineN] = eRankMax;
                    }
                }
            } while (!peakQueue.empty());
        }
    }

    const double rank = std::min(rGradientLeftIn.rank(), rGradientRightIn.rank());
    const interface::GeoDoublearray oGeoFirstPeakOut (rGradientLeftIn.context(),  m_firstPeakOut,  rGradientLeftIn.analysisResult(),  rank);
    const interface::GeoDoublearray oGeoSecondPeakOut(rGradientRightIn.context(), m_secondPeakOut, rGradientRightIn.analysisResult(), rank);
    const interface::GeoDoublearray oGeoThirdPeakOut (rGradientLeftIn.context(),  m_thirdPeakOut,  rGradientLeftIn.analysisResult(),  rank);
    const interface::GeoDoublearray oGeoFourthPeakOut(rGradientRightIn.context(), m_fourthPeakOut, rGradientRightIn.analysisResult(), rank);

    preSignalAction();
    m_oPipeOutFirstPeak. signal(oGeoFirstPeakOut);
    m_oPipeOutSecondPeak.signal(oGeoSecondPeakOut);
    m_oPipeOutThirdPeak. signal(oGeoThirdPeakOut);
    m_oPipeOutFourthPeak.signal(oGeoFourthPeakOut);
}

double SelectFourPeaks::computeError(const peakArray& peaks, const std::vector<double>& gradientLeft, const std::vector<double>& gradientRight,
                                     const double maxValue)
{
    double error = 0;
    for (int i = 0; i < m_numberPeaks; ++i)
    {
        double sum = 0;
        double me = 0;
        if (peaks[i].first + m_peakWidth * 0.5 > m_imageSize.width)
        {
            break;
        }
        for (int j = peaks[i].first - m_peakWidth * 0.5; j < peaks[i].first + m_peakWidth * 0.5; ++j)
        {
            if (peaks[i].second == Gradient::left)
            {
                sum += maxValue - gradientLeft[j];
            } else if (peaks[i].second == Gradient::right)
            {
                sum += maxValue - gradientRight[j];
            }
            me += maxValue;
        }
        error += sum/me;
    }
    return error / m_numberPeaks;
}

std::vector<double> SelectFourPeaks::searchAllMaxima(const geo2d::VecDoublearray& gradientLeftIn, const geo2d::VecDoublearray& gradientRightIn)
{
    std::vector<double> globalMaxima;
    globalMaxima.assign(gradientLeftIn.size(), 0);

    for (uint i = 0; i < gradientLeftIn.size(); ++i)
    {
        int maxLeftGradient = 0;
        int maxRightGradient = 0;

        auto& maxima = m_maxima[i];
        maxima.clear();
        geo2d::Range searchArea;
        searchArea.start() = m_maxFilterLength;
        searchArea.end() = gradientLeftIn[i].size() - m_maxFilterLength;

        // compute alle local maxima (only positive)
        std::pair<int, Gradient> lastMaximum (0, Gradient::undefined);
        while (searchArea.start() < searchArea.end())
        {
            if (lastMaximum.first == maxLeftGradient && maxLeftGradient >= 0)
            {
                maxLeftGradient = searchLocalMax(gradientLeftIn[i], searchArea);
                if (gradientLeftIn[i].getData()[maxLeftGradient] > globalMaxima[i])
                {
                    globalMaxima[i] = gradientLeftIn[i].getData()[maxLeftGradient];
                }
            }
            if (lastMaximum.first == maxRightGradient && maxRightGradient >= 0)
            {
                maxRightGradient = searchLocalMax(gradientRightIn[i], searchArea);
                if (gradientRightIn[i].getData()[maxRightGradient] > globalMaxima[i])
                {
                    globalMaxima[i] = gradientRightIn[i].getData()[maxRightGradient];
                }
            }

            // set lastMaximum to the maximum that is more to the left
            if ((maxLeftGradient > maxRightGradient && maxRightGradient > 0) || maxLeftGradient < 0)
            {
                lastMaximum.first = maxRightGradient;
                lastMaximum.second = Gradient::right;
            } else if (maxLeftGradient > 0)
            {
                lastMaximum.first = maxLeftGradient;
                lastMaximum.second = Gradient::left;
            }

            if (maxLeftGradient == -1 && maxRightGradient == -1)
            {
                break; // no more maxima can be found
            }

            maxima.push_back(lastMaximum);
            searchArea.start() = lastMaximum.first + 1;
        }
    }
    return globalMaxima;
}

int SelectFourPeaks::searchLocalMax(const geo2d::Doublearray &gradient, const geo2d::Range searchArea)
{
    const std::vector<double> &value = gradient.getData();
    const std::vector<int> &rank = gradient.getRank();

    const int deltaX = 1;

    if (deltaX > searchArea.end() - searchArea.start())
    {
        return -1;
    }

    int tempMax = 0;
    // in the begin of the search area there is no left value to compare
    for (int x = searchArea.start(); x < searchArea.start() + deltaX; ++x)
    {
        const bool biggerThanLastMax = searchArea.start() == 0 || value[x] > value[searchArea.start() - 1];
        if (rank[x] > 0 && value[x] > value[x + deltaX] && biggerThanLastMax && tempMax < x)
        {
            tempMax = x;
        }
    }
    if (tempMax > 0)
    {
        return tempMax;
    }

    for (int x = searchArea.start() + deltaX; x < searchArea.end() - deltaX; ++x)
    {
        if (rank[x] > 0 && value[x] > (value[x - deltaX]) && value[x] > value[x + deltaX])
        {
            return x;
        }
    }

    tempMax = 0;
    // in the end of the search area there is no right value to compare
    for (int x = searchArea.end() - deltaX; x < searchArea.end(); ++x)
    {
        if (rank[x] > 0 && value[x] > value[x - deltaX])
        {
            if (tempMax < x)
            {
                tempMax = x;
            }
        }
    }
    if (tempMax > 0)
    {
        return tempMax;
    }

    return -1; // no maximum found
}

}
}

/**
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     MM
 *  @date       2022
 *  @brief      Fliplib filter 'SelectFourPeaks' in component 'Filter_SeamSearch'. Calculates begin and and of both heat affected zones, so also start and end of seam.
 */

#pragma once

#include "fliplib/Fliplib.h"            ///< fliplib image processing pipes and filter framework
#include "fliplib/TransformFilter.h"    ///< base class
#include "fliplib/PipeEventArgs.h"      ///< event processing
#include "fliplib/SynchronePipe.h"      ///< in- / output

#include "geo/geo.h"                    ///< GeoVecDoublearray
#include "geo/array.h"                  ///< TArray
#include "geo/size.h"                   ///< size

namespace precitec
{
namespace filter
{

class FILTER_API SelectFourPeaks  : public fliplib::TransformFilter
{
public:
    SelectFourPeaks();
    ~SelectFourPeaks();

    static const std::string m_oFilterName;

    /// set filter parameter defined in database / xml file
    void setParameter();
    /// paint overerlay primitives
    void paint();


protected:
    /// in pipe registrastion
    bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
    /// in pipe event processing
    void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg);


private:
    enum class Gradient
    {
        undefined = 0,
        left,
        right
    };
    static const int m_numberPeaks = 4;

    typedef std::array<std::pair<int, Gradient>, m_numberPeaks> peakArray;

    /***
     * Finds the next local maximum. Values with bad rank will be ignored.
     * @param gradient - gradient where the maximum is searched
     * @param searchArea - range in which is searched for the maximum
     * @return - x position of local maximum or -1 if is there is none
     **/
    int searchLocalMax(const geo2d::Doublearray &gradient, const geo2d::Range searchArea);
    std::vector<double> searchAllMaxima(const geo2d::VecDoublearray& gradientLeftIn, const geo2d::VecDoublearray& gradientRightIn);
    /// computes one error for four peaks
    double computeError(const peakArray& peaks, const std::vector<double>& gradientLeft, const std::vector<double>& gradientRight,
                        const double maxValue);

    const fliplib::SynchronePipe<interface::GeoVecDoublearray>* m_pPipeInGradientLeft;
    const fliplib::SynchronePipe<interface::GeoVecDoublearray>* m_pPipeInGradientRight;
    const fliplib::SynchronePipe<interface::GeoDoublearray>*    m_pPipeInMaxFilterLength;
    const fliplib::SynchronePipe<interface::GeoDoublearray>*    m_pPipeInImageSize;
    fliplib::SynchronePipe<interface::GeoDoublearray>           m_oPipeOutFirstPeak;
    fliplib::SynchronePipe<interface::GeoDoublearray>           m_oPipeOutSecondPeak;
    fliplib::SynchronePipe<interface::GeoDoublearray>           m_oPipeOutThirdPeak;
    fliplib::SynchronePipe<interface::GeoDoublearray>           m_oPipeOutFourthPeak;

    int m_maxFilterLength;
    geo2d::Size m_imageSize;

    interface::SmpTrafo m_smpTrafo;
    int m_numberGradients; ///< size of gradient left. gradient right should have the same size.

    std::vector<std::vector<std::pair<int, Gradient>>> m_maxima; ///< all local maxima and at which gradient (left or right) it is

    geo2d::Doublearray m_firstPeakOut;
    geo2d::Doublearray m_secondPeakOut;
    geo2d::Doublearray m_thirdPeakOut;
    geo2d::Doublearray m_fourthPeakOut;

    // parameters
    int m_oHeatAffectedZoneLeftWidth;
    int m_oHeatAffectedZoneRightWidth;
    int m_oSeamWidth;
    int m_oHeatAffectedZoneLeftVariance;  ///< Allowed variance of the width, in percent
    int m_oHeatAffectedZoneRightVariance; ///< Allowed variance of the width, in percent
    int m_oSeamVariance;                  ///< Allowed variance of the width, in percent
    int m_peakWidth;
};

}
}

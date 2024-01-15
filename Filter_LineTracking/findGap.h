/**
 * @file
 * @copyright   Precitec Vision GmbH & Co. KG
 * @author      MM
 * @date        2022
 * @brief       Laserline tracking filter, finds gap in laseline. If there is no gap, than returns the end of the line.
 */

#pragma once


#include "fliplib/Fliplib.h" ///< fliplib image processing pipes and filter framework
#include "fliplib/TransformFilter.h" ///< base class
#include <fliplib/SynchronePipe.h> ///< Inputs and outputs
#include "overlay/overlayPrimitive.h" ///< Color
#include <geo/geo.h> ///< VecDoublearray


namespace precitec
{
namespace filter
{

class FILTER_API FindGap  : public fliplib::TransformFilter
{
public:
    FindGap();
    ~FindGap();

    static const std::string m_filterName;

    void setParameter() override;
    void paint() override;
    void arm(const fliplib::ArmStateBase& state) override;

protected:
    bool subscribe(fliplib::BasePipe& pipe, int group) override;
    void proceed(const void* sender, fliplib::PipeEventArgs& e) override;

private:
    void setDefaultOut(const std::vector<double>& line);

    const fliplib::SynchronePipe<interface::GeoVecDoublearray>* m_pipeInLine;

    fliplib::SynchronePipe<interface::GeoDoublearray>* m_pipePositionX;
    fliplib::SynchronePipe<interface::GeoDoublearray>* m_pipePositionY;

    geo2d::Doublearray m_outX;
    geo2d::Doublearray m_outY;

    interface::SmpTrafo m_smpTrafo; ///< roi translation

    std::vector<double> m_paintLine;

    SearchDirType m_searchDirection;

    int m_numberValuesForAveraging;
    int m_gapWidth;
    int m_maxNumberOfGaps;
    int m_maxJumpDownY; ///< Possible gap if difference from mean value of m_numberValuesForAveraging values to next value is bigger than m_maxJumpDownY
    int m_maxJumpUpY; ///< Possible gap if difference from mean value of m_numberValuesForAveraging values to next value is smaller than m_maxJumpDownY
    int m_maxJumpX; ///< Maximal allowed jump on x axis, if next point after jump is on the same height (inconclusively same y pos)

    geo2d::Point m_result;
    image::Color m_color;
};

}
}

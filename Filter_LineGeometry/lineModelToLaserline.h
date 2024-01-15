
/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     MM
 *  @date       2022
 *  @brief      Converts lines type LineModel into Doublearray.
 */

#pragma once


#include <fliplib/Fliplib.h> ///< Fliplib image processing pipes and filter framework
#include <fliplib/TransformFilter.h> ///< Basisclass
#include <fliplib/PipeEventArgs.h> ///< Eventprocessing
#include <fliplib/SynchronePipe.h> ///< Inputs and outputs

#include <geo/geo.h> ///< Size2d, VecDoublearray
#include <math/2D/LineEquation.h>

#include "overlay/overlayCanvas.h"
#include "MultipleIndicesStrategy.h"


namespace precitec
{
namespace filter
{

/**
* @ingroup Filter_LineGeometry
* @brief Converts lines type LineModel into Doublearray.
*/
class FILTER_API LineModelToLaserline  : public fliplib::TransformFilter
{
public:
    LineModelToLaserline();

    static const std::string m_filterName;
    static const std::string m_startInName;
    static const std::string m_endInName;

    void setParameter() override;
    void paint() override;

protected:
    bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) override;
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e) override;

private:
    // Local Strategy enum, because if using directly MultipleIndicesStratecy, cast int from parameter to enum would go wrong.
    enum class Strategy
    {
        UseFirstIndexes,
        ExtractShortestLine,
        ExtractLongestLine
    };

    const fliplib::SynchronePipe<interface::GeoLineModelarray>* m_pipeEquationIn;
    const fliplib::SynchronePipe<interface::GeoDoublearray>* m_pipeStartIn;
    const fliplib::SynchronePipe<interface::GeoDoublearray>* m_pipeEndIn;
    fliplib::SynchronePipe<interface::GeoVecDoublearray> m_pipeLineOut;

    image::Color m_paintColor; ///< parameter
    Strategy m_strategy = Strategy::ExtractLongestLine; ///< parameter, longest line without padding

    std::vector<double> m_paintLine;
    interface::SmpTrafo m_spTrafo;
};

}
}

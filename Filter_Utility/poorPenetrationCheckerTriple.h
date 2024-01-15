/**
* @file
* @copyright	Precitec Vision GmbH & Co. KG
* @author		MM
* @date		    2021
* @brief		This filter checks if a poor penetration candidate is a real poor penetration, analogue to PoorPenetrationChecker but all parameters were tripled.
*/

#pragma once

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include <geo/geo.h>

#include "overlay/overlayPrimitive.h"
#include "poorPenetrationPaint.h"

namespace precitec
{
namespace filter
{

class FILTER_API PoorPenetrationCheckerTriple : public fliplib::TransformFilter
{
public:
    PoorPenetrationCheckerTriple();
    ~PoorPenetrationCheckerTriple();

    /**
    * @brief Set filter parameters.
    */
    void setParameter() override;

    void paint() override;

protected:
    /**
    * @brief In-pipe registration.
    * @param p_rPipe Reference to pipe that is getting connected to the filter.
    * @param p_oGroup Group number (0 - proceed is called whenever a pipe has a data element, 1 - proceed is only called when all pipes have a data element).
    */
    bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) override;

    /**
    * @brief Processing routine.
    */
    void proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rE) override;

private:
    interface::SmpTrafo m_oSpTrafo; ///< roi translation

    const fliplib::SynchronePipe< interface::GeoPoorPenetrationCandidatearray >* m_pPipeInData; ///< Data in-pipe.
    fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutData;                         ///< Data out-pipe.

    // parameters
    int m_oActiveParamSets;

    int m_oActiveParams;
    int m_oMinWidth;
    int m_oMaxWidth;
    int m_oMinLength;
    int m_oMaxLength;
    int m_oMinGradient;
    int m_oMaxGradient;
    int m_oMinGreyvalGap;
    int m_oMaxGreyvalGap;
    int m_oMinRatioInnerOuter;
    int m_oMaxRatioInnerOuter;
    int m_oMinStandardDeviation;
    int m_oMaxStandardDeviation;
    int m_oMinDevelopedLength;
    int m_oMaxDevelopedLength;

    int m_oActiveParams1;
    int m_oMinWidth1;
    int m_oMaxWidth1;
    int m_oMinLength1;
    int m_oMaxLength1;
    int m_oMinGradient1;
    int m_oMaxGradient1;
    int m_oMinGreyvalGap1;
    int m_oMaxGreyvalGap1;
    int m_oMinRatioInnerOuter1;
    int m_oMaxRatioInnerOuter1;
    int m_oMinStandardDeviation1;
    int m_oMaxStandardDeviation1;
    int m_oMinDevelopedLength1;
    int m_oMaxDevelopedLength1;

    int m_oActiveParams2;
    int m_oMinWidth2;
    int m_oMaxWidth2;
    int m_oMinLength2;
    int m_oMaxLength2;
    int m_oMinGradient2;
    int m_oMaxGradient2;
    int m_oMinGreyvalGap2;
    int m_oMaxGreyvalGap2;
    int m_oMinRatioInnerOuter2;
    int m_oMaxRatioInnerOuter2;
    int m_oMinStandardDeviation2;
    int m_oMaxStandardDeviation2;
    int m_oMinDevelopedLength2;
    int m_oMaxDevelopedLength2;

    std::vector<std::vector<InfoLine>> m_allInfoLines; // store InfoLines for paint()

    /**
     * @brief Counts how many PPCandidates already exists with the same rectangle-measurements. Used to decide the position of the number at the overlay.
     */
    int getNumberOfStoredPP(const geo2d::PoorPenetrationCandidate& cand, const std::vector<geo2d::PoorPenetrationCandidate> allPP);

    int checkForErrors(const interface::GeoPoorPenetrationCandidatearray &candidates, int minLength, int maxLength,int minWidth, int maxWidth, int minGradient, int maxGradient,
                    int minGreyvalGap, int maxGreyvalGap, int minRatioInnerOuter, int maxRatioInnerOuter, int minStandardDeviation, int maxStandardDeviation,
                    int minDevelopedLength, int maxDevelopedLength, int activeParams);

    PoorPenetrationOverlay m_overlay;
};

}
}

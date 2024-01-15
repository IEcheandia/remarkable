/*!
 *  @copyright      Precitec Vision GmbH & Co. KG
 *  @author         Urs Gisiger (GUR)
 *  @date           2022
 *  @file
 *  @brief          Fliplib filter 'SelectPeaksCont' in component 'Filter_SeamSearch'.
 *                  Copied from filter 'SelectPeaks', added inpipes 'StartPosition' and 'CenterPosition'.
 *                  Calculates right and left seam position, using the 'CenterPosition' from the before
 *                  image as "expected" seam center position for the actual image.
 */

#ifndef SELECT_PEAKS_CONT_H_
#define SELECT_PEAKS_CONT_H_

#include "fliplib/Fliplib.h"            ///< fliplib image processing pipes and filter framework
#include "fliplib/TransformFilter.h"    ///< base class
#include "fliplib/PipeEventArgs.h"      ///< event processing
#include "fliplib/SynchronePipe.h"      ///< in- / output

#include "common/frame.h"               ///< ImageFrame
#include "geo/geo.h"                    ///< GeoVecDoublearray
#include "geo/array.h"                  ///< TArray
#include "geo/size.h"                   ///< size

namespace precitec {
namespace filter {

/**
 * Fliplib filter 'SelectPeaksXT' in component 'Filter_SeamSearch'. Calculates right and left seam position from gradients.
 */
class FILTER_API SelectPeaksCont : public fliplib::TransformFilter
{
public:
    SelectPeaksCont();

    static const std::string m_oFilterName;         ///< Name Filter
    static const std::string m_oPipeOutName1;       ///< Name Out-Pipe
    static const std::string m_oPipeOutName2;       ///< Name Out-Pipe

    //! Calculates right and left seam position from line/stripe gradients.
    /*!
    \param p_rGradientLeftIn      left search gradient
    \param p_rGradientRightIn     right search gradient
    \param p_oMaxFilterLenght     max filter lenght for boundary lenght
    \param p_oDefaultSeamWidth    default seam width
    \param p_oThresholdLeft       gradient threshold for left peaks
    \param p_oThresholdRight      gradient threshold for right peaks
    \param p_rContourLeftOut      left seam position.
    \param p_rContourRightOut     right seam position.
    \return void
    \sa -
    */
    void calcSelectPeaksCont(
        const geo2d::VecDoublearray &p_rGradientLeftIn,
        const geo2d::VecDoublearray &p_rGradientRightIn,
        int                         p_oMaxFilterLenght,
        int                         p_oDefaultSeamWidth,
        int                         p_oThresholdLeft,
        int                         p_oThresholdRight,
        geo2d::Doublearray          &p_rContourLeftOut,
        geo2d::Doublearray          &p_rContourRightOut
    );


    /// set filter parameter defined in database / xml file
    void setParameter();
    /// paint overlay primitives
    void paint();


protected:
    /// in-pipe registrastion
    bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
    /// in-pipe event processing
    void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg);


private:

    //! (Re)initializes output member structures based on input structure dimension.
    /*!
    \param  p_rGradientLeftIn    left search gradient
    \param  p_rGradientRightIn   right search gradient
    \return void
    \sa -
    */
    void reinitialize(
        const geo2d::VecDoublearray     &p_rGradientLeftIn,
        const geo2d::VecDoublearray     &p_rGradientRightIn
    ); ///< (re)initialization of output structure


    typedef fliplib::SynchronePipe< interface::GeoVecDoublearray >  line_pipe_t;
    typedef fliplib::SynchronePipe< interface::GeoDoublearray >     scalar_pipe_t;
    typedef std::vector<double>::const_iterator                     vec_double_cit_t;
    typedef std::vector<double>::const_reverse_iterator             vec_double_crit_t;

    const line_pipe_t *             m_pPipeInGradientLeft;      ///< in pipe left search gradient
    const line_pipe_t *             m_pPipeInGradientRight;     ///< in pipe right search gradient
    const scalar_pipe_t *           m_pPipeInMaxFLenght;        ///< in pipe max filter length WORKAROUND
    const scalar_pipe_t *           m_pPipeInImgSize;           ///< in pipe image size
    const scalar_pipe_t *           m_pPipeInDefaultSeamWidth;  ///< in pipe expected seam width
    const scalar_pipe_t *           m_pPipeInCenterPos;         ///< in pipe expected seam center position
    const scalar_pipe_t *           m_pPipeInStartPos;          ///< in pipe expected seam center position in first image
    scalar_pipe_t                   m_oPipeOutContourLeft;      ///< out pipe
    scalar_pipe_t                   m_oPipeOutContourRight;     ///< out pipe

    std::vector<vec_double_cit_t>   m_oPeaksL;
    std::vector<vec_double_cit_t>   m_oPeaksR;
    geo2d::Doublearray              m_oContourLeftOut;          ///< output left seamposition
    geo2d::Doublearray              m_oContourRightOut;         ///< output right seamposition
    geo2d::Size                     m_oImageSize;               ///< image size

    int                             m_oMaxFilterLenght;         ///< max filter lenght for boundary lenght
    int                             m_oDefaultSeamWidth;        ///< default seam width
    int                             m_oCenterPosROI;            ///< Expected 'Seam center position' inside the ROI
    int                             m_oStartPos;                ///< Expected 'Start position' of the seam
    int                             m_oOldSeamPosLeft;          ///< Expected 'Left seam position' from before images
    int                             m_oOldSeamPosRight;         ///< Expected 'Right seam position' from before images
    int                             m_oOldSeamPosLeftROI;       ///< Expected 'Left seam position' inside the ROI
    int                             m_oOldSeamPosRightROI;      ///< Expected 'Right seam position' inside the ROI
    int                             m_oThresholdLeft;           ///< gradient threshold for left peaks
    int                             m_oThresholdRight;          ///< gradient threshold for right peaks
}; // SelectPeaksCont

} // namespace filter
} // namespace precitec

#endif /*SELECT_PEAKS_CONT_H_*/




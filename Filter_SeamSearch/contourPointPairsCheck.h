/*!
 *  @copyright        Precitec Vision GmbH & Co. KG
 *  @author           Urs Gisiger (GUR)
 *  @date             2022
 *  @file             contourPointPairsCheck.h
 *  @brief            Fliplib filter 'ContourPointPairsCheck' in component 'Filter_SeamSearch'. Checks left/right contour points for special conditions.
 */

#ifndef CONTOUR_POINT_PAIRS_CHECK_H_
#define CONTOUR_POINT_PAIRS_CHECK_H_

#include "fliplib/Fliplib.h"            ///< fliplib image processing pipes and filter framework
#include "fliplib/TransformFilter.h"    ///< base class
#include "fliplib/PipeEventArgs.h"      ///< event processing
#include "fliplib/SynchronePipe.h"      ///< in- / output

#include "geo/geo.h"                    ///< GeoDoublearray
#include "geo/array.h"                  ///< TArray
#include "geo/size.h"                   ///< size

namespace precitec
{
    using interface::GeoDoublearray;
    using geo2d::Doublearray;

namespace filter
{
    using fliplib::SynchronePipe;


///  ContourPointPairsCheck filter.

/**
 * Fliplib filter 'ContourPointPairsCheck' in component 'Filter_SeamSearch'. Checks left/right contour points for special conditions.
 */
class FILTER_API ContourPointPairsCheck  : public fliplib::TransformFilter
{

public:

    /// CTor.
    ContourPointPairsCheck();
    /// DTor.
    virtual ~ContourPointPairsCheck();

    static const std::string m_oFilterName;          ///< Name Filter
    static const std::string m_oPipeOutName1;        ///< Name Out-Pipe
    static const std::string m_oPipeOutName2;        ///< Name Out-Pipe
    static const std::string m_oPipeOutName3;        ///< Name Out-Pipe

    //! Checks left and right contour points for "number", "standard dev. width", "max dist. position".

    /*!
      \param p_rContourLIn                 left search gradient
      \param p_rContourRIn                 right search gradient
      \param p_oNumberBigGradient          Min. number of point pairs with big gradients
      \param p_oMaxStandardDeviationWidth  Max. standard deviation of point distances
      \param p_oMaxDistancePosition        Max. distance of point pair center positions
      \param p_oSeamPosL                   output left seam position
      \param p_oSeamPosR                   output right seam position
      \param p_oSeamPosOK                  output found SeamPos is OK or not
      \return void
      \sa -
    */

    static void calcConditionsForSeamPos
    (
        const Doublearray  &p_rContourLIn,
        const Doublearray  &p_rContourRIn,
        int                p_oNumberBigGradient,
        double             p_oMaxStandardDeviationWidth,
        int                p_oMaxDistancePosition,
        geo2d::Doublearray &p_oSeamPosL,
        geo2d::Doublearray &p_oSeamPosR,
        bool               &p_oSeamPosOK
    );

    /// set filter parameter defined in database / xml file
    void setParameter();

    /// paint overlay primitives
    void paint();


protected:

    /// in pipe registrastion
    bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
    /// in pipe event processing
    void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg);


private:

    //! (Re)initializes output member structures based on input structure dimension.

    void reinitialize ();

    typedef fliplib::SynchronePipe< interface::GeoVecDoublearray >  line_pipe_t;
    typedef fliplib::SynchronePipe< interface::GeoDoublearray >     scalar_pipe_t;

    const scalar_pipe_t*    m_pPipeInContourL;             ///< in pipe left contour points
    const scalar_pipe_t*    m_pPipeInContourR;             ///< in pipe right contour points
    const scalar_pipe_t*    m_pPipeInImgSize;              ///< in pipe img size

    scalar_pipe_t           m_oPipeOutSeamPosL;            ///< out pipe
    scalar_pipe_t           m_oPipeOutSeamPosR;            ///< out pipe
    scalar_pipe_t           m_oPipeOutSeamPosOK;           ///< out pipe

    geo2d::Doublearray      m_oSeamPosL;                   ///< output seam position left
    geo2d::Doublearray      m_oSeamPosR;                   ///< output seam position right
    geo2d::Size             m_oImageSize;                  ///< image size

    int                     m_oNumberBigGradient;          ///< number of found big gradient pairs
    double                  m_oMaxStandardDeviationWidth;  ///< max standard deviaton of width
    int                     m_oMaxDistancePosition;        ///< max distance to mean center position
    bool                    m_oSeamPosOK;                  ///< if found SeamPos is OK or not

}; // ContourPointPairsCheck

} // namespace filter
} // namespace precitec

#endif /*CONTOUR_POINT_PAIRS_CHECK_H_*/




/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		2015
 * 	@brief 		This filter gets a seam width and looks for a minimum in the laserline width curve 
 */

#ifndef LINEWIDTHMINIMUM_H_
#define LINEWIDTHMINIMUM_H_

#include <fliplib/Fliplib.h>			///< Fliplib image processing pipes and filter framework
#include <fliplib/TransformFilter.h>	///< Basisclass
#include <fliplib/PipeEventArgs.h>		///< Eventprocessing
#include <fliplib/SynchronePipe.h>		///< Inputs and outputs

#include <common/frame.h>				///< ImageFrame
#include <image/image.h>				///< BImage
#include <geo/geo.h>					///< Size2d, Intarray
#include <geo/array.h>					///< ByteArray

namespace precitec {
namespace filter {


class FILTER_API LineWidthMinimum  : public fliplib::TransformFilter
{

public:

	/// CTor.
	LineWidthMinimum();
	/// DTor.
	virtual ~LineWidthMinimum();

	static const std::string m_oFilterName;		///< Name Filter
	static const std::string PIPENAME_SEAMPOS_OUT;		///< Name Out-Pipe
	static const std::string PIPENAME_SEAMLEFT_OUT;		///< Name Out-Pipe
	static const std::string PIPENAME_SEAMRIGHT_OUT;		///< Name Out-Pipe
	static const std::string PIPENAME_SEAMFINDINGS_OUT;		///< Name Out-Pipe

	/// Set filter parameters as defined in database / xml file.
	void setParameter();
	/// paints overerlay primitives
	void paint();

	/**
	 * @brief 
	 *
	 * @param p_rImageIn       Input image.
	 * @param p_rLaserLineIn   LaserLine input object.
	 * @param p_oLineHeight    Height of the laser line object.
	 * @param p_rProfileOut    Profile output object.
	 * @param p_oProfileHeight Height of the output profile (for each of the upper and lower band).
	 */
    void calcLineWidthMinimum(
        const geo2d::VecDoublearray &p_rLaserLineIn,
        const geo2d::Doublearray    &p_rSeamWidth,
        geo2d::Doublearray          &p_rSeamPosOut,
        geo2d::Doublearray          &p_rSeamLeftOut,
        geo2d::Doublearray          &p_rSeamRightOut,
        geo2d::SeamFindingarray     seamFindingArrayOut,
        geo2d::VecDoublearray       &p_oLaserlineWidth );
    void calcLineWidthDoubleMinimum(
        const geo2d::VecDoublearray &p_rLaserLineIn,
        const geo2d::Doublearray    &p_rSeamWidth,
        geo2d::Doublearray          &p_rSeamPosOut,
        geo2d::Doublearray          &p_rSeamLeftOut,
        geo2d::Doublearray          &p_rSeamRightOut,
        geo2d::SeamFindingarray     seamFindingArrayOut,
        geo2d::VecDoublearray       &p_oLaserlineWidth );

protected:

    /// In-pipe registration.
    bool subscribe(fliplib::BasePipe& pipe, int group);
    /// In-pipe event processing.
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);

private:

    const fliplib::SynchronePipe< interface::GeoVecDoublearray >   * m_pPipeInLaserLine;    ///< In pipe
    const fliplib::SynchronePipe< interface::GeoDoublearray >      * m_pPipeInSeamWidth;    ///< In pipe
    const fliplib::SynchronePipe< interface::GeoSeamFindingarray > * m_pPipeInSeamFindings; ///< In pipe

    fliplib::SynchronePipe< interface::GeoDoublearray >       * m_pPipeOutSeamPos;      ///< Out pipe
    fliplib::SynchronePipe< interface::GeoDoublearray >       * m_pPipeOutSeamLeft;     ///< Out pipe
    fliplib::SynchronePipe< interface::GeoDoublearray >       * m_pPipeOutSeamRight;    ///< Out pipe
    fliplib::SynchronePipe< interface::GeoSeamFindingarray >  * m_pPipeOutSeamFindings; ///< Out pipe

    interface::SmpTrafo                                         m_oSpTrafo;             ///< roi translation
    int                                                         m_oMode;                ///< Diff. ways of 'min' search
    int                                                         m_oFilterLength;        ///< Filter for checking of laserline width/intensity
    int                                                         m_oMinYDistance;        ///< Min distance between min and max filter sum

    //variables needed for paint
    int                    m_resultSeamLeft;
    int                    m_resultSeamRight;
    int                    m_resultSeamPos;
    bool                   m_IsMinOk;
    int                    m_minX_1;
    int                    m_minSum_1;
    int                    m_minX_2;
    int                    m_minSum_2;
    geo2d::VecDoublearray  m_oLaserlineWidth;      // Stores the calculated width of the laserline

};

} // namespace precitec
} // namespace filter

#endif /* LINEWIDTHMINIMUM_H_ */

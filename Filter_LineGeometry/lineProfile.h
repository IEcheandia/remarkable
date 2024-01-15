/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Stefan Birmanns (SB)
 * 	@date		2011
 * 	@brief 		This filter extracts a grey level profile around a laser-line.
 */

#ifndef LINEPROFILE_H_
#define LINEPROFILE_H_

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

/**
 * @ingroup Filter_LineGeometry
 * @brief This filter extracts a grey level profile around a laser-line.
 *
 * @details The filter will ignore the actual laser-line itself. The size / height of the laser-line can be configured using the line height parameter. The area over
 * which the profile is computed can be specified using the profile height parameter. The profile height parameter specifies the height of one half of the total area, as there are two sub-areas, one above and one below the
 * laser line. The height of the laser line parameter also reflects only half of the total height of the line:
 * @code
 *                -     -   X
 *                ^     |   X
 * ProfileH ----> |     |   X
 *                v     |   X
 *                -  -  -   X
 *                   ^  |
 * LineH    ------>  |  |
 *                   v  |
 *                   -  O
 *                      |
 *                      |
 *                      |
 *                      -   X
 *                      |   X
 *                      |   X
 *                      |   X
 *                      -   X
 * @endcode
 * The profile is computed over the two areas marked with the X.
 */
class FILTER_API LineProfile  : public fliplib::TransformFilter
{

public:

	/// CTor.
	LineProfile();
	/// DTor.
	virtual ~LineProfile();

	static const std::string m_oFilterName;		///< Name Filter
	static const std::string PIPENAME_OUT1;		///< Name Out-Pipe

	/// Set filter parameters as defined in database / xml file.
	void setParameter();
	/// paints overerlay primitives
	void paint();

	/**
	 * @brief This function extracts a grey level profile around a laser line.
	 *
	 * @param p_rImageIn       Input image.
	 * @param p_rLaserLineIn   LaserLine input object.
	 * @param p_oLineHeight    Height of the laser line object.
	 * @param p_rProfileOut    Profile output object.
	 * @param p_oProfileHeight Height of the output profile (for each of the upper and lower band).
	 */
	static void extractLineProfile( const image::BImage &p_rImageIn, const geo2d::VecDoublearray &p_rLaserLineIn, int p_oLineHeight, geo2d::VecDoublearray &p_rProfileOut, int p_oProfileHeight );

protected:

	/// In-pipe registration.
	bool subscribe(fliplib::BasePipe& pipe, int group);
	/// In-pipe event processing.
	void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);

private:

	const fliplib::SynchronePipe< interface::ImageFrame >*   	m_pPipeInImageFrame;	///< In pipe
	const fliplib::SynchronePipe< interface::GeoVecDoublearray >* 	m_pPipeInLaserLine;		///< In pipe
	fliplib::SynchronePipe< interface::GeoVecDoublearray >*       	m_pPipeOutProfile;		///< Out pipe

	interface::SmpTrafo											m_oSpTrafo;				///< roi translation
	int 														m_oLineHeight;			///< Height of the laser line
	int															m_oProfileHeight;		///< Height of the area of the profile
	geo2d::VecDoublearray											m_oProfileOut;			///< Output profile
};

} // namespace precitec
} // namespace filter

#endif /* LINEPROFILE_H_ */

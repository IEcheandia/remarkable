/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		Stefan Birmanns (SB), HS
 *  @date		2011
 *  @brief		Computes maximum / minimum of a laserline.
 */

#ifndef LINEEXTREMUM_H_
#define LINEEXTREMUM_H_


#include <fliplib/Fliplib.h>			///< Fliplib image processing pipes and filter framework
#include <fliplib/TransformFilter.h>	///< Basisclass
#include <fliplib/PipeEventArgs.h>		///< Eventprocessing
#include <fliplib/SynchronePipe.h>		///< Inputs and outputs

#include <image/image.h>				///< BImage
#include "filter/parameterEnums.h"		///< enum ExtremumType

#include <common/frame.h>				///< ImageFrame
#include <geo/geo.h>					///< Size2d, VecDoublearray
#include <geo/array.h>					///< ByteArray


namespace precitec {
namespace filter {

/**
 * @ingroup Filter_LineGeometry
 * @brief This filter finds the maximum or minimum of a laser line and returns it as a point output pipe.
 */
class FILTER_API LineExtremum  : public fliplib::TransformFilter
{
public:

	/// CTor.
	LineExtremum();

	static const std::string m_oFilterName;		///< Name Filter
	static const std::string PIPENAME_OUT1;		///< Name Out-Pipe 1
	static const std::string PIPENAME_OUT2;		///< Name Out-Pipe 2

	/// Set filter parameters defined in database / xml file
	void setParameter();

	/// paints overlay
	void paint();

	void arm(const fliplib::ArmStateBase& state);	///< arm filter

	/**
	 * @brief Calculates / find an extremum of a line.
	 *
	 * @param p_rLineIn    		(Laser)-Line input object.
	 * @param p_oExtremumType	Shall the function search for the maximum (p_oExtremumType = eMaximum) or minimum (p_oExtremumType = eMinimum).
	 *
	 * @param p_oPosition		Index of extremum found
	 * @return void				
	 */
	void findExtremum( const geo2d::VecDoublearray &p_rLineIn, ExtremumType p_oExtremumType, geo2d::Pointarray &p_rPosition);


protected:

	/// in pipe registration
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	/// in pipe event processing
	void proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rEventArg);

private:

	const fliplib::SynchronePipe< interface::GeoVecDoublearray >* 	m_pPipeLineIn;			///< In pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >       		m_oPipePositionXOut;	///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >       		m_oPipePositionYOut;	///< Out pipe
	
	// parameters
	ExtremumType													m_oExtremumType;		///< Searching for the maximum or minimum?
	SearchDirType													m_oDirection;			///< left to right, or right to left

	// internal variables
	interface::SmpTrafo												m_oSpTrafo;				///< roi translation	
	geo2d::Pointarray								  				m_oOut;					///< Output point.
};

} // namespace precitec
} // namespace filter

#endif /* LINEEXTREMUM_H_ */

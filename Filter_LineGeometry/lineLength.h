/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		JS
 *  @date		2014
 *  @brief		Computes Length of a valid line
 */

#ifndef LINELENGTH_H_
#define LINELENGTH_H_


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
class FILTER_API LineLength  : public fliplib::TransformFilter
{
public:

	/// CTor.
	LineLength();

	static const std::string m_oFilterName;		///< Name Filter
	static const std::string PIPENAME_OUT1;		///< Name Out-Pipe 1

	/// Set filter parameters defined in database / xml file
	void setParameter();

	/// paints overlay
	void paint();

	/**
	 * @brief Calculates length of a line.
	 *
	 * @param p_rLineIn    		(Laser)-Line input object.
	 * @param p_oDirection      Count direction from first valid to first invalid point(rank==0) in the line
	 * @param p_rLengthOut		Length of valid line
	 * @return void				
	 */
    void calcConnectedLengthPercentage (const geo2d::VecDoublearray &p_rLineIn);


protected:

	/// in pipe registration
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	/// in pipe event processing
	void proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rEventArg);

private:


    const fliplib::SynchronePipe< interface::GeoVecDoublearray >* 	m_pPipeLineIn;

    fliplib::SynchronePipe<interface::GeoDoublearray> m_pipeConnectedLengthPercentageOut;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_pipeNumberOfPointsOut;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_pipeXStartOut;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_pipeXEndOut;

	// parameters
	SearchDirType													m_oDirection;			///< left to right, or right to left

	// internal variables
    interface::SmpTrafo m_oSpTrafo;
    geo2d::Doublearray m_connectedLengthPercentage;
    geo2d::Doublearray m_numberOfPoints;
    geo2d::Doublearray m_xStart;
    geo2d::Doublearray m_xEnd;

    geo2d::Point m_paintStart;
    geo2d::Point m_paintEnd;
};

} // namespace precitec
} // namespace filter

#endif /* LINEEXTREMUM_H_ */

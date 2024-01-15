/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Stefan Birmanns (SB)
 * 	@date		2015
 * 	@brief 		This filter detects a step in the laser-line.
 */

#ifndef LINESTEP_H_
#define LINESTEP_H_

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

class FILTER_API LineStep  : public fliplib::TransformFilter
{
	enum { eStepX, eStepY, eStepRank, eStepHeight };
	typedef std::tuple< double, double, int, double > Step;
	enum { eStart, eEnd, eLength, eSlope };
	typedef std::tuple< geo2d::Point, geo2d::Point, double, double > Segment;

public:

	/// CTor.
	LineStep();
	/// DTor.
	virtual ~LineStep();

	static const std::string m_oFilterName;		///< Name Filter
	static const std::string PIPENAME_OUT1;		///< Name Out-Pipe
	static const std::string PIPENAME_OUT2;		///< Name Out-Pipe

	/// Set filter parameters as defined in database / xml file.
	void setParameter();
	/// paints overerlay primitives
	void paint();

protected:

	/// In-pipe registration.
	bool subscribe(fliplib::BasePipe& pipe, int group);
	/// In-pipe event processing.
	void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);

private:

	void findSteps( const geo2d::VecDoublearray &p_rLineIn, const geo2d::VecDoublearray &p_rGradientIn, geo2d::Pointarray &p_rPositionOut );
	Step findStep( const geo2d::Doublearray &p_rLineIn, const geo2d::Doublearray &p_rGradientIn );

private:

	const fliplib::SynchronePipe< interface::GeoVecDoublearray > * 	m_pPipeInLaserLine;		///< In pipe
	const fliplib::SynchronePipe< interface::GeoVecDoublearray > * 	m_pPipeInLaserGradient;	///< In pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >       		m_oPipePositionXOut;	///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >       		m_oPipePositionYOut;	///< Out pipe

	// parameters
	double															m_oMinLengthLeft;
	double															m_oMinLengthRight;
	double															m_oMinHeight;
	double															m_oMaxHeight;
	unsigned int													m_oMaxDistance;
	double															m_oGradientThreshold;
	double															m_oMaxAngle;

	// internal variables
	interface::SmpTrafo												m_oSpTrafo;				///< roi translation
	std::unique_ptr<math::ImageCoordsTo3DCoordsTransformer>					m_pCoordTransformer;
	geo2d::Pointarray								  				m_oOut;					///< Output points.
	std::vector<Segment>											m_oSegments;			///< Segments, stored to be rendered in the paint routine.
	int																m_oSelectedLeft;		///< The selected left segment (right is +1).
	Step															m_oStep;				///< The position of the step.
	filter::LaserLine m_oTypeOfLaserLine;
};

} // namespace precitec
} // namespace filter

#endif /* LINESTEP_H_ */

/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Stefan Birmanns (SB)
 * 	@date		2015
 * 	@brief 		This filter detects a step in the laser-line.
 */

#ifndef STEP_H_
#define STEP_H_

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

class FILTER_API Step : public fliplib::TransformFilter
{
	enum { eStepX, eStepY, eStepRank, eStepHeight, eStepScore, eStepValid };
	typedef std::tuple< double, double, int, double, double, bool > StepCandidate;

public:

	/// CTor.
	Step();
	/// DTor.
	virtual ~Step();

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

	/// find steps in all laser-lines
	void findSteps( const geo2d::VecDoublearray &p_rLineIn, geo2d::Pointarray &p_rPositionOut );
	/// find step in a single laser-line
	StepCandidate findStep( const geo2d::Doublearray &p_rLineIn );

	/// compute height of a single step candidate
	void calcHeight( const geo2d::Doublearray &p_rLineIn, StepCandidate& p_rStep );
	/// compute a line fit
	void lineFit(const geo2d::Doublearray &p_rLineIn, unsigned int p_oMinIndex, unsigned int p_oMaxIndex, double& p_rMOut, double& p_rTOut);

private:

	const fliplib::SynchronePipe< interface::GeoVecDoublearray > * 	m_pPipeInLaserLine;		///< In pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >       		m_oPipePositionXOut;	///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >       		m_oPipePositionYOut;	///< Out pipe

	// gui parameters
	bool															m_oLeft;
	double															m_oHeight;
	double															m_oMinHeight;
	double															m_oMaxHeight;
	double															m_oLengthLeft;
	double															m_oLengthRight;
	double															m_oAngle;

	// internal variables
	interface::SmpTrafo												m_oSpTrafo;				///< roi translation
	std::unique_ptr<math::ImageCoordsTo3DCoordsTransformer> m_pCoordTransformer;	///< converter between image coordinates and 3d coordinates 
	geo2d::Pointarray												m_oOut;					///< Output points.
	StepCandidate													m_oStep;				///< The position of the step.
	std::vector<double>												m_oScores;				///< The similarity with a step at each position of the laser-line.
	std::vector<double>												m_oAvg;					///< Center position of the feature on the ll.
	std::vector<StepCandidate>										m_oCandidates;			///< The actual candidate positions.
	double															m_oScale;				///< Scaling of the score-function for the paint routine.

	std::vector<double>												m_oFeature;				///< Shape of the feature.
	unsigned int													m_oFeatureLeft;			///< Right boundary of the left segment of the feature.
	unsigned int													m_oFeatureCenter;		///< Center-index of the feature.
	unsigned int													m_oFeatureRight;		///< Left boundary of the right segment of the feature.
	filter::LaserLine m_oTypeOfLaserLine;
};

} // namespace precitec
} // namespace filter

#endif /* STEP_H_ */

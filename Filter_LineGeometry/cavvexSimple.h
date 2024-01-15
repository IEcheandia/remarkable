/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		CB
 * 	@date		2020
 * 	@brief 		This filter calculates the values for convexity, concavity and height differencev with out LineFit
 */

#ifndef CAVVEXSIMPLE_H_
#define CAVVEXSIMPLE_H_

#include <fliplib/Fliplib.h>			///< Fliplib image processing pipes and filter framework
#include <fliplib/TransformFilter.h>	///< Basisclass
#include <fliplib/PipeEventArgs.h>		///< Eventprocessing
#include <fliplib/SynchronePipe.h>		///< Inputs and outputs

#include <common/frame.h>				///< ImageFrame
#include <image/image.h>				///< BImage
#include <geo/geo.h>					///< Size2d, Intarray
#include <geo/array.h>					///< ByteArray

#include "LineFitter.h"
#include "math/3D/projectiveMathStructures.h"

namespace precitec {
namespace filter {


class FILTER_API CavvexSimple  : public fliplib::TransformFilter
{

public:

	/// CTor.
	CavvexSimple();
	/// DTor.
	virtual ~CavvexSimple();

	static const std::string m_oFilterName;		///< Name Filter
	static const std::string PIPENAME_CONVEX_OUT;		///< Name Out-Pipe
	static const std::string PIPENAME_CONCAVE_OUT;		///< Name Out-Pipe
	static const std::string PIPENAME_HEIGHTDIFF_OUT;		///< Name Out-Pipe
	static const std::string PIPENAME_CONVEXPOSX_OUT;		///< Name Out-Pipe
	static const std::string PIPENAME_CONCAVEPOSX_OUT;		///< Name Out-Pipe
	static const std::string m_oParamTypeOfLaserLine;		///< Parameter: Taype of LaserLine (e.g. FrontLaserLine, BehindLaserLine)

	/// Set filter parameters as defined in database / xml file.
	void setParameter();
	/// paints overerlay primitives
	void paint();

	virtual void arm(const fliplib::ArmStateBase& state);	///< arm filter

protected:

	/// In-pipe registration.
	bool subscribe(fliplib::BasePipe& pipe, int group);
	/// In-pipe event processing.
	void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);

private:

	const fliplib::SynchronePipe< interface::GeoVecDoublearray > * m_pPipeInLaserLine;		///< In pipe
	const fliplib::SynchronePipe< interface::GeoDoublearray >    * m_pPipeInSeamLeft;		///< In pipe
	const fliplib::SynchronePipe< interface::GeoDoublearray >    * m_pPipeInSeamRight;		///< In pipe
	const fliplib::SynchronePipe< interface::GeoDoublearray >    * m_pPipeInAngle;			///< In pipe
	const fliplib::SynchronePipe< interface::GeoLineModelarray > * m_pPipeInLineLeft;		///< In pipe
	const fliplib::SynchronePipe< interface::GeoLineModelarray > * m_pPipeInLineRight;		///< In pipe
	const fliplib::SynchronePipe< interface::GeoDoublearray >    * m_pPipeInLeftLineSlope;			///< In pipe
	const fliplib::SynchronePipe< interface::GeoDoublearray >    * m_pPipeInLeftLineYIntercept;		///< In pipe
	const fliplib::SynchronePipe< interface::GeoDoublearray >    * m_pPipeInRightLineSlope;			///< In pipe
	const fliplib::SynchronePipe< interface::GeoDoublearray >    * m_pPipeInRightLineYIntercept;	///< In pipe

	fliplib::SynchronePipe< interface::GeoDoublearray >			* m_pPipeOutConvexity;		///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >			* m_pPipeOutConcavity;		///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >			* m_pPipeOutHeightDifference;	///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >			* m_pPipeOutConvexityPosX;		///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >			* m_pPipeOutConcavityPosX;		///< Out pipe

	interface::SmpTrafo											m_oSpTrafo;					///< roi translation
	geo2d::Doublearray											m_oConvexityOut;			///< Output Convexity
	geo2d::Doublearray											m_oConcavityOut;			///< Output Concavity
	geo2d::Doublearray											m_oConvexityOutPosX;		///< Output ConvexityPosX
	geo2d::Doublearray											m_oConcavityOutPosX;		///< Output ConcavityPosX
	geo2d::Doublearray											m_oHeightDiffOut;			///< Output HeightDiff

	interface::GeoDoublearray	m_rGeoDoubleArraySeamLeft;
	interface::GeoDoublearray	m_rGeoDoubleArraySeamRight;
	interface::GeoDoublearray	m_rGeoDoubleArrayAngle;

	int m_firstSeamLeft;
	int m_firstSeamRight;

	int m_overlayMin, m_overlayMax;

	int m_oMode;
	CalcType m_oCalcType;
	bool m_oInvertHeightDiff;

	filter::LaserLine m_oTypeOfLaserLine;							///< which laser line should be used?

	// for painting
	int m_paintNumber;
	double m_paintSlopeLeft, m_paintYInterceptLeft;
	double m_paintSlopeRight, m_paintYInterceptRight;
	int m_paintCavX, m_paintCavY;
	int m_paintVexX, m_paintVexY;
	int m_paintCavLineX, m_paintCavLineY;
	int m_paintVexLineX, m_paintVexLineY;
	int m_paintCavMiddleY, m_paintVexMiddleY;
	bool m_hasConvex, m_hasConcave;

	int m_paintHeightDiffX1, m_paintHeightDiffY1, m_paintHeightDiffX2, m_paintHeightDiffY2;
	double m_leftLineSlope, m_leftLineYIntercept, m_rightLineSlope, m_rightLineYIntercept;

	// Erweiterungen zu SEL100
	//bool m_useSEL100Calculation;
	//bool m_isCalculationLeftPossible;
	//bool m_isCalculationRightPossible;
	//bool m_leftCorrected;
	//bool m_rightCorrected;

	//void applyResultsFromOneSideToOther(const std::vector<double, std::allocator<double>> &p_rLaserLineIn_Data, const std::vector<int, std::allocator<int>> p_rLaserLineIn_Rank, double& p_rSlopeLeft, int& p_rSlopeRankLeft, double& p_rYInterceptionLeft, int& p_rYInterceptionRankLeft, double& p_rSlopeRight, int& p_rSlopeRankRight, double& p_rYInterceptionRight, int& p_rYInterceptionRankRight);
		
	//void calcOneLine(const std::vector<double, std::allocator<double>> &rLaserLineIn_Data, const std::vector<int, std::allocator<int>> rLaserLineIn_Rank,
	//int startX, int EndX, double & slopeOut, int & slopeOutRank, double & yInterceptionOut, int & yInterceptionOutRank, bool isLeftSide, int seamLeft, int seamRight);

	double calc2dDistance(double x1, double y1, double x2, double y2);

//	LineFitter _lineFitter;
	bool m_badInput;
};

} // namespace precitec
} // namespace filter

#endif /* CAVVEXSIMPLE_H_ */

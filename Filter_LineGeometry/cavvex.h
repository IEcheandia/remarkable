/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		2015
 * 	@brief 		This filter calculates the values for convexity, concavity and height difference
 */

#ifndef CAVVEX_H_
#define CAVVEX_H_

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

class Statistic1D
{
public:
	Statistic1D();
	~Statistic1D();
	void addValue(double value);
	void addValues(double value1, double value2);
	void delValue(double value);
	void reset();
	double getCurrentMean();

private:
	int _count;
	double _currentSum;
};


class FILTER_API Cavvex  : public fliplib::TransformFilter
{

public:

	/// CTor.
	Cavvex();
	/// DTor.
	virtual ~Cavvex();

	static const std::string m_oFilterName;		///< Name Filter
	static const std::string PIPENAME_CONVEX_OUT;		///< Name Out-Pipe
	static const std::string PIPENAME_CONCAVE_OUT;		///< Name Out-Pipe
	static const std::string PIPENAME_HEIGHTDIFF_OUT;		///< Name Out-Pipe
	static const std::string m_oParamTypeOfLaserLine;		///< Parameter: Taype of LaserLine (e.g. FrontLaserLine, BehindLaserLine)

	static const std::string m_oParamWhichFitName;		///< Parameter: Taype of LaserLine (e.g. FrontLaserLine, BehindLaserLine)

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

	fliplib::SynchronePipe< interface::GeoDoublearray >			* m_pPipeOutConvexity;		///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >			* m_pPipeOutConcavity;		///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >			* m_pPipeOutHeightDifference;	///< Out pipe

	interface::SmpTrafo											m_oSpTrafo;					///< roi translation
	geo2d::Doublearray											m_oConvexityOut;			///< Output Convexity
	geo2d::Doublearray											m_oConcavityOut;			///< Output Concavity
	geo2d::Doublearray											m_oHeightDiffOut;			///< Output HeightDiff

	interface::GeoDoublearray	m_rGeoDoubleArraySeamLeft;
	interface::GeoDoublearray	m_rGeoDoubleArraySeamRight;
	interface::GeoDoublearray	m_rGeoDoubleArrayAngle;

	int m_firstSeamLeft;
	int m_firstSeamRight;

	int m_overlayMin, m_overlayMax;

	int m_oMode;
	CalcType m_oCalcType;
	int m_oLeftLineRoiStart;
	int m_oLeftLineRoiEnd;
	int m_oRightLineRoiStart;
	int m_oRightLineRoiEnd;
	bool m_oInvertHeightDiff;

	enum eWhichFit{
		eBoth =0,
		eLeft =1,
		eRight=2
	};
	eWhichFit m_oWhichFit;         // 0 heisst beide, 1 ist links, 2 ist rechts.
	filter::LaserLine m_oTypeOfLaserLine;							///< which laser line should be used?

	// for painting
	int m_paintNumber;
	int m_paintStartXLeft, m_paintEndXLeft;
	int m_paintStartXRight, m_paintEndXRight;
	double m_paintSlopeLeft, m_paintYInterceptLeft;
	double m_paintSlopeRight, m_paintYInterceptRight;
	int m_paintCavX, m_paintCavY;
	int m_paintVexX, m_paintVexY;
	int m_paintCavLineX, m_paintCavLineY;
	int m_paintVexLineX, m_paintVexLineY;
	bool m_hasConvex, m_hasConcave;

	int m_paintHeightDiffX1, m_paintHeightDiffY1, m_paintHeightDiffX2, m_paintHeightDiffY2;

	// Erweiterungen zu SEL100
	bool m_useSEL100Calculation;
	bool m_isCalculationLeftPossible;
	bool m_isCalculationRightPossible;
	bool m_leftCorrected;
	bool m_rightCorrected;

	void applyResultsFromOneSideToOther(const std::vector<double> &p_rLaserLineIn_Data, const std::vector<int> & p_rLaserLineIn_Rank, double& p_rSlopeLeft, int& p_rSlopeRankLeft, double& p_rYInterceptionLeft, int& p_rYInterceptionRankLeft, double& p_rSlopeRight, int& p_rSlopeRankRight, double& p_rYInterceptionRight, int& p_rYInterceptionRankRight);
		
	void calcOneLine(const std::vector<double> &rLaserLineIn_Data, const std::vector<int> &rLaserLineIn_Rank,
	int startX, int EndX, double & slopeOut, int & slopeOutRank, double & yInterceptionOut, int & yInterceptionOutRank, bool isLeftSide, int seamLeft, int seamRight);

	double calc2dDistance(double x1, double y1, double x2, double y2);

	LineFitter _lineFitter;
	bool m_badInput;
};

} // namespace precitec
} // namespace filter

#endif /* CAVVEX_H_ */

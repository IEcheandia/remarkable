/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		2016
 * 	@brief 		This filter gets a minimum and maximum seam width and looks for a minimum in the 2 laserline width curves 
 */

#ifndef TWOLINESWIDTHMINIMUM_H_
#define TWOLINESWIDTHMINIMUM_H_

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

class StatisticCalculator
{
public:
	StatisticCalculator();
	void reset();
	void addValue(double value);
	double getMedian();
	double getMean();

private:
	std::vector<double> _data;
	void sortIt();
	void exchange(double & d1, double & d2);
};

struct SingleTwoLinesPoint
{
	SingleTwoLinesPoint();
	SingleTwoLinesPoint(double dataLine1, int rankLine1, double dataLine2, int rankLine2);

	double line1Data;
	int line1Rank;
	double line2Data;
	int line2Rank;
	double lineSum;
	int lineSumRank;

};

class TwoLinesContainer
{
public:
	TwoLinesContainer();
	void reset();
	void addSingleTwoLinesPoint(SingleTwoLinesPoint singleTwoLinesPoint);
	SingleTwoLinesPoint getSingleTwoLinesPoint(int pos);
	int getSize();
	int getMaxValue();
	void calcMinimum(int seamWidth, int & pos1, int & pos2, int & posSum, double & width1, double & width2, double & widthSum);
	double getAverageWidthWithoutArea(int left, int right);
	double getAverageInArea(int left, int right);

	void resetSums();
	int calcSum1(int first, int last);
	int calcSum2(int first, int last);
	int calcSumSum(int first, int last);

private:
	std::vector<SingleTwoLinesPoint> _container;
	int _maxVal;

	bool _hasSum1;
	int _curFirst1, _curLast1, _curSum1;
	bool _hasSum2;
	int _curFirst2, _curLast2, _curSum2;
	bool _hasSumSum;
	int _curFirstSum, _curLastSum, _curSumSum;



};

enum ResultType { SumLine=0, Roi1, Roi2};

struct SeamFindResult
{
	SeamFindResult();
	SeamFindResult(int leftPos, int rightPos, int seamWidth, double averageLineWidth, ResultType typeOfResult);

	int _leftPos;
	int _rightPos;
	int _seamWidth;
	double _averageLineWidth;
	ResultType _typeOfResult;
};

class SeamFindResultContainer
{
public:
	SeamFindResultContainer();

	void reset();
	void addResult(SeamFindResult result);
	SeamFindResult getSingleSeamFindResult(int pos);
	int getSize();

	double getAverageLeft();
	double getMedianLeft();
	double getAverageRight();
	double getMedianRight();
	double getAverageSeamWidth();
	double getAverageLineWidth();
	double getAverageLineWidthForSeamWidth(int minSeamWidth, int maxSeamWidth);

private:
	std::vector<SeamFindResult> _container;
	double _leftSum, _rightSum, _widthSum, _averageSum; 

};


class FILTER_API TwoLinesWidthMinimum  : public fliplib::TransformFilter
{

public:

	/// CTor.
	TwoLinesWidthMinimum();
	/// DTor.
	virtual ~TwoLinesWidthMinimum();

	static const std::string m_oFilterName;		///< Name Filter
	static const std::string PIPENAME_SEAMPOS_OUT;		///< Name Out-Pipe
	static const std::string PIPENAME_SEAMLEFT_OUT;		///< Name Out-Pipe
	static const std::string PIPENAME_SEAMRIGHT_OUT;		///< Name Out-Pipe

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
	void calcLineWidthMinimum( const geo2d::VecDoublearray &p_rLaserLineIn, const geo2d::Doublearray & p_rSeamWidth, 
		geo2d::Doublearray &p_rSeamPosOut, geo2d::Doublearray &p_rSeamLeftOut, geo2d::Doublearray &p_rSeamRightOut, int & drawPos, int & drawPosLeft, int & drawPosRight);

protected:

	/// In-pipe registration.
	bool subscribe(fliplib::BasePipe& pipe, int group);
	/// In-pipe event processing.
	void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);

	bool calculateSeamPos(const geo2d::VecDoublearray & firstLaserLineIn, const geo2d::VecDoublearray & secondLaserLineIn, 
						  const geo2d::Doublearray & minimumSeamWidth, const geo2d::Doublearray & maximumSeamWidth,
						  geo2d::Doublearray & seamPosOut, geo2d::Doublearray & seamLeftOut, geo2d::Doublearray & seamRightOut);

private:

	const fliplib::SynchronePipe< interface::GeoVecDoublearray > * m_pPipeInFirstLaserLine;	///< In pipe
	const fliplib::SynchronePipe< interface::GeoVecDoublearray > * m_pPipeInSecondLaserLine;///< In pipe
	const fliplib::SynchronePipe< interface::GeoDoublearray >    * m_pPipeInSeamWidthMin;	///< In pipe
	const fliplib::SynchronePipe< interface::GeoDoublearray >    * m_pPipeInSeamWidthMax;	///< In pipe

	fliplib::SynchronePipe< interface::GeoDoublearray >       * m_pPipeOutSeamPos;		///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >       * m_pPipeOutSeamLeft;		///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >       * m_pPipeOutSeamRight;	///< Out pipe

	interface::SmpTrafo											m_oSpFirstTrafo;				///< roi translation
	interface::SmpTrafo											m_oSpSecondTrafo;				///< roi translation
	int 														m_oMode;				///< 
	int 														m_oResolution;			///< Parameter for the Resolution, can speed up the process
	//geo2d::Doublearray										    m_oSeamPosOut;			///< Output profile
	//geo2d::Doublearray										    m_oSeamLeftOut;			///< Output profile
	//geo2d::Doublearray										    m_oSeamRightOut;			///< Output profile

	int m_resultSeamLeft1, m_resultSeamRight1, m_resultSeamPos1;
	int m_resultSeamLeft2, m_resultSeamRight2, m_resultSeamPos2;
	int m_resultSeamLeftTotal, m_resultSeamRightTotal, m_resultSeamPosTotal;

	bool _isTrafo1Crucial;
	int _overlappSize;
	TwoLinesContainer _twoLinesContainer;

	bool m_isPaintPossible;

	//std::vector<int> _pos1, _pos2, _posSum;

};



} // namespace precitec
} // namespace filter

#endif /* TWOLINESWIDTHMINIMUM_H_ */

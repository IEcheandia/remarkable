/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		2015
 * 	@brief 		This filter deletes part of the laser line.
 */

#ifndef IMPROVELINE_H_
#define IMPROVELINE_H_

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

class DrawPoint
{
public:
	DrawPoint();
	DrawPoint(int x, int y);
	int _x;
	int _y;
};

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


class FILTER_API ImproveLine  : public fliplib::TransformFilter
{

public:

	ImproveLine();
	virtual ~ImproveLine();

	static const std::string m_oFilterName;				///< Name Filter
	//static const std::string PIPENAME_IMAGE_OUT;		///< Name Out-Pipe
	static const std::string PIPENAME_LASERLINE_OUT;	///< Name Out-Pipe
	//static const std::string PIPENAME_DOUBLE1_OUT;		///< Name Out-Pipe
	//static const std::string PIPENAME_DOUBLE2_OUT;		///< Name Out-Pipe
	//static const std::string PIPENAME_DOUBLE3_OUT;		///< Name Out-Pipe

	/// Set filter parameters as defined in database / xml file.
	void setParameter();
	/// paints overerlay primitives
	void paint();

protected:

	/// In-pipe registration.
	bool subscribe(fliplib::BasePipe& pipe, int group);
	/// In-pipe event processing.
	void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);
	void snipLine(const precitec::image::BImage & rImageIn, const std::vector<double, std::allocator<double>> & rLaserLineIn_Data, const std::vector<int, std::allocator<int>> & rLaserLineIn_Rank, 
				std::vector<double, std::allocator<double>> & rLineOut_Data, std::vector<int, std::allocator<int>> & rLineOut_Rank);
	void smoothLine(int mode, const precitec::image::BImage & rImageIn, const std::vector<double, std::allocator<double>> & rLaserLineIn_Data, const std::vector<int, std::allocator<int>> & rLaserLineIn_Rank, 
				std::vector<double, std::allocator<double>> & rLineOut_Data, std::vector<int, std::allocator<int>> & rLineOut_Rank);


private:

	const fliplib::SynchronePipe< interface::ImageFrame >        * m_pPipeInImageFrame;		///< In pipe
	const fliplib::SynchronePipe< interface::GeoVecDoublearray > * m_pPipeInLaserLine;		///< In pipe
	const fliplib::SynchronePipe< interface::GeoDoublearray >    * m_pPipeInDouble1;		///< In pipe
	const fliplib::SynchronePipe< interface::GeoDoublearray >    * m_pPipeInDouble2;		///< In pipe
	const fliplib::SynchronePipe< interface::GeoDoublearray >    * m_pPipeInDouble3;		///< In pipe

	fliplib::SynchronePipe< interface::ImageFrame >				 * m_pPipeOutImageFrame;	///< Out pipe
	fliplib::SynchronePipe< interface::GeoVecDoublearray >       * m_pPipeOutLaserLine;		///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >			 * m_pPipeOutDouble1;		///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >			 * m_pPipeOutDouble2;		///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >			 * m_pPipeOutDouble3;		///< Out pipe

	interface::SmpTrafo											m_oSpTrafo;				///< roi translation

	int m_oMode;
	int m_oMeanRange;
	int m_oMinBright;
	//int m_oParameter2;
	//int m_oParameter3;
	//int m_oParameter4;
	//int m_oParameter5;
	//int m_oParameter6;
	//int m_oParameter7;

	geo2d::VecDoublearray m_oLaserLineOut;
	//geo2d::Doublearray m_oDouble1Out;		
	//geo2d::Doublearray m_oDouble2Out;		
	//geo2d::Doublearray m_oDouble3Out;		

	bool m_hasPainting;
	std::vector<DrawPoint> _drawPoints;

	double calcMean(const image::BImage &p_rImageIn, const std::vector<double, std::allocator<double>> data, const std::vector<int, std::allocator<int>> rank, int index, int range);


};

} // namespace precitec
} // namespace filter

#endif /* IMPROVELINE_H_ */

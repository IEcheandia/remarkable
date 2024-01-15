/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		2015
 * 	@brief 		This filter calculates a fitting line out of the laser line
 */

#ifndef LINEFIT_H_
#define LINEFIT_H_

#include <fliplib/Fliplib.h>			///< Fliplib image processing pipes and filter framework
#include <fliplib/TransformFilter.h>	///< Basisclass
#include <fliplib/PipeEventArgs.h>		///< Eventprocessing
#include <fliplib/SynchronePipe.h>		///< Inputs and outputs

#include <common/frame.h>				///< ImageFrame
#include <image/image.h>				///< BImage
#include <geo/geo.h>					///< Size2d, Intarray
#include <geo/array.h>					///< ByteArray

#include "LineFitter.h"

namespace precitec {
namespace filter {


class FILTER_API LineFit  : public fliplib::TransformFilter
{

public:

	/// CTor.
	LineFit();
	/// DTor.
	virtual ~LineFit();
    enum class ErrorDirection
    {
        fromLeft,
        fromRight
    };

	static const std::string m_oFilterName;		///< Name Filter
	static const std::string PIPENAME_SLOPE_OUT; ///< Name Out-Pipe
	static const std::string PIPENAME_YINTERSEPT_OUT; ///< Name Out-Pipe
    static const std::string PIPENAME_LINE_OUT; ///< Name Out-Pipe
    static const std::string PIPENAME_ERROR_OUT; ///< Name Out-Pipe

	/// Set filter parameters as defined in database / xml file.
	void setParameter();
	/// paints overerlay primitives
	void paint();

	void calcLine( const geo2d::VecDoublearray &p_rLaserLineIn, int p_oRoiStart, int RoiEnd, bool computeError,
		geo2d::Doublearray & slopeOut, geo2d::Doublearray & yInterceptionOut, geo2d::Doublearray & errorOut);
	static void calcOneLine(const geo2d::Doublearray &rLaserLineIn, bool horizontalMean, int startX, int endX, double & slopeOut, double & yInterceptionOut, int & outRank);
	static double calcOneLineFitError(const geo2d::Doublearray & rLaserLineIn, int startX, int endX, double slope, double yInterception);
    static void calcOneLineErrorProfiles(geo2d::Doublearray& absErrorProfile, geo2d::Doublearray& sumSquaredErrorProfile,
                                         const geo2d::Doublearray& laserLine, const math::LineEquation& line, ErrorDirection direction);

protected:

	/// In-pipe registration.
	bool subscribe(fliplib::BasePipe& pipe, int group);
	/// In-pipe event processing.
	void proceed( const void* p_pSender, fliplib::PipeEventArgs& e );

private:

	const fliplib::SynchronePipe< interface::GeoVecDoublearray > * m_pPipeInLaserLine;	///< In pipe

	fliplib::SynchronePipe< interface::GeoDoublearray >				m_oPipeOutSlope;			///< Data out-pipe.
	fliplib::SynchronePipe< interface::GeoDoublearray >				m_oPipeOutYIntercept;		///< Data out-pipe.
    fliplib::SynchronePipe<interface::GeoLineModelarray>			m_oPipeOutLineEquation;	///< Data out-pipe.
	fliplib::SynchronePipe< interface::GeoDoublearray >				m_oPipeOutError;		///< Data out-pipe.

	interface::SmpTrafo											m_oSpTrafo;				///< roi translation
	int 														m_oRoiStart;			///< ROI Start %
	int															m_oRoiEnd;				///< ROI end %
	bool														m_oHorizontalMean;		///< 0: LineFit free orientation, 1: LineFit horizontal (Mean) 

	geo2d::Doublearray										m_oSlopeOut;
	geo2d::Doublearray										m_oYInterceptOut;


	bool m_isValid;

    int m_paintLineSize, m_paintStartX, m_paintEndX;
	double m_paintSlope, m_paintYIntercept;

	// eigentliche LineFit-Berechnungs-Klassen
	//void reset();
 //   void addPoint(double x, double y);
 //   void delPoint(double x, double y);
 //   bool calcMB(double & m, double & b);
 //   double Sx, Sy, Sxx, Sxy; // Container-Variablen zur Berechnung der Geraden
 //   int iPointAnz; 
};

} // namespace precitec
} // namespace filter

#endif /* LINEFIT_H_ */

/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		2015
 * 	@brief 		
 */

#ifndef PARABELFIT_H_
#define PARABELFIT_H_

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


class FILTER_API ParabelFit  : public fliplib::TransformFilter
{

public:

	/// CTor.
	ParabelFit();
	/// DTor.
	virtual ~ParabelFit();

	static const std::string m_oFilterName;		///< Name Filter
	static const std::string PIPENAME_PARABEL_A_OUT;		///< Name Out-Pipe
	static const std::string PIPENAME_PARABEL_B_OUT;		///< Name Out-Pipe
	static const std::string PIPENAME_PARABEL_C_OUT;		///< Name Out-Pipe
	static const std::string PIPENAME_VALUE1_OUT;		///< Name Out-Pipe
	static const std::string PIPENAME_VALUE2_OUT;		///< Name Out-Pipe

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

	void calcParabel(const image::BImage &p_rImageIn, const geo2d::VecDoublearray &p_rLaserLineIn, geo2d::Doublearray seamLeft, geo2d::Doublearray seamRight,
		geo2d::Doublearray & outA, geo2d::Doublearray & outB, geo2d::Doublearray & outC, geo2d::Doublearray & value1, geo2d::Doublearray & value2);

protected:

	/// In-pipe registration.
	bool subscribe(fliplib::BasePipe& pipe, int group);
	/// In-pipe event processing.
	void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);

private:

	const fliplib::SynchronePipe< interface::ImageFrame >        * m_pPipeInImageFrame;				///< In pipe
	const fliplib::SynchronePipe< interface::GeoVecDoublearray > * m_pPipeInLaserLine;				///< In pipe
	const fliplib::SynchronePipe< interface::GeoDoublearray >    * m_pPipeInLeftSeam;				///< In pipe
	const fliplib::SynchronePipe< interface::GeoDoublearray >    * m_pPipeInRightSeam;				///< In pipe

	fliplib::SynchronePipe< interface::GeoDoublearray >       * m_pPipeOutParabelA;		///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >       * m_pPipeOutParabelB;		///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >       * m_pPipeOutParabelC;		///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >       * m_pPipeOutValue1;		///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >       * m_pPipeOutValue2;	///< Out pipe

	interface::SmpTrafo											m_oSpTrafo;				///< roi translation

	int m_oMode;

	geo2d::Doublearray m_oValue1Out;			
	geo2d::Doublearray m_oValue2Out;		
	geo2d::Doublearray m_oParabelAOut;			
	geo2d::Doublearray m_oParabelBOut;			
	geo2d::Doublearray m_oParabelCOut;		

	double m_paintA, m_paintB, m_paintC;
	int m_paintSeamPos;
	int m_paintCorrectedSeamLeft, m_paintCorrectedSeamRight;
	int m_paintSeamLeft, m_paintSeamRight;

	int m_resultSeamLeft, m_resultSeamRight, m_resultSeamPos;
	int m_correctedSeamLeft, m_correctedSeamRight;
	int m_seamStartX, m_seamEndX;
	int m_lastSeamLeft, m_lastSeamRight;

	//int m_leftSeam, m_rightSeam;

	//int m_numberOfDebugPoints;

	//void setModeVariables();

};

} // namespace precitec
} // namespace filter

#endif /* PARABELFIT_H_ */

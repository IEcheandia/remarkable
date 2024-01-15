/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		
 *  @date		2011
 *  @brief		Computes local maximum / minimum of a laserline.
 */

#ifndef LINEEXTREMUMNUMBER_H_
#define LINEEXTREMUMNUMBER_H_


#include <fliplib/Fliplib.h>			///< Fliplib image processing pipes and filter framework
#include <fliplib/TransformFilter.h>	///< Basisclass
#include <fliplib/PipeEventArgs.h>		///< Eventprocessing
#include <fliplib/SynchronePipe.h>		///< Inputs and outputs
#include <array>
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
class FILTER_API LineExtremumNumber  : public fliplib::TransformFilter
{
public:

	/// CTor.
	LineExtremumNumber();

	static const std::string m_oFilterName;		///< Name Filter
	static const std::string PIPENAME_OUT1;		///< Name Out-Pipe 1
	static const std::string PIPENAME_OUT2;		///< Name Out-Pipe 2
	static const std::string m_oParameterComputeAverageName;   ///< Name Compute Average

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
	 * @param p_oExtremumNumber	For which (number) local Extremum shall we search
	 * @param p_oExtremumThreshold	Threshold for candidate extremum points
	 * @param p_oExtremumDistance	Distance on x axis
	 * @param p_oExtremumDifference	Difference in value
	 * @param p_oPosition		Index of extremum found
	 * @return void				
	 */
	void findExtremumNumber( const geo2d::VecDoublearray &p_rLineIn, ExtremumType p_oExtremumType,
		int p_oExtremumNumber, double p_oExtremumThreshold, int p_oExtremumDistance, double p_oExtremumDifference, geo2d::DPointarray &p_rPosition);


	/**
		 * @brief Calculates / find an extremum of a line.
		 *
		 * @param p_rLineIn    			(Laser)-Line input object.
		 * @param p_oExtremumType		Shall the function search for the maximum (p_oExtremumType = eMaximum) or minimum (p_oExtremumType = eMinimum).
		 * @param p_oExtremumNumber		For which (number) local Extremum shall we search
		 * @param p_oExtremumThreshold	Threshold for local maximum
		 * @param p_oPosition			Index of extremum found
		 * @return point
		 */
	
		std::pair<double, int >searchLocalMax(const geo2d::Doublearray &rLineIn, ExtremumType p_oExtremumType, const int p_oExtremumNumber, 
			const double p_oExtremumThreshold, const int p_oExtremumDistance, const double p_oExtremumDifference, SearchDirType p_oDirection);


	//calculate crossing of y = threshold . Returns as soon p_oExtremumNumber th crossing is found
		std::pair<double, int> searchZeroCrossing(const geo2d::Doublearray &rLineIn, const int p_oExtremumNumber,
			const double p_oExtremumThreshold, const double p_oExtremumDifference,  SearchDirType p_oDirection);


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
	int				                                                m_oExtremumNumber;      ///< Extremum Number
	double                                                          m_oExtremumThreshold;   ///< Threshold
	int                                                             m_oExtremumDistance;    ///< Distance between the points
	double                                                          m_oExtremumDifference; ///< Difference between the points
	bool															m_oComputeAverage;		///< Mittelwert berechnen?

	// internal variables
	geo2d::VecDoublearray													m_oLinesIn;				/// Input Lines for paint
	interface::SmpTrafo												m_oSpTrafo;				///< roi translation	
	geo2d::DPointarray								  				m_oOut;					///< Output point. - testdouble

	geo2d::TPoint<double>	m_oGlobalPos;         // Position in aktuellem Bild - d.h. in aktuellen HW ROI Koordinaten   (m_oSpTrafo->dx(), m_oSpTrafo1->dy());
	geo2d::TPoint<double>	m_oSensorPos;		//	Position auf dem Sensor (kompletter Chip i.d.R. 1024x1024 pixel
	geo2d::TPoint<double>	m_oHWRoiPos;		//	Position auf dem Hw Roi

	std::vector<geo2d::DPoint>										m_oResPoints;              ///<intermediate results: local maxima testdouble

	int m_overlayMin, m_overlayMax;

	double m_iThreshold ; //actual threshod for the current image (adjusted with the signal average)
};

} // namespace precitec
} // namespace filter

#endif /* LINEEXTREMUMNUMBER_H_ */

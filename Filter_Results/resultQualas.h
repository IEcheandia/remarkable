/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		Feb, 2015
 * 	@brief		Filter which computes the relevant numbers for weldings bath analysis.
 */

#ifndef RESULTQUALAS_INCLUDED_H_
#define RESULTQUALAS_INCLUDED_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include "common/frame.h"
#include "geo/geo.h"
#include "geo/array.h"


namespace precitec {
namespace filter {

/**
 * @brief Filter which computes the contour of a pore.
 */
class FILTER_API ResultQualas : public fliplib::TransformFilter
{
public:

	/**
	 * @brief CTor.
	 */
	ResultQualas();

	/**
	 * @brief Set filter parameters.
	 */
	void setParameter();

	/**
	 * @brief Paint overlay output.
	 */
	void paint();

	static const std::string m_oFilterName;							///< Filter name.
	static const std::string m_oPipeOutPenetrationIndexName;		///< Pipe name: out-pipe.
	static const std::string m_oPipeOutSeamPositionName;			///< Pipe name: out-pipe.
	static const std::string m_oPipeOutSeamAsymmetryName;			///< Pipe name: out-pipe.
	static const std::string m_oPipeOutAdditionalMaterialXName;		///< Pipe name: out-pipe.
	static const std::string m_oPipeOutAdditionalMaterialYName;		///< Pipe name: out-pipe.

protected:

	/**
	 * @brief In-pipe registration.
	 */
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);

	/**
	 * @brief Processing routine.
	 */
	void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs&);

private:
	bool m_isValid;

	typedef fliplib::SynchronePipe<interface::GeoDoublearray>	scalar_pipe_t;
	typedef fliplib::SynchronePipe<interface::GeoBlobarray>		blob_pipe_t;

	const blob_pipe_t*			m_pPipeInBlobSmall;			///< in-pipe.
	const blob_pipe_t*			m_pPipeInBlobLarge;			///< in-pipe.
	const scalar_pipe_t*		m_pPipeInCircleMiddleX;		///< in-pipe.
	const scalar_pipe_t*		m_pPipeInCircleMiddleY;		///< in-pipe.
	const scalar_pipe_t*		m_pPipeInCircleRadius;		///< in-pipe.

	interface::GeoBlobarray		m_rGeoBlobsSmallIn;
	interface::GeoBlobarray		m_rGeoBlobsLargeIn;
	interface::GeoDoublearray	m_rGeoDoubleArrayInX;
	interface::GeoDoublearray	m_rGeoDoubleArrayInY;
	interface::GeoDoublearray	m_rGeoDoubleArrayInR;

	double calcWeldingBathArea();
	double calcWeldingBathLength();
	double calcWeldingBathCenterOfMassX();
	double calcWeldingBathCenterOfMassY();
	double calcKeyholeCenterOfMassX();
	double calcKeyholeCenterOfMassY();
	double calcSeamWidth();
	double calcSeamDepth();
	void calcAdditionalMaterialY();
	double calcSeamLayer();
	double calcSeamAsymmetry();		

	double m_weldingBathArea;
	double m_weldingBathLength;
	double m_weldingBathCenterOfMassX;
	double m_weldingBathCenterOfMassY;
	double m_keyholeCenterOfMassX;
	double m_keyholeCenterOfMassY;
	double m_seamWidth;
	double m_seamDepth;
	double m_additionalMaterialX;
	double m_additionalMaterialY;
	double m_additionalMaterialMinX, m_additionalMaterialMaxX, m_additionalMaterialMinY, m_additionalMaterialMaxY;
	double m_seamLayer;
	double m_seamAsymmetry;		

	fliplib::SynchronePipe< interface::GeoDoublearray >		m_oPipeOutPenetrationIndex;			///< Data out-pipe.
	fliplib::SynchronePipe< interface::GeoDoublearray >		m_oPipeOutSeamPosition;				///< Data out-pipe.
	fliplib::SynchronePipe< interface::GeoDoublearray >		m_oPipeOutSeamAsymmetry;			///< Data out-pipe.
	fliplib::SynchronePipe< interface::GeoDoublearray >		m_oPipeOutAdditionalMaterialX;		///< Data out-pipe.
	fliplib::SynchronePipe< interface::GeoDoublearray >		m_oPipeOutAdditionalMaterialY;		///< Data out-pipe.

	interface::SmpTrafo			m_oSpTrafo;				///< roi translation
}; // class ResultQualas

} // namespace filter
} // namespace precitec

#endif /* RESULTQUALAS_INCLUDED_H_ */




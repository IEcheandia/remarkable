/**
*  @copyright:		Precitec Vision GmbH & Co. KG
*  @author			OS / GUR
*  @date			01/2015
*  @file
*  @brief			Performs a circle fit on a contour array
*/

#ifndef CIRCLEFITCONTOUR_INCLUDED_H
#define CIRCLEFITCONTOUR_INCLUDED_H

// local includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"

#include "system/types.h"
#include "common/frame.h"
#include "filter/parameterEnums.h"
#include "image/image.h"
#include "overlay/overlayCanvas.h"   // overlay, Color


// std lib
#include <string>


namespace precitec {
	namespace filter {

/**
 * @brief The CircleFitContour class
 * Macht einen Kreisfit auf ein Konturpunkte-Array.
 * Diese Klasse setzt die Berechnung des optimalen Kreises analog zu
 * http://www.dtcenter.org/met/users/docs/write_ups/circle_fit.pdf
 * um.
*/
class /*FILTER_API*/ CircleFitContour  : public fliplib::TransformFilter {

public:

	static const std::string m_oFilterName;		///< Filter name.
	// Ausgabe: CenterPoint und Radius des Kreises.
	static const std::string m_oPipeOutNameX;			///< Out-pipe name.
	static const std::string m_oPipeOutNameY;			///< Out-pipe name.
	static const std::string m_oPipeOutNameR;			///< Out-pipe name.
	static const std::string m_oPipeOutNameIsCircle;	///< Out-pipe name.

	/**
	 * @brief CircleFitContour Constructor.
	 */
	CircleFitContour();

	/**
	 * @brief CircleFitContour Destructor.
	 */
	~CircleFitContour();

private:
	
	/**
	 * @brief Set filter parameters.
	 */
	void setParameter();

	/**
	 * @brief In-pipe registration.
	 * @param p_rPipe Reference to pipe that is getting connected to the filter.
	 * @param p_oGroup Group number (0 - proceed is called whenever a pipe has a data element, 1 - proceed is only called when all pipes have a data element).
	 */
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);

	void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg);

	/**
		@brief		Calulates a circle for a contour point array
		@param		p_contourPointList		Contour point array with left or right points from 'SimpleTracking'.
		@param		circleX					Internal X position of circle center point.
		@param		circleY					Internal Y position of circle center point.
		@param		circleR					Internal radius of circle.
		@param		valid					Calculation of circle is OK.
		@param		sliceHeight				Height of a slice in the grey ROI.
		@param		p_oIgnoreTop			Amount of slices to ignore (in percent of ROI height), from top of the given ROI (= in y direction).
		@param		p_oIgnoreBottom			Amount of slices to ignore (in percent of ROI height), from bottom of the given ROI (= in y direction).
		@param		p_oMinRadius			Min. size in pixel for the found radius to be a (correct) circle.
		@param		p_oMaxRadius			Max. size in pixel for the found radius to be a (correct) circle.
	*/
	void doCircleFitContour(
			interface::GeoDoublearray	p_contourPointList,
			int							*circleX,
			int							*circleY,
			int							*circleR,
			bool						*valid,
			int							sliceHeight,
			int							p_oIgnoreTop,
			int							p_oIgnoreBottom,
			int							p_oMinRadius,
			int							p_oMaxRadius);

	/**
	 * @brief Draws the center point and/or circle and/or contour points.
	 */
	void paintCircleContour(
			const interface::GeoDoublearray	p_contourPointList,
			int								*circleX,
			int								*circleY,
			int								*circleR );

	// Eingang:
	//  -  Konturpunkte-Array mit linken oder rechten Konturpunkten (nur X-Koordinate!)
	//  -  Anzahl Streifen zur Berechnung der Y-Koordinaten
	//  -  Bild mit ROI-Angaben
	const fliplib::SynchronePipe< interface::GeoDoublearray >	*m_pPipeInContourPointList;		///< Data in-pipe contour point list.
	const fliplib::SynchronePipe< interface::GeoDoublearray >	*m_pPipeInNSlices;	            ///< Data in-pipe number of slices.
	const fliplib::SynchronePipe< interface::GeoDoublearray >	*m_pPipeInImgSize;				///< Data in-pipe image with grey ROI size.

	// Ausgabe: CenterPoint und Radius vom "gefundenen" Kreis, Flag ob Kreis gefunden wurde
	fliplib::SynchronePipe< interface::GeoDoublearray >			m_oPipeOutCenterPointX;			///< Data out-pipe.
	fliplib::SynchronePipe< interface::GeoDoublearray >			m_oPipeOutCenterPointY;			///< Data out-pipe.
	fliplib::SynchronePipe< interface::GeoDoublearray >			m_oPipeOutRadius;				///< Data out-pipe.
	fliplib::SynchronePipe< interface::GeoDoublearray >			m_oPipeOutIsCircle;				///< Data out-pipe.

	double						m_oMinRadiusMM;			/// Min. size in mm for the found radius to be a (correct) circle.
	int							m_oMinRadiusPix;		/// Min. size in Pixel for the found radius to be a (correct) circle.
	double						m_oMaxRadiusMM;			/// Max. size in mm for the found radius to be a (correct) circle.
	int							m_oMaxRadiusPix;		/// Max. size in Pixel for the found radius to be a (correct) circle.
	int							m_oIgnoreTop;			/// Amount of slices to ignore (in percent of ROI height), from top of the given ROI (= in y direction).
	int							m_oIgnoreBottom;		/// Amount of slices to ignore (in percent of ROI height), from bottom of the given ROI (= in y direction).
	int							m_oNSlices;				/// Number of horizotal slices the image is divided into

	int							m_oRoiHeight;			/// Height of grey ROI
	int							m_oRoiWidth;			/// Width of grey ROI
	int							m_oRoiOffsetX;			/// x offset of grey ROI
	int							m_oRoiOffsetY;			/// y offset of grey ROI

	// Zwischenresultate des gefundenen Kontur-Kreises
	int m_resultX, m_resultY, m_resultR;
	bool m_isValid;

};

} // namespace filter
} // namespace precitec


#endif /*CIRCLEFITCONTOUR_INCLUDED_H*/




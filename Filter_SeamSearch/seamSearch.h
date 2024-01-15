/**
*  @file			
*  @copyright		Precitec Vision GmbH & Co. KG
*  @author			Simon Hilsenbeck (HS)
*  @date			2011
*  @brief			Component-wide definitions
*/


#ifndef SEAMSEARCH_20112706_H
#define SEAMSEARCH_20112706_H

//#include <utility>				///< tuple
#include "geo/geo.h"			///< GeoVecDoublearray, VecDoublearray


namespace precitec {
	namespace filter {

		/**
		  *  \brief	Calculates the rank for a seam position based on side conditions.
		  *  \param p_rSeamPosL				left seam position
		  *  \param p_rSeamPosR				right seam position
		  *  \param p_oProfileSize			profile length (usually equal to roi width)
		  *  \param p_op_oBoundaryLength	boundary lenght (image border to be excluded)
		  *  \return void
		  *  \sa SelectPeaks, Maximum, Gradient, LineMovingAverage
		*/
		void calcRank(
			geo2d::Doublearray			&p_rSeamPosL, 
			geo2d::Doublearray			&p_rSeamPosR, 
			int							p_oProfileSize, 
			int							p_oBoundaryLength
			);



		/**  \brief		Checks seamposition integrity. Inplace.
		  *  \details	Reduces rank for left and right seam / gap position if positions lie on image boundary,
		  *				if left position is greater than right position,
		  *				or if left and righ position are equal.
		  *  \param	p_rContourL			left contour points
		  *  \param	p_rContourR			right contour points
		  *  \param	p_oImgWidth			image width
		  *  \param	p_oDefaultSeamWidth	default seam width as reference
		  *  \return	void			
		  *  \sa overloads of this function
		*/
		void enforceIntegrity (
			geo2d::Doublearray	&p_rContourL,
			geo2d::Doublearray	&p_rContourR,
			int					p_oImgWidth,
			int					p_oDefaultSeamWidth = 100
		);


	} // namespace filter
} // namespace precitec


#endif // #ifndefSEAMSEARCH_20112706_H

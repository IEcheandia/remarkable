/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		HS
 * 	@date		2013
 * 	@brief		The class PoreStatistics provides momentums, eigenvalues and eigenvectors of a blob (pore) segment.
 */

#ifndef PORESTATISTICS_20130213_INCLUDED_H
#define PORESTATISTICS_20130213_INCLUDED_H

#include "image/image.h"
#include "geo/range.h"
#include "geo/rect.h"
#include "majorAxes.h"

namespace precitec {
namespace filter {

/**
 *	@brief	The class PoreStatistics provides momentums, eigenvalues and eigenvectors of a blob (pore) segment.
*/
class PoreStatistics
{
public:
	static const double	m_oEvBadValue;		// used if eigen value length zero obtained. Prevents later division by zero.

	void		reset();

	void		calcMomentsV1(	geo2d::Rect	p_oRect, const image::BImage& p_rBinaryImage);
	inline void	sumToMoments1(	int		p_oX, int		p_oY);
	void		sumToMoments2(	int		p_oX, int		p_oY);
	void		normMoments2();
	void		calcMomentsV2(	geo2d::Rect	p_oRect, const image::BImage& p_rBinaryImage);
	void		sumToMoments(	int p_oX,	int p_oY);
	void		normMoments();
	void		calcEigenValues();
	void		calcEigenVectors();
	void		calcCenterOfMass();
	const MajorAxes& getMajorAxes() const;

	// Momente
	double		m_oSum_;
	double		m_oSum_x;
	double		m_oSum_y;
	double		m_oSum_xx;
	double		m_oSum_xy;
	double		m_oSum_yy;

	// Eigenschaften aus den Momenten
	MajorAxes	m_oMajorAxes;
}; // PoreStatistics

} // namespace filter
} // namespace precitec

#endif // PORESTATISTICS_20130213_INCLUDED_H

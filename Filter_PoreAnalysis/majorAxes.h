/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		HS
 * 	@date		2013
 * 	@brief		The class MajorAxes represents two eigenvalues, two eigenvectors and their length.
 */

#ifndef MAJORAXES_20130213_INCLUDED_H
#define MAJORAXES_20130213_INCLUDED_H


#include "geo/range.h"
#include "geo/rect.h"
#include "geo/point.h"

namespace precitec {
namespace filter {


class MajorAxes {
public:
	MajorAxes();

	geo2d::Point			m_oCenterOfMass;			// --> Schwerpunktskoordinaten
	double					m_oEigenValue1;				// --> Eigenwert 1
	double					m_oEigenValue2;				// --> Eigenwert 2
	geo2d::TPoint<double>	m_oEigenVector1;			// --> Richtung Hauptachse 1
	geo2d::TPoint<double>	m_oEigenVector2;			// --> Richtung Hauptachse 2
	double					m_oEigenVector1Length;		// --> Laenge Hauptachse 1
	double					m_oEigenVector2Length;		// --> Laenge Hauptachse 2
};


} // namespace filter
} // namespace precitec

#endif // MAJORAXES_20130213_INCLUDED_H

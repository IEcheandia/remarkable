/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		HS
 * 	@date		2013
 * 	@brief		The class MajorAxes represents two eigenvalues, two eigenvectors and their length.
 */

#include "majorAxes.h"

namespace precitec {
namespace filter {

MajorAxes::MajorAxes()
	:
	m_oEigenVector1Length	( 1 ),	// dummy value 1 will lead to bad rank as done in PoreStatistics::calcEigenValues ()
	m_oEigenVector2Length	( 1 )	// dummy value 1 will lead to bad rank as done in PoreStatistics::calcEigenValues ()
{}		

	
} // namespace filter
} // namespace precitec

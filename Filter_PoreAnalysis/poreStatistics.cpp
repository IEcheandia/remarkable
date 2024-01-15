/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		HS
 * 	@date		2013
 * 	@brief		The class PoreStatistics provides momentums, eigenvalues and eigenvectors of a blob (pore) segment.
 */

#include "poreStatistics.h"

#include <sstream>
#include <cmath>

#include "geo/point.h"
#include "module/moduleLogger.h"


namespace precitec {
	using namespace image;
	using namespace geo2d;
namespace filter {
	/*static*/ const double	PoreStatistics::m_oEvBadValue	= 1.2345678;

	//----------------------------------------------------------------------------
	//------------------------- Klasse PoreStatistics ------------------------------------
	//----------------------------------------------------------------------------

	void PoreStatistics::reset()
	{
		m_oSum_							= 0.0;
		m_oSum_x						= 0.0;
		m_oSum_y						= 0.0;
		m_oSum_xx						= 0.0;
		m_oSum_xy						= 0.0;
		m_oSum_yy						= 0.0;
		m_oMajorAxes.m_oCenterOfMass	= Point(0,0);
	}

	//----------------------------------------------------
	//--------------- Version 1 (konservative Version) ---
	//----------------------------------------------------

	void PoreStatistics::calcMomentsV1(geo2d::Rect p_oRect, const BImage& p_rBinaryImage)
	{
		byte const *pLine ( nullptr );
		int oRow ( 0 ), oCol ( 0 );
		
		// Momente vom Grad 0 und 1 berechnen
		for (oRow = p_oRect.y().start(); oRow <= p_oRect.y().end(); oRow++)
		{
			pLine = p_rBinaryImage[oRow];
			for (oCol = p_oRect.x().start(); oCol <= p_oRect.x().end(); oCol++)
			{
				if (pLine[oCol]) // --> Objektpunkt im Binaerbild
				{
					sumToMoments1(oCol, oRow);
				}
			} // for oCol
		} // for oRow
		
		// Momente vom Grad 2 berechnen
		calcCenterOfMass();
		for (oRow = p_oRect.y().start(); oRow < p_oRect.y().end(); oRow++)
		{
			pLine = p_rBinaryImage[oRow];
			for (oCol = p_oRect.x().start(); oCol < p_oRect.x().end(); oCol++)
			{
				if (pLine[oCol]) // --> Objektpunkt im Binaerbild
				{
					sumToMoments2(oCol, oRow);
				}
			} // for oCol
		} // for oRow
		normMoments2();
	}

	void PoreStatistics::sumToMoments1 (int p_oX, int p_oY) 
	{
		m_oSum_     ++;
		m_oSum_x    += p_oX;
		m_oSum_y    += p_oY;
	}

	void PoreStatistics::sumToMoments2 (int p_oX, int p_oY) 
	{
		int xTrans = p_oX - m_oMajorAxes.m_oCenterOfMass.x;
		int yTrans = p_oY - m_oMajorAxes.m_oCenterOfMass.y;
		m_oSum_xx   += xTrans * xTrans;
		m_oSum_xy   += xTrans * yTrans;
		m_oSum_yy   += yTrans * yTrans;
	}

	void PoreStatistics::normMoments2()
	{
		if (m_oSum_ == 0) {
			//wmLog(eDebug, "Sum is zero. Cannot calculate moments.\n");
			return;
		} // if

		double norm = m_oSum_;
		m_oSum_xx   /=  norm;
		m_oSum_xy   /= -norm;
		m_oSum_yy   /=  norm;
	}

	//----------------------------------------------------
	//--------------- Version 2 (schnelle Version) -------
	//----------------------------------------------------

//	extern int iResGlobal;

	void PoreStatistics::calcMomentsV2(geo2d::Rect p_oRect, const BImage &p_rBinaryImage)
	{
		int oRow ( 0 ), oCol ( 0 );		
		int iAufloesung = ( (p_oRect.x().length() + p_oRect.y().length()) / 2) / 100 + 1;

		for (oRow = p_oRect.y().start(); oRow <= p_oRect.y().end(); oRow += iAufloesung)
		{
			//-LOW- pLine = p_rBinaryImage[oRow];
			const auto* pLine = p_rBinaryImage[oRow] + p_oRect.x().start();
			for (oCol = p_oRect.x().start(); oCol <= p_oRect.x().end(); oCol += iAufloesung)
			{
				//-LOW- if (pLine[oCol+...]) // --> Objektpunkt im Binaerbild
				if (*(pLine+=iAufloesung)) // --> Objektpunkt im Binaerbild
				{
					//cout << "-";
					sumToMoments(oCol, oRow);
				}
			} // for oCol
		} // for oRow

		//timer.restart();
		//cout << "calcMomentsV2 = " << timer << endLine;
		
		normMoments();
	}



	inline void PoreStatistics::sumToMoments (int p_oX, int p_oY) 
	{
		m_oSum_     ++;
		m_oSum_x    += p_oX;
		m_oSum_y    += p_oY;
		m_oSum_xx   += p_oX*p_oX;
		m_oSum_xy   += p_oX*p_oY;
		m_oSum_yy   += p_oY*p_oY;
	}

	void PoreStatistics::normMoments()
	{
		if (m_oSum_ == 0) {
			//wmLog(eDebug, "Sum is zero. Cannot calculate moments.\n");
			return;
		} // if
		double sumInvers = 1.0/m_oSum_;

		// Trafo nach Steiner
 		m_oSum_xx  =    (m_oSum_xx*sumInvers) - (m_oSum_x*sumInvers)*(m_oSum_x*sumInvers);
 		m_oSum_xy  = -( (m_oSum_xy*sumInvers) - (m_oSum_x*sumInvers)*(m_oSum_y*sumInvers) );
 		m_oSum_yy  =    (m_oSum_yy*sumInvers) - (m_oSum_y*sumInvers)*(m_oSum_y*sumInvers);
	}
	//----------------------------------------------------
	//--------------- Eigenwerte -------------------------
	//----------------------------------------------------

	// Matrix (mit e --> lambda):
	// 
	//   | m_oSum_xx - e    -m_oSum_xy     |
	//   |                               |   =  0
	//   | -m_oSum_xy        m_oSum_yy - e | 
	// 
	//  --> quadratische Gleichung in e (lambda)
	//  --> e1, e2
	void PoreStatistics::calcEigenValues ()
	{
		double b_halbe = -(m_oSum_xx + m_oSum_yy) * 0.5;
		double c  = m_oSum_xx * m_oSum_yy  -  m_oSum_xy * m_oSum_xy;
		double discriminante = sqrt(fabs(b_halbe * b_halbe - c));
		
		m_oMajorAxes.m_oEigenValue1 = - b_halbe + discriminante;
		m_oMajorAxes.m_oEigenValue2 = - b_halbe - discriminante;
		
		m_oMajorAxes.m_oEigenVector1Length = (std::sqrt(m_oMajorAxes.m_oEigenValue1) * 2.0) * 2.0;
		m_oMajorAxes.m_oEigenVector2Length = (std::sqrt(m_oMajorAxes.m_oEigenValue2) * 2.0) * 2.0;
		if (m_oMajorAxes.m_oEigenVector2Length == 0)  {
			//wmLog(eDebug, "Pore analysis: Eigenvalue e2 set from 0 to m_oEvBadValue.\n");
			m_oMajorAxes.m_oEigenVector2Length = m_oEvBadValue;
		} // if
		
	} // calcEigenValues

	//----------------------------------------------------
	//--------------- Eigenvektoren ----------------------
	//----------------------------------------------------

	//   1.Zeile der Matrix auf 0 setzen 
	// 
	//  (m_oSum_xx - e1) * x + (m_oSum_xy) * y = 0
	//  y = [(m_oSum_xx - e1) / m_oSum_xy] * x;
	//  mit x = m_oSum_xy folgt: y = m_oSum_xx - e1
	//  2.Eigenvektor ist orthogonal
		 
	void PoreStatistics::calcEigenVectors()
	{
		// Normierungsfaktor berechnen: ax + by = 0 --> Normierung: sqrt (a*a + b*b)
		double dSqr = (m_oSum_xx - m_oMajorAxes.m_oEigenValue1) * (m_oSum_xx - m_oMajorAxes.m_oEigenValue1) + (m_oSum_xy * m_oSum_xy);
		if (dSqr == 0) {
			//wmLog(eDebug, "Normalization factor is zero. Cannot calculate eigen vectors.\n");
			return;
		} // if
		double dInvNorm = 1.0 / sqrt(dSqr);
		
		m_oMajorAxes.m_oEigenVector1 = TPoint<double>(m_oSum_xy,  m_oSum_xx - m_oMajorAxes.m_oEigenValue1) * dInvNorm;
		m_oMajorAxes.m_oEigenVector2 = TPoint<double>(m_oMajorAxes.m_oEigenVector1.y, -m_oMajorAxes.m_oEigenVector1.x);
	} // calcEigenVectors

	//----------------------------------------------------
	//--------------- Schwerpunkt ------------------------
	//----------------------------------------------------

	void PoreStatistics::calcCenterOfMass ()
	{
		if (m_oSum_ == 0) {
			//wmLog(eDebug, "Sum is zero. Cannot calculate gravity center.\n");
			return;
		} // if

		m_oMajorAxes.m_oCenterOfMass.x = int (m_oSum_x / m_oSum_);
		m_oMajorAxes.m_oCenterOfMass.y = int (m_oSum_y / m_oSum_);
	}


	//----------------------------------------------------
	//--------------- MajorAxes ------------------------
	//----------------------------------------------------

	const MajorAxes& PoreStatistics::getMajorAxes() const
	{
		return m_oMajorAxes;
	}

} // namespace filter
} // namespace precitec

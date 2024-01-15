/*
 * projectiveMathStructures.cpp
 *
 *  Created on: Mar 5, 2013
 *      Author: abeschorner
 */

#include <iterator>
#include <limits>
#include <cmath>
#include <fstream>
#include <algorithm>

#include "math/3D/projectiveMathStructures.h"
#include "math/mathCommon.h"

namespace precitec {
namespace math {

Vec3D::Vec3D() : m_oEntries()
{
	m_oEntries[0] = 1.0; m_oEntries[1] = 1.0; m_oEntries[2] = 1.0;
}

Vec3D::Vec3D(const double p_oAt1, const double p_oAt2, const double p_oAt3)
{
	m_oEntries[0] = p_oAt1; m_oEntries[1] = p_oAt2; m_oEntries[2] = p_oAt3;
}

Vec3D::Vec3D(const Vec3D &p_rVec) : m_oEntries(p_rVec.m_oEntries)
{
}

bool Vec3D::normalizeByZ(Vec2DHomogeneous &p_rDest)
{
	p_rDest[0] = m_oEntries[0];
	p_rDest[1] = m_oEntries[1];
	p_rDest[2] = m_oEntries[2];
	if (m_oEntries[2] > math::eps)
	{
		p_rDest[0] /= m_oEntries[2];
		p_rDest[1] /= m_oEntries[2];
		p_rDest[2] = 1.0;

		return true;
	}
	return false;
}

double Vec3D::dotProduct(Vec3D const &p_rRight)
{
	return (m_oEntries[0]*p_rRight[0] + m_oEntries[1]*p_rRight[1] + m_oEntries[2]*p_rRight[2]);
}

Vec3D Vec3D::crossProduct(Vec3D const &p_rRight)
{
	Vec3D oRet;

	oRet[0] = (*this)[1]*p_rRight[2] - (*this)[2]*p_rRight[1];   // a2b3 - a3b2
	oRet[1] = (*this)[2]*p_rRight[0] - (*this)[0]*p_rRight[2];   // a3b1 - a1b3
	oRet[2] = (*this)[0]*p_rRight[1] - (*this)[1]*p_rRight[0];   // a1b2 - a2b1

	return oRet;
}

// p_rSeg must have been precomputed by precomputeSegmentData before calling this function
Vec3D Vec3D::projOntoSegment( LineSegment &p_rSeg, bool oRestrictToSegment=true )
{
	p_rSeg.setLastProjectionValid();

	if (std::abs(p_rSeg.m_oLen) < math::eps)
	{
		p_rSeg.invalidateLastProjection();
		return p_rSeg.m_oStart; // start and endpoint are equal
	}

	// take distance from point to one of the segment bounding points. we choose the startpoint, but could also use (p_rSegEnd - *this)
	Vec3D oDelta(*this - p_rSeg.m_oStart);
	double oDist = (p_rSeg.m_oSegment).dotProduct(oDelta)/p_rSeg.m_oLen;
	if (oRestrictToSegment)
	{
		if (oDist < 0)
		{
			p_rSeg.invalidateLastProjection();
			return p_rSeg.m_oStart;
		} else if (oDist > 1.0)
		{
			p_rSeg.invalidateLastProjection();
			return p_rSeg.m_oEnd;
		}
	}

	return (p_rSeg.m_oStart + oDist*p_rSeg.m_oSegment);
}

bool Vec3D::writeToFile(std::ofstream &p_rFile)
{
	//p_rFile.write((char*)& (m_oEntries[0]), sizeof(double)*m_oEntries.size());
	std::copy(m_oEntries.begin(), m_oEntries.end(), std::ostreambuf_iterator<char>(p_rFile)); // C4244 disabled - possible loss of data
	return p_rFile.good();
}

// --------------------

void LineSegment::preComputeSegmentData( Vec3D const &p_rSegStart, Vec3D const &p_rSegEnd )
{
	// direction of segment and its length squared (we do not need the square root)
	m_oStart = p_rSegStart; m_oEnd = p_rSegEnd;
	m_oSegment = p_rSegEnd - p_rSegStart;
	m_oLen = m_oSegment.norm2();
	m_oLastProjectionValid = false;
}

void LineSegment::setLastProjectionValid()
{
	m_oLastProjectionValid = true;
}

void LineSegment::invalidateLastProjection()
{
	m_oLastProjectionValid = false;
}

bool LineSegment::lastProjectionValid()
{
	return m_oLastProjectionValid;
}


// -------------------


Vec2DHomogeneous::Vec2DHomogeneous()
{
	m_oEntries[0] = 1.0; m_oEntries[1] = 1.0; m_oEntries[2] = 1.0;
}

Vec2DHomogeneous::Vec2DHomogeneous(double p_oAt1, double p_oAt2)
{
	m_oEntries[0] = p_oAt1; m_oEntries[1] = p_oAt2; m_oEntries[2] = 1.0;
}

Vec2DHomogeneous::Vec2DHomogeneous(const Vec2DHomogeneous &p_rVec)
{
	m_oEntries[0] = p_rVec[0]; m_oEntries[1] = p_rVec[1]; m_oEntries[2] = p_rVec[2];
}



} //namespaces
}

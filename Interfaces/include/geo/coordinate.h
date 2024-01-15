#ifndef _COORDINATE_H_
#define _COORDINATE_H_

#include <iomanip> //setprecision
#include <iostream> 
#include <array>
#include <string>
#include <cassert>
#include <cmath> //abs

#include "filter/parameterEnums.h" //for laser line
#include "math/3D/projectiveMathStructures.h"

namespace precitec
{
	namespace geo2d
	{
		enum eCoordDimension
		{
			eScreenX,
			eScreenY,
			eRealX, 
			eRealY
		};
		template <typename T>
		struct coord2DTemplate ///< Simple 2D screen coordinate struct
		{
			T ScreenX;
			T ScreenY;
			coord2DTemplate(const T x, const T y) :
				ScreenX(x), ScreenY(y)
			{}
			coord2DTemplate(const std::array<T, 2>  pArray) :
				ScreenX(pArray[0]), ScreenY(pArray[1])
			{}
			std::array<T, 2> toArray() const //work-around for tClusters
			{
			return std::array<T, 2>{{ScreenX, ScreenY}};
			}
			friend coord2DTemplate operator +(coord2DTemplate lhs, const coord2DTemplate & rhs)
			{
				lhs.ScreenX += rhs.ScreenX;
				lhs.ScreenY += rhs.ScreenY;
				return lhs; // return the result by value (uses move constructor)
			};
			friend coord2DTemplate operator -(coord2DTemplate lhs, const coord2DTemplate & rhs)
			{
				lhs.ScreenX -= rhs.ScreenX;
				lhs.ScreenY -= rhs.ScreenY;
				return lhs; // return the result by value (uses move constructor)
			};

			friend std::ostream& operator<<(std::ostream& out, const coord2DTemplate& pCoord)
			{
				const int precision = 3;
				out << std::setfill(' ') << "[" << std::setw(precision) << pCoord.ScreenX << ","
					<< std::setw(precision) << pCoord.ScreenY << "] ";
				return out;
			}

		};
			typedef coord2DTemplate<int> coord2D;
			typedef coord2DTemplate<double> coord2DScreen;

		struct coordScreenToPlaneDouble ///< internal member as doubles, for greater precision
		{
			double ScreenX; //0
			double ScreenY;
			double RealX; //2
			double RealY;
			coordScreenToPlaneDouble() :
				ScreenX(0), ScreenY(0), RealX(0), RealY(0)
			{}
			coordScreenToPlaneDouble(const double& xcoord, const double& ycoord, const double& xmm, const double& ymm) :
				ScreenX(xcoord), ScreenY(ycoord), RealX(xmm), RealY(ymm)
			{}
			coordScreenToPlaneDouble(const coordScreenToPlaneDouble & coord) :
				ScreenX(coord.ScreenX), ScreenY(coord.ScreenY), RealX(coord.RealX), RealY(coord.RealY)
			{}
			double get(eCoordDimension d) const
			{
				switch ( d )  //no break statements because I'm just returning
				{
					default:
					case eScreenX:
						return ScreenX;
					case eScreenY:
						return ScreenY;
					case eRealX:
						return RealX;
					case eRealY:
						return RealY;
				}
			}
			coord2DScreen getScreenCoords() const
			{
				return coord2DScreen(ScreenX, ScreenY);
			}
			void convertToRealWorldCoords(math::RealWorldTransform p_transform)
			{
				RealX = p_transform.m_oScale * (RealX + p_transform.m_oTx);
				RealY = p_transform.m_oScale * (RealY + p_transform.m_oTy);
                if (p_transform.m_invertX)
                {
                    RealX = -RealX;
                }
                if (p_transform.m_invertY)
                {
                    RealY = -RealY;
                }
			}
			friend std::ostream& operator<<(std::ostream& out, const coordScreenToPlaneDouble & pCoord)
			{
				const std::string indent = "";
				const int precision = 6;
				out << indent;
				out << std::setprecision(precision) << std::setfill(' ') << "[" << pCoord.ScreenX << ","
					<< pCoord.ScreenY << "] ";
				out << std::setprecision(precision) << " (" << pCoord.RealX << "," << pCoord.RealY << ") ";
				return out;
			}

			friend coordScreenToPlaneDouble operator +(coordScreenToPlaneDouble lhs, const coordScreenToPlaneDouble & rhs)
			{
				lhs.ScreenX += rhs.ScreenX;
				lhs.ScreenY += rhs.ScreenY;
				lhs.RealX += rhs.RealX;
				lhs.RealY += rhs.RealY;
				return lhs; // return the result by value (uses move constructor)
			}

			friend coordScreenToPlaneDouble operator - (coordScreenToPlaneDouble lhs, const coordScreenToPlaneDouble & rhs)
			{
				lhs.ScreenX -= rhs.ScreenX; //here of type double instead of uint, no particular precautions
				lhs.ScreenY -= rhs.ScreenY;
				lhs.RealX -= rhs.RealX;
				lhs.RealY -= rhs.RealY;
				return lhs; // return the result by value (uses move constructor)
			}

			//scaling
			friend coordScreenToPlaneDouble operator *(coordScreenToPlaneDouble lhs, const double & rhs)
			{
				lhs.ScreenX *= rhs;
				lhs.ScreenY *= rhs;
				lhs.RealX *= rhs;
				lhs.RealY *= rhs;
				return lhs; // return the result by value (uses move constructor)
			}
			friend bool operator==(const coordScreenToPlaneDouble& lhs, const coordScreenToPlaneDouble& rhs)
			{
				return std::abs(lhs.ScreenX - rhs.ScreenX) < 1e-8 &&
					std::abs(lhs.ScreenY - rhs.ScreenY) < 1e-8 &&
					std::abs(lhs.RealX - rhs.RealX) < 1e-8  &&
					std::abs(lhs.RealY - rhs.RealY) < 1e-8;
			}

			
		};


		
	

	}
}

#endif //_COORDINATE_H_

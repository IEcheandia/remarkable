#pragma once

#include "calibrationCornerGrid.h"
#include "calibration3DCoords.h"

#include <algorithm>

namespace precitec
{
namespace math
{
    
  
  class UniformGridMapBuilder
  {
  public:
    struct  t_map_center
    {
        double x; double y;
    };
    
    UniformGridMapBuilder(const std::vector<std::pair<CornerGridMap, t_map_center>> & maps, t_map_center center);

    
    const UniformGridMap & getAverageMap() const;
    
    
    //TODO: move to pImpl  / calibrationdata
    static UniformGridMap transformToUniformGridInScreenSpace (const CornerGridMap & p_rInputGridMap, int resolutionX, int resolutionY ,  unsigned int imageWidth, unsigned int imageHeight,
        double imageOriginX, double imageOriginY);  
    
    private:
    
    void computeWeights();
    
    
    void addWeightedPlaneCoordinates (int yAvg, const system::t_coordScreenToPlaneLine& rLine, double w_x, double w_y);
            
    const std::vector<std::pair<CornerGridMap, t_map_center>> & m_maps;
    t_map_center m_center;
    UniformGridMap m_averageMap;
    std::vector<double> m_weight_x;
    std::vector<double> m_weight_y;
    std::vector<double> m_dist2;
    

  };
  
  
class Calibration3DCoordsTransformer 
{
    public:
        Calibration3DCoordsTransformer(precitec::math::Calibration3DCoords & oCoords)
        : m_rCoords(oCoords)
        {}
        
        template<class UnaryOperationX, class UnaryOperationY>
        void applyTransformToInternalPlane(UnaryOperationX unary_op_x,UnaryOperationY unary_op_y)
        {
            std::for_each(m_rCoords.m_oCoordsArrayX.begin(), m_rCoords.m_oCoordsArrayX.end(), unary_op_x);
            std::for_each(m_rCoords.m_oCoordsArrayY.begin(), m_rCoords.m_oCoordsArrayY.end(), unary_op_y);
        }
        void applyTranslationToInternalPlane(double tx, double ty)
        {
            //internal array are still implemented as float
            float f_tx = tx;
            float f_ty = ty;
            
            applyTransformToInternalPlane([&] (float & x) { x += f_tx; }, [&] (float & y) { y += f_ty; } );                    
        }
        void applyScaleToInternalPlane(double sx, double sy)
        {
            //internal array are still implemented as float
            float f_sx = sx;
            float f_sy = sy;            
            applyTransformToInternalPlane([&] (float & x) { x *= f_sx; }, [&] (float & y) { y *= f_sy; } );  
        }
        void translateToOffsetOrigin(int x, int y)
        {
            float xOffset, yOffset;
            bool ok = m_rCoords.getCoordinates(xOffset, yOffset, x,y);
            assert(ok); (void)ok;

            applyTransformToInternalPlane([&] (float & x) { x -= xOffset; }, [&] (float & y) { y -= yOffset; } );                    
                
        }
        
    private:
        precitec::math::Calibration3DCoords & m_rCoords;
    
    
};

} //end namespace math
} //end namespace precitec
 


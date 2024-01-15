#include "UniformMapBuilder.h"
#include "util/camGridData.h"
#include "calibration3DCoordsInterpolator.h"
#include <numeric>




namespace precitec
{
namespace math
{
    using system::tGridMap2D3D; // as m_oMapGrid in CalibrationCornerGrid std::map<int, std::vector<geo2d::coordScreenToPlaneDouble>
    using geo2d::coordScreenToPlaneDouble;
    using math::sCellData;
    using math::CornerGridMap;
    
   UniformGridMapBuilder::UniformGridMapBuilder(const std::vector<std::pair<CornerGridMap, t_map_center>> & maps, t_map_center center)
    : m_maps(maps), m_center(center)
    {
        //TODO verify the provided maps enclose the new center
        computeWeights();
        
        int resolutionX = 20;
        int resolutionY = 20;
        
        bool initialized = false;
        for (unsigned int i = 0; i < m_maps.size(); i++)
        {
            const auto & rMap = m_maps[i].first;
            if ((!initialized))
            {
                m_averageMap.reset(rMap.getValidArea());
            }
            
            if (rMap.getValidArea() !=  m_averageMap.getValidArea())
            {
                // TODO notify error (input maps referring to different images)
                continue;
            }
            
            unsigned int imageWidth = m_averageMap.getValidArea().width;
            unsigned int imageHeight = m_averageMap.getValidArea().height;
            double imageOriginX = imageWidth/2;
            double imageOriginY = imageHeight/2;
            
            auto oUniformMap = transformToUniformGridInScreenSpace (rMap, resolutionX,  resolutionY, 
                imageWidth, imageHeight,imageOriginX, imageOriginY);
            
            if (!initialized)
            {
                assert(std::all_of(oUniformMap.m_oLineBoundaries.begin(), oUniformMap.m_oLineBoundaries.end(), 
                                   [](system::tLineBoundaries::value_type & rEntry)
                                   { auto yAvg = rEntry.first; auto minmax = rEntry.second;
                                      return minmax.first == yAvg &&  minmax.second == yAvg ;
                                }));
                
                for (auto & rMapEntry : oUniformMap.getGrid2D3D())
                {
                     auto yAvg = rMapEntry.first;
                     system::t_coordScreenToPlaneLine zeros(rMapEntry.second); 
                     std::for_each(zeros.begin(), zeros.end(), [](geo2d::coordScreenToPlaneDouble & coord){
                         coord.RealX = 0; coord.RealY = 0;
                     });
                    m_averageMap.addRow(yAvg, yAvg, yAvg, zeros );
                }
                initialized = true;
            }
            
            for (auto rMapEntry : oUniformMap.getGrid2D3D())
            {
                addWeightedPlaneCoordinates (rMapEntry.first, rMapEntry.second, m_weight_x[i], m_weight_y[i] );
            }
        } //end for m_maps
        
    } //end constructor

    


const UniformGridMap& UniformGridMapBuilder::getAverageMap() const
{
    return m_averageMap;
}


UniformGridMap UniformGridMapBuilder::transformToUniformGridInScreenSpace(const CornerGridMap& p_rInputGridMap, int resolutionX, int resolutionY, unsigned int imageWidth, unsigned int imageHeight, double imageOriginX, double imageOriginY)
{
    UniformGridMap oResult;

    assert(imageOriginX >= 0 && imageOriginX < imageWidth);
    assert(imageOriginY >= 0 && imageOriginY < imageHeight);

    //not optimized (the whole coordintates are being computed, then subsampled)
    Calibration3DCoords oTmp;
    Calibration3DCoordsInterpolator oInterpolator(oTmp);
    oInterpolator.resetGridCellData(imageWidth,imageHeight);
    bool ok = oInterpolator.allCellsTo3D(p_rInputGridMap, /*extrapolate*/ true, /*rectify*/ true);

    assert(ok);
    if (!ok)
    {
        return oResult;
    }

    float oOriginXmm, oOriginYmm;
    oTmp.getCoordinates(oOriginXmm, oOriginYmm, imageOriginX, imageOriginY);

    oResult.reset(p_rInputGridMap.getValidArea());
    assert( oResult.getTransform().m_oScale == 1.0); // we keep the coordinates computed by the 3dInterpolator

    for (unsigned int y = 0; y < imageHeight; y += resolutionY)
    {
        system::t_coordScreenToPlaneLine rRow ;
        rRow.reserve(imageWidth/resolutionX);
        for (unsigned int x = 0; x < imageWidth; x += resolutionX)
        {
            float Xmm,Ymm;
            oTmp.getCoordinates(Xmm, Ymm, x,y);

            geo2d::coordScreenToPlaneDouble transformedCoordinate;
            transformedCoordinate.ScreenX = x;
            transformedCoordinate.ScreenY = y;
            transformedCoordinate.RealX = Xmm - oOriginXmm;
            transformedCoordinate.RealY = Ymm - oOriginYmm;
            rRow.push_back(transformedCoordinate);
        }
        oResult.addRow(y,y,y, std::move(rRow));
    }
    return oResult;
}


void UniformGridMapBuilder::computeWeights()
{
    auto n = m_maps.size();
    assert(n>0);
    m_weight_x.reserve(n);
    m_weight_y.reserve(n);
    m_dist2.reserve(n);

    double sumX = 0.0;
    double sumY = 0.0;
    int num_exact_match_x = 0;
    int num_exact_match_y = 0;

    for (auto && rMap : m_maps)
    {
        //independent from direction?
        auto dx = std::abs(m_center.x - rMap.second.x);
        if (dx==0)
        {
            num_exact_match_x++;
        }
        auto dy = std::abs(m_center.y - rMap.second.y);

        if (dy==0)
        {
            num_exact_match_y++;
        }

        m_weight_x.push_back(dx);
        m_weight_y.push_back(dy);
        sumX += dx;
        sumY += dy;
        m_dist2.push_back(dx*dx + dy *dy);

    }

    if ( num_exact_match_x == 0)
    {
        std::for_each( m_weight_x.begin(), m_weight_x.end(), [&](double &v) {
            v = 1.0 - v /sumX;
        });
    }
    else
    {
        double avg = 1.0 / double(num_exact_match_x);
        std::for_each( m_weight_x.begin(), m_weight_x.end(), [&](double &v) {
            v = (v == 0) ? avg : 0.0;
        });
    }
    if (num_exact_match_y == 0)
    {
        std::for_each( m_weight_y.begin(), m_weight_y.end(), [&](double &v) {
            v = 1.0 - v /sumY;
        });
    }
    else
    {
        double avg = 1.0 / double(num_exact_match_y);
        std::for_each( m_weight_y.begin(), m_weight_y.end(), [&](double &v) {
            v = (v == 0) ? avg : 0.0;
        });
    }
    // just use the distance
    int numExactMatch = std::count(m_dist2.begin(), m_dist2.end(), 0.0);
    if ( numExactMatch > 0)
    {
        double weight = 1.0 / double(numExactMatch);
        for (unsigned int i = 0; i < m_dist2.size(); i++)
        {
            if (m_dist2[i] == 0)
            {
                m_weight_x[i] = weight;
            }
            else
            {
                m_weight_x[i] = 0.0;
            }
        }
    }
    else
    {

        double sumInverseWeight = std::accumulate(m_dist2.begin(), m_dist2.end(), 0.0, [](double out, double w) {
            return out + 1/w;
        });
        std::transform(m_dist2.begin(), m_dist2.end(), m_weight_x.begin(), [&sumInverseWeight](double & d2) {
            return 1 / (d2*sumInverseWeight);
        } );


    }
    m_weight_y = m_weight_x;

    assert(precitec::math::isClose(std::accumulate(m_weight_x.begin(), m_weight_x.end(), 0.0), 1.0));
    assert(precitec::math::isClose(std::accumulate(m_weight_y.begin(), m_weight_y.end(), 0.0), 1.0));

}


void UniformGridMapBuilder::addWeightedPlaneCoordinates(int yAvg, const system::t_coordScreenToPlaneLine& rLine, double w_x, double w_y)
{
    //the computation is the same as changing scale, but in this case it's only an intermediate computation, not a change of reference system
    auto & rRow = m_averageMap.m_oGrid[yAvg];
    assert(rRow.size() == rLine.size());
    auto itRow = rRow.begin();
    for (auto &rInputCoord: rLine)
    {
        assert(itRow != rRow.end());
        assert(itRow->ScreenX == rInputCoord.ScreenX);
        assert(itRow->ScreenY == rInputCoord.ScreenY);
        itRow->RealX +=(rInputCoord.RealX * w_x);
        itRow->RealY +=(rInputCoord.RealY * w_y);
        itRow++;
    }
}

} //end namespace math
} //end namespace precitec
 


/*!
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			LB
 *  @date			2020
 *  @file
 *  @brief			Container for position-dependent calibration correction
 */

#ifndef CALIBRATIONCORRECTIONCONTAINER_H
#define CALIBRATIONCORRECTIONCONTAINER_H

#include "common/defines.h"
#include "message/serializer.h"
#include "message/messageBuffer.h"
#include "geo/point.h"

#include <array>
#include <string>
#include <set>

//forward declaration
class TestCalibrationCorrectionContainer;


namespace precitec {
//forward declaration
namespace calibration {
    class AcquireScanfieldRoutine;
    class CalibrateScanField; 
    class IDMMeasurements;
}
    
namespace coordinates{
    
    
struct CalibrationCameraCorrection
{
    double m_oTCPXOffset = 0.0;
    double m_oTCPYOffset = 0.0;
    void apply(double & rTCPx, double & rTCPy ) const
    {
        rTCPx +=  m_oTCPXOffset;
        rTCPy +=  m_oTCPYOffset;
    }
    static CalibrationCameraCorrection weightedAverage(double k1, const CalibrationCameraCorrection & rValue1, double k2, const CalibrationCameraCorrection & rValue2)
    {
        return {
                k1 * rValue1.m_oTCPXOffset + k2 * rValue2.m_oTCPXOffset, 
                k1 * rValue1.m_oTCPYOffset + k2 * rValue2.m_oTCPYOffset
            };
    }
};

struct CalibrationIDMCorrection
{
    int m_oDelta; //FIXME at the moment just the median value of m_rawIDMMeasurements, should be the offset to the parabola fit due to the lens distortion
    int apply(int depth) const
    {
        return depth + m_oDelta;
    }
    static CalibrationIDMCorrection weightedAverage(double k1, const CalibrationIDMCorrection & rValue1, double k2, const CalibrationIDMCorrection & rValue2)
    {
        return {
                int(k1 * rValue1.m_oDelta + k2 * rValue2.m_oDelta)
            };
    }

};
    
// 2D grid (for easier access and interpolation)
template<typename T>
class CalibrationCorrectionContainer : public system::message::Serializable
{
public:
    typedef double InputElementType;      
    enum index_type {eX, eY, eNUM};
    typedef std::array<InputElementType, index_type::eNUM> InputPosition; //i.e Scanner position [mm]
    
    CalibrationCorrectionContainer<T>(){};
    CalibrationCorrectionContainer<T>(const std::vector<std::pair<geo2d::DPoint, T>> & offsets);

    
    void serialize ( system::message::MessageBuffer &buffer ) const;
    void deserialize( system::message::MessageBuffer const&buffer );
    
    T get(InputElementType x,  InputElementType y) const;
    T get(InputPosition pos) const;
    unsigned int getNumberOfMeasurements() const
    {
        return m_correctionArray.size();
    }
    
    
    template <index_type t>
    double transformInputToIndex (InputElementType realValue) const
    {
        static_assert(t >=  0 && t < index_type::eNUM,  "");
        return (realValue - m_min[t]) / m_delta[t];
    }
    
    template <index_type t>
    InputElementType transformIndexToInput (double index) const
    {
        static_assert(t >=  0 && t < index_type::eNUM,  "");
        return index * m_delta[t] + m_min[t];
    }
    
    InputPosition computeInputPosition (int index) const;
    
    // index in the 1d array (no bound check)
    unsigned int index(unsigned int i,  unsigned int j) const;
    
    void resize(unsigned int w,  unsigned int h);
    
    std::vector<T> m_correctionArray;
    unsigned int m_width;
    unsigned int m_height;
    InputPosition m_min;
    std::array<InputElementType, index_type::eNUM> m_delta;
    friend TestCalibrationCorrectionContainer;
};

template<typename T>
CalibrationCorrectionContainer<T>::CalibrationCorrectionContainer(const std::vector<std::pair<geo2d::DPoint, T>> & offsets)
{
    //assuming the configuration already contains equally spaced points, with precise position (because I am not using a fuzzy comparison to store the results)
    typedef typename CalibrationCorrectionContainer<T>::index_type index_type;

    auto lambda_loadGridSize= [&offsets, this] ()
    {
        std::map<double, std::set<double>> rows;
        for (auto & rEntry: offsets)
        {
            auto & rPosition = rEntry.first;
            rows[rPosition.y].insert(rPosition.x);
        }
        
        m_width = 0;
        m_height = rows.size();
        m_min =  {0.0, 0.0};
        m_delta = {1.0, 1.0}; // default 1 to avoid division by zero
            
        auto lambda_checkDelta = [&] (std::set<double> & rValues, double delta)
        {
            auto it = rValues.begin();
            auto prevValue = *it;
            it++;
            for (; it != rValues.end(); it++ )
            {
                if ( (*it) - prevValue != delta)
                {
                    return false;
                }
                prevValue = *it;
            }
            return true;
        };
        
        int rowCounter = 0;
        double prevY = 0.0;
        for (auto & row: rows)
        {
            if (rowCounter == 0)
            {
                m_min[index_type::eY] = row.first;
                auto & rXpositions = row.second;
                unsigned int numColums = rXpositions.size();
                m_width = numColums ;
                m_min[index_type::eX] = numColums > 0 ? *(rXpositions.begin()) : 0.0;
                if (numColums > 1)
                {                            
                    m_delta[index_type::eX] = *next(rXpositions.begin()) - m_min[index_type::eX];
                }
            }
            
            auto & rXpositions = row.second;
            if (rXpositions.size() != m_width)
            {
                return false;
            }
            if (*rXpositions.begin() != m_min[index_type::eX])
            {                
                return false;
            }
            if (!lambda_checkDelta(rXpositions, m_delta[index_type::eX]))
            {
                return false;
            }
                        
            if (rowCounter == 1)
            {
                assert(m_min[index_type::eY] == prevY);
                m_delta[index_type::eY] = row.first - prevY; 
            }
            
            if (rowCounter > 1)
            {
                if ( (row.first - prevY) != m_delta[eY])
                {                    
                    return false;
                }
            }
            
            prevY = row.first;
            rowCounter++;
        }
        return true;
    };

    bool valid = lambda_loadGridSize();
    if (!valid)
    {
        m_correctionArray.clear();
        return;
    }
    resize(m_width, m_height);
    for (auto & rEntry :  offsets)
    {
        auto & rScannerPos = rEntry.first;
        auto & rOffset = rEntry.second;
        double curIndex = index(transformInputToIndex<index_type::eX>(rScannerPos.x), transformInputToIndex<index_type::eY>(rScannerPos.y));
        assert(curIndex ==  trunc(curIndex));
        assert(curIndex < m_correctionArray.size());
        m_correctionArray[curIndex] = rOffset;
    }
}

class CalibrationCameraCorrectionContainer : public system::message::Serializable
{
public:
    static CalibrationCameraCorrectionContainer load(std::string filename);
    static bool write(const CalibrationCameraCorrectionContainer & corrections, std::string filename); 
    
    CalibrationCameraCorrectionContainer(){}
    CalibrationCameraCorrectionContainer(const std::vector<std::pair<geo2d::DPoint, CalibrationCameraCorrection>> & offsets)
    : m_data(offsets)
    {}
    
    void serialize ( system::message::MessageBuffer &buffer ) const;
    void deserialize( system::message::MessageBuffer const&buffer );
    
    CalibrationCameraCorrection get(CalibrationCorrectionContainer<CalibrationCameraCorrection>::InputPosition pos) const;
    CalibrationCameraCorrection get(double x,  double y) const;
    bool empty() const {return m_data.getNumberOfMeasurements() == 0;}

private:
    CalibrationCorrectionContainer<CalibrationCameraCorrection> m_data;

    friend calibration::AcquireScanfieldRoutine;
    friend calibration::CalibrateScanField;
    friend TestCalibrationCorrectionContainer;   
};

class CalibrationIDMCorrectionContainer : public system::message::Serializable
{
public:
    static CalibrationIDMCorrectionContainer load(std::string filename);
    static bool write(const CalibrationIDMCorrectionContainer & corrections, std::string filename); 
    
    CalibrationIDMCorrectionContainer();
    CalibrationIDMCorrectionContainer(const std::vector<std::pair<geo2d::DPoint, CalibrationIDMCorrection>> & offsets); 
    
    void serialize ( system::message::MessageBuffer &buffer ) const;
    void deserialize( system::message::MessageBuffer const&buffer );
    
    int getDelta(double x,  double y) const;
    double get(double x,  double y, double rawZ) const;
    bool empty() const {return m_data.getNumberOfMeasurements() == 0;}
private:
    
    struct Model
    {

        double fit( double x, double y, double rawZ ) const
        {
            //return std::sqrt(rawZ*rawZ + x*x + y*y); 
            return rawZ; 
        }
    };
    CalibrationCorrectionContainer<CalibrationIDMCorrection> m_data;
    Model m_model;
    friend TestCalibrationCorrectionContainer;
    friend calibration::IDMMeasurements;
};

class CalibrationCameraCorrectionState
{
public:
    CalibrationCameraCorrectionState (CalibrationCameraCorrectionContainer init);
    
    bool updateCorrectionPosition(double x,  double y, int index);
    geo2d::DPoint getCurrentPosition(int index) const;
    const CalibrationCameraCorrection & getCurrentCorrection(int index) const;
    CalibrationCameraCorrection computeCorrection(double x,  double y) const;
    const CalibrationCameraCorrectionContainer & getCorrectionContainer() const;

private:
    const CalibrationCameraCorrectionContainer m_container;
    std::array<CalibrationCorrectionContainer<CalibrationCameraCorrection>::InputPosition, g_oNbParMax> m_currentPosition;
    std::array<CalibrationCameraCorrection, g_oNbParMax> m_currentCorrection;
    friend calibration::CalibrateScanField;
};





template<typename T>
void CalibrationCorrectionContainer<T>::serialize ( system::message::MessageBuffer &buffer ) const
{
    marshal(buffer, m_width );
    marshal(buffer, m_height);
    marshal(buffer, m_min[0]);
    marshal(buffer, m_min[1]);
    marshal(buffer, m_delta[0]);
    marshal(buffer, m_delta[1]);        
    assert(buffer.hasSpace(sizeof(CalibrationCameraCorrection) *m_correctionArray.size() ));
    marshal(buffer, m_correctionArray);
    
}

template<typename T>
void CalibrationCorrectionContainer<T>::deserialize( system::message::MessageBuffer const&buffer )
{
    deMarshal(buffer, m_width );
    deMarshal(buffer, m_height);
    deMarshal(buffer, m_min[0]);
    deMarshal(buffer, m_min[1]);
    deMarshal(buffer, m_delta[0]);
    deMarshal(buffer, m_delta[1]);
    deMarshal(buffer, m_correctionArray);
}

template<typename T>
void CalibrationCorrectionContainer<T>::resize(unsigned int w,  unsigned int h)
{
    m_width = w;
    m_height = h;
    m_correctionArray.resize(w*h);
}

template<typename T>
T CalibrationCorrectionContainer<T>::get(InputElementType x,  InputElementType y) const
{
    if (m_correctionArray.size() == 0)
    {
        return {};
    }
    
    if (m_correctionArray.size() == 1)
    {
        return m_correctionArray[0];
    }
    
    double i = transformInputToIndex<eX>(x);
    double j = transformInputToIndex<eY>(y);
    
    // index outside of grid: assuming that the calibration covers the full grid,  no extrapolation is performed
    {
        if (i < 0)
        {
            i = 0;
        }
        if (i >  m_width -1)
        {
            i = m_width -1;
        }
        if (j < 0)
        {
            j = 0;
        }
        if (j > m_height -1)
        {
            j = m_height -1;
        }
    }

    
    // no interpolation needed
    if (i == trunc(i) && j ==  trunc(j))
    {
        return m_correctionArray[index(i, j)];
    }
        
    // bilinear interpolation
    {
        auto fInterpolateHorizontally = [this] (double index_i, double index_j)
        {   
            if (index_i ==  trunc(index_i))
            {
                // j is a integer index, nothing to interpolate horizontally
                return m_correctionArray[index(index_i, index_j)];
            }
            
            double i1 = std::floor(index_i);
            double i2 = std::ceil(index_i);
        
            assert((i2 -i1 ) ==  1);
            const auto & v1 = m_correctionArray[index(i1, index_j)];
            const auto & v2 = m_correctionArray[index(i2, index_j)];
            auto k1 = i2 -index_i;
            auto k2 = index_i - i1;
            return T::weightedAverage(k1, v1, k2, v2);
        };
        
        
        if (j == trunc(j))
        {
            // j is a integer index, nothing to interpolate vertically
            return fInterpolateHorizontally(i, j);                                
        }
        
        double j1 = std::floor(j);
        double j2 = std::ceil(j);
        assert((j2 -j1) ==  1);
        auto oCorrectionY1 = fInterpolateHorizontally(i, j1);
        auto oCorrectionY2 = fInterpolateHorizontally(i, j2);
        return T::weightedAverage(j2-j, oCorrectionY1, j -j1, oCorrectionY2);
    }            
}

template<typename T>
T CalibrationCorrectionContainer<T>::get(InputPosition pos) const
{
    return get(pos[eX], pos[eY]);
}


template<typename T>
typename CalibrationCorrectionContainer<T>::InputPosition CalibrationCorrectionContainer<T>::computeInputPosition (int index) const
{
    int j = index / m_width;
    int i =  index % m_width;
    return { transformIndexToInput<eX>(i), transformIndexToInput<eY>(j) };
}

// index in the 1d array (no bound check)
template<typename T>
unsigned int CalibrationCorrectionContainer<T>::index(unsigned int i,  unsigned int j) const
{
    return j * m_width + i;
}



}
}
#endif

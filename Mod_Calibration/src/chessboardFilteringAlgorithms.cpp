#include "calibration/chessboardFilteringAlgorithms.h"
#include <cmath>

namespace precitec
{
namespace calibration_algorithm
{


array2D::array2D() : m_oSize(7)   // symmetric (3,1,3) x (3,1,3) kernel
{
    init();
}

array2D::array2D(const int p_oSize) : m_oSize(p_oSize)
{
    init();
}

void array2D::init()
{
    m_oData.resize(m_oSize);
    for (std::size_t i=0; i < m_oData.size(); ++i)
    {
        m_oData[i].assign(m_oSize, 0.0);
    }
    m_oKernelSize = m_oSize/2;
    m_oFilterTransposed.resize(image::Size2d(m_oSize, m_oSize));
}

void array2D::setSize(const int p_oSize)
{
    m_oSize = p_oSize;
    init();
}

double array2D::at(const int p_oX, const int p_oY) const
{
    return m_oData[p_oY][p_oX];
}

void array2D::set(const int p_oX, const int p_oY, double p_oVal)
{
    m_oFilterTransposed.setValue(p_oY, p_oX, p_oVal);
    m_oData[p_oY][p_oX] = p_oVal;
}

bool array2D::setArray(const std::vector<double> &p_rVec, const int p_oSize, const double oMultiplier)
{
    const unsigned int oSize = (p_oSize * p_oSize);
    if (p_rVec.size() != oSize)
    {
        return false;
    }
    if (m_oSize != p_oSize)
    {
        setSize(p_oSize);
        m_oFilterTransposed.resize(image::Size2d(p_oSize, p_oSize));
    }
    for (int i=0; i < p_oSize; ++i)
    {
        for (int j=0; j < p_oSize; ++j)
        {
            if (std::fabs( p_rVec[p_oSize*i+j] ) > 0.000001)
            {
              this->set(i, j, oMultiplier*p_rVec[(p_oSize*i)+j]);
            } else
            {
              this->set(i, j, p_rVec[(p_oSize*i)+j]);
            }

        }
    }

    return true;
}

// get norm of filter (sum over entries)
double array2D::getNorm() const
{
    double oSum = 0;
    for (int x=0; x < this->size(); ++x)
    {
        for (int y=0; y < this->size(); ++y)
        {
             oSum += this->at(x, y);
        }
    }
    assert(m_oFilterTransposed.isContiguos());
    auto sum2 = std::accumulate(m_oFilterTransposed.begin(), m_oFilterTransposed.end(), 0.0);
    if (std::abs( oSum- sum2)  > 1e-16 )
    {
        std::cout << oSum << " " << sum2;
        assert(false);
    }
    return oSum;
}

const double* array2D::transposedBegin() const
{
    return m_oFilterTransposed.begin();
}


const double* array2D::transposedEnd() const
{
    return m_oFilterTransposed.end();
}



}
}

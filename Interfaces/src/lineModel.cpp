#include "geo/lineModel.h"
#include "math/mathCommon.h"

namespace precitec
{
namespace geo2d
{
    LineModel::LineModel()
        : m_a(0),
        m_b(0),
        m_c(0),
        m_oCenter(0.0,0.0)
    {
    }

    LineModel::LineModel(double x, double y, double a, double b, double c)
        : m_a(a),
        m_b(b),
        m_c(c),
        m_oCenter(x,y)
    {
        //ax + by + c = 0
        assert(math::isClose(m_a * x + m_b * y + m_c , 0.0));
    }

    void LineModel::getCoefficients(double & a, double &b, double &c) const
    {
        a = m_a;
        b = m_b;
        c = m_c;
    }

    const geo2d::DPoint & LineModel::getCenter() const
    {
        return m_oCenter;
    }

    std::tuple<double, double, double> LineModel::getCoefficients() const
    {
        return {m_a, m_b, m_c};
    }
}
}

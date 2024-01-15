/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		LB
* 	@date		2018
* 	@brief		Data structure which represents a line equation
*/

#ifndef LINEMODEL_H
#define LINEMODEL_H

#include <string>
#include <tuple>

#include "geo/point.h"
#include "InterfacesManifest.h"

namespace precitec
{
namespace geo2d
{

class INTERFACES_API LineModel
{
public:
    LineModel();
    LineModel(double x, double y, double a, double b, double c);
    
    void getCoefficients(double & a, double &b, double & c) const;
    std::tuple<double, double, double> getCoefficients() const;
    const geo2d::DPoint & getCenter() const;

private:
    double m_a;
    double m_b;
    double m_c;
    geo2d::DPoint m_oCenter;

};


} //namespaces
}
#endif

#ifndef PARABOLAEQUATION_H_
#define PARABOLAEQUATION_H_

#include "Analyzer_Interface.h"

namespace precitec {
namespace math {

class ANALYZER_INTERFACE_API ParabolaEquation
{
public:
    ParabolaEquation(void);
    virtual ~ParabolaEquation(void);
    void Reset();
    void AddPoint(double x, double y);
    void DelPoint(double x, double y);
    bool CalcABC(double & a, double & b, double & c);

protected:
    double S01, S11, S21;
    double S10, S20, S30, S40;
    int iPointAnz;
};


} // namespaces
}

#endif /* PARABOLAEQUATION_H_ */


#ifndef GENERALFIT_H
#define GENERALFIT_H

#include <vector>
#include "numericalTypes.h"

/*
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		JS
* 	@date		2016
* 	@brief		This class calculates the coefficients of a fitted polynom for a data set
*/

class GeneralFit
{

public:


    GeneralFit(std::vector<double> &xx, std::vector<double> &yy,std::vector<double> &ssig, std::vector<double> funks(const Doub))
        : ndat(xx.size()),
          x(xx),
          y(yy),
          sig(ssig),
          funcs(funks)
    {
            ma = funcs(x[0]).size();
            a.resize(ma);
            covar.resize(ma,ma);
            ia.resize(ma);
            for (Int i=0;i<ma;i++)  //alle Koeefizienten werden angepasst
            {
                ia[i] = true;
            }
    }


    ~GeneralFit(){}


    int testFunc(int var);
    void fit();
    void hold(const Int i, const Doub val);
    void free(const Int i);
    std::vector<double> getCoefficients();





private:

   int ndat;                                        ///< Anzahl der Punkte
    int ma;                                         ///<Anzahl der Parameter a(laufen von 0 bis M-1 --> Grad des Polynoms
    std::vector<double> &x,&y,&sig;                 ///<Daten x, y nd deren Unsicherheit sig

    std::vector<double> (*funcs)(const Doub);       ///<Fitting function
    VecBool ia;                                     ///<Welche Parameter werden fix gehalten: false (setzen mit Funktion hold

    std::vector<double> a;                          ///< Ergebnis vector mit den Koeffizienten
    MatDoub covar;                                  ///< Kovarianz Matrix
    Doub chisq;                                     ///< chi quadrat: Fehler des fits


};

#endif // GENERALFIT_H

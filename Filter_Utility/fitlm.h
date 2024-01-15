#ifndef FITLM_H
#define FITLM_H


#include <vector>
#include "numericalTypes.h"
#include "gaussjordan.h"


class FitLM {

public:


    FitLM(std::vector<double> &xx,std::vector<double>  &yy, std::vector<double> &ssig,
         std::vector<double>  &aa,void funks(const double, std::vector<double>&, double &, std::vector<double> &),
         const double TOL) :
        ndat(xx.size()),
        ma(aa.size()),
        x(xx),
        y(yy),
        sig(ssig),
        tol(TOL),
        funcs(funks),
        ia(ma),
        alpha(ma,ma),
        a(aa),
        covar(ma,ma)
    {
        for (Int i=0;i<ma;i++)
            ia[i] = true;
	}

    ~FitLM(){}

    /**
     * @brief Does the lm fit
    */
    void fit();

    /**
     * @brief delivers xi^2
     */
    double getchisq();

    /**
    * @brief sets a parameter of the model fix
    */
    void hold(const Int i, const double val)
    {
        ia[i]=false;
        a[i]=val;
    }
    void free(const Int i)
    {
        ia[i]=true;
    }

    std::vector<double> getCoefficients()
    {
        return m_oCoefficients;
    }


	void setLearnParams(int &done, int &iteration, double &lambda1, double &lambda2)
	{
		m_oDone = done;
		m_oMaxIteration=iteration;
		m_oLearnParam1 = lambda1;
		m_oLearnParam2 = lambda2;

	}



    /**
     * @brief generates a gauss function with disruptions for test purpose
     * @param coeff coefficeints : c[0]*exp((x - c[1]/c[2])^2)
     * @param gauss minimim
     * @param gauss maximum
     * @return gauss vector
    */
    std::vector<double> generateGauss(std::vector<double> &coeff,std::vector<double> &x,double &min,double &max);

    /**
     * @brief calcutales the gauss function according to the coefficients
     * @param coeff coefficeints : c[0]*exp((x - c[1]/c[2])^2)
     * @parama x values
     * @param gauss minimim
     * @param gauss maximum
     * @return gauss vector
    */
    std::vector<double> gaussVal(std::vector<double> &coeff,std::vector<double> &m_dXVec, double &min,double &max);

    /**
     * @brief calcutales the sine function according to the coefficients
     * @param coeff coefficeints : c[0] + c[1]*sin(x*c[2] + c[3])
     * @param m_dXVec x values
     * @param gauss minimim
     * @param gauss maximum
     * @return gauss vector
     */
    std::vector<double> sinusVal(std::vector<double> &coeff,std::vector<double> &m_dXVec, double &min,double &max);

private:

    void mrqcof(std::vector<double> &a, MatDoub_O &alpha, std::vector<double> &beta);
    void covsrt(MatDoub_IO &covar);


    int   ndat;                                                                          ///< Amount of points
    int   ma;                                                                            ///< number of coefficients internal
    std::vector<double> &x;                                                              ///< x data
    std::vector<double> &y;                                                              ///< y data
    std::vector<double>&sig;                                                             ///< sigma for each point
    double tol;                                                                          ///< Tolerance for the iterative change of the parameters
    void (*funcs)(const double, std::vector<double> &, double &, std::vector<double> &); ///< fit function
    VecBool ia;                                                                           ///< bool value to hold a parameter constant while fitting
    MatDoub alpha;                                                                        ///< A matrix
    std::vector<double> a;                                                                ///< coefficients of the model
    MatDoub covar;                                                                        ///< covariance matrix
    double  chisq;                                                                        ///< chi^2
    std::vector<double>  m_oCoefficients ;                                                ///< end coefficients
    int   mfit;                                                                            ///< number of parameters to fit
	int   m_oDone;
	int  m_oMaxIteration;
	double m_oLearnParam1;
	double m_oLearnParam2;

};
#endif // FITLM_H

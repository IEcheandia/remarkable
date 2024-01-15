#ifndef MODEL_H
#define MODEL_H
/*
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		JS
* 	@date		2016
* 	@brief		Interface to the mathematical functions
*/


//#include "generalfit.h"

#include <sstream>
#include <iostream>
#include <vector>

enum FitType {
	ePoly = 0,			///< polynom fit
	eSine = 1,			///< sine fit
	eGauss = 2		    ///< gauss fit
};

const double PI2 = 6.283185307179586476925286766559;

class Model
{

public:


	/**
	* @brief CTor.
	*/
	
	Model(	unsigned int &m_oConsensSet,
			double &m_oErrorThreshold,
			unsigned int &m_oIterations,
			unsigned int &m_oDegree,
			int &m_oEraseKernelSize,
			int &m_oEraseKernelDistance,
			unsigned int &m_oMode,
			std::vector<double> &m_oXVec,
			std::vector<double> &m_oYVec,
			double &m_oPeriodLength,
			FitType m_oModelType,
			int &m_oEraseStart,
			double &m_oMean,
			double &oAmplitude,
			int   &oHold,
			double &oLearnParam1,
			double &oLearnParam2
			) :
		m_dYVec(m_oYVec),
		m_dXVec(m_oXVec),
		m_oGrad(m_oDegree),
		m_oConsensNumber(m_oConsensSet),
		m_oIterationNumber(m_oIterations),
		m_oError(m_oErrorThreshold),
		m_oProcessingMode(m_oMode),
		m_oWindowSize(m_oEraseKernelSize),
		m_oIntervalDistance(m_oEraseKernelDistance),
		m_oPeriod(m_oPeriodLength),
		m_oType(m_oModelType),
		m_oEraseEntryStart(m_oEraseStart),
		m_oOffsetStart(m_oMean),
		m_oAmplitudeStart(oAmplitude),
		m_oHoldCoeff(oHold),
		m_oLambda1(oLearnParam1),
		m_oLambda2(oLearnParam2)
		{
		//m_oPeriod = PI2 / m_oPeriod;
	    }


	/**
	* @brief DTor.
	*/
	virtual ~Model()
	{
	}
		
		
	//void processDataNE();

	/**
	 * @brief Calculates a sine function with the paramets theta: theta[0] + theta[1]*sine(theta[2]*x + theta[3])
	 * @param x  x Values
	 * @param theta parameters
	 * @param minimum minimum of sine
	 * @param maximum maximum of sine
	 */
	std::vector<double> calculateSinus(std::vector<double> &x, std::vector<double> &theta, double &minimum, double & maximum);

	/**
	 * @brief fits a sine function to the vales x and y)
	 * @param x  x values
	 * @param y  y values
	 * @param period  start value of the parameter "peroid" to find the right minimum in fit method
	 * @param offset  start value of the parameter "offset" to find the right minimum in fit method
	 * @param amplitue start value of the parameter "amplitude" to find the right minimum in fit method
	 * @return coefficients of the aine function
	 */
	std::vector<double> fitSine(std::vector<double> &xVec, std::vector<double> &yVec,double period,double offset, double amplitude, int hold);

	/**
	 * @brief fits a polynom to the vales x and y
	 * @param x  x values
	 * @param y  y values
	 * @return coefficients of the polynom
	*/
	std::vector<double> fitNormalEquationPoly(std::vector<double> &xVec, std::vector<double> &yVec);


	/**
	 * @brief calculates the consensus number: how many point of the data are closer than the threshold to the model,
	 * calculates the mean square error of the fit
	 * @param polyVec data of the model
	 * @param dataVec measured data
	 * @return mean square error between model and data
	*/
	double calculateError(std::vector<double> &polyVec, std::vector<double>& dataVec, int& consensNumber, double threshold);


	/**
	 * @brief  eliminates data out of a vector with a window size and an interval distance
	 * @param  dataVec data in which elements will be elimitated
	 * @param  windowSize over which size the data will be eliminated
	 * @param  intervalDistance distance between the eliminating windows
	 * @return the resized data
	*/
	std::vector<double> eraseNumbers(std::vector<double> &dataVec, int windowSize, int intervalDistance, int start);

	/**
	 * @brief  executes the ransac process, it iterates data fits while eliminating measurement data
	 * to find the best fit without outliers
	*/
	void processRansac();

	/**
	 * @brief  replaces outliers with data from the model to get a proper measurement data set without ouliers
	 *
	*/
	void processCleanSignal();

	/**
	 * @brief  makes the celaning process: replaces the outliers
	 * @param vecFitted Model data
	 * @param yVec y data
	 * @param xVec x data
	 * @param threshold outlier threshold
	 *
	*/
	int cleanSignal(std::vector<double>& vecFitted, std::vector<double>& yVec, std::vector<double>& xVec, double threshold);

	double calcMinMaxMean(std::vector<double> &m_oYVec, std::vector<double> &m_oXVec, double &min, double &max);



	std::vector<double> getCurve();
	std::vector<double> getCoefficients();
	double getMin();
	double getMax();
	double getMean();
	double getOutlier();
	double getKoeff1();
	double getKoeff2();
	double getKoeff3();
	double getConsens();
	double getError();
	double getCost();



	/**
	* @brief Calculates the value of a polynomial of degree n evaluated at x. The input
    *	      argument pCoeff is a vector of length n+1 whose elements are the coefficients
	*          in incremental powers of the polynomial to be evaluated.
    * @param: oCoeff polynomial coefficients
	* @param: oX     x axis values
    * @return: polynomial values
	*/
	template<typename T>
	std::vector<T> polyval(const std::vector<T>& oCoeff, const std::vector<T>& oX)
	{
		size_t nCount = oX.size();
		size_t nDegree = oCoeff.size();
		std::vector<T>	oY(nCount);

		for (size_t i = 0; i < nCount; i++)
		{
			T nY = 0;
			T nXT = 1;
			T nX = oX[i];
			for (size_t j = 0; j < nDegree; j++)
			{
				// multiply current x by a coefficient
				nY += oCoeff[j] * nXT;
				// power up the X
				nXT *= nX;
			}
			oY[i] = nY;
		}

		return oY;
	}



private:
		
	std::vector<double> m_dYVec;  ///< measured data
	std::vector<double> m_dXVec;  ///< x coordinates of the measured data

	int m_oGrad;                   ///< polynom order
	int m_oConsensNumber;          ///< consensus data - how many points are under the the error around the model
	int m_oIterationNumber;        ///< how much iterations will be done in the ransac process
	double m_oError;               ///< square error between model and data
	unsigned int m_oProcessingMode;///< which mode will be used for a front up cleaning of the data

	int m_oWindowSize;             ///< data eliminating window
	int m_oIntervalDistance;       ///< data eliminating window distance
	double m_oPeriod;              ///< guess period for the sine function
	FitType  m_oType;              ///< type of model
	int   m_oEraseEntryStart;      ///< where to start in the vector to erase entries

	double m_oOffsetStart;         ///< offset start value for the sine fit
	double m_oAmplitudeStart;      ///< amplitue start value for the sine fit
	int    m_oHoldCoeff;           ///< hold period length fix in the fit
	double m_oLambda1;
	double m_oLambda2;


	std::vector<double> m_oCoeff;   ///< model coefficients
	std::vector<double> m_oFittedY; ///< fitted data vector
	std::vector<double> m_oBestCoeff;///< best coefficients as a result of the ransac process
	

	int   m_oDataLength;             ///< data vector length
	double m_oLowerThreshold;        ///< thresholds for front up cleaning
	double m_oUpperThreshold;        ///< threshold for front up claaning
	//const int m_oLength = 400;
	int   m_oOutlierCounter;          ///< number of outliers
	double m_oMax;                    ///< model maximum
	double m_oMin;                    ///< model minimum
	double m_oMeanValue;              ///< model mean value
	double m_oMaxConsens;             ///< best number of consensus at result of the ransac process
	double m_oBestConsens;            ///<
	double m_oBestError;              ///< smallest error as a result of the ransac process
	double m_oBestCost;               ///< value of the cost function



};

#endif // MODEL_H

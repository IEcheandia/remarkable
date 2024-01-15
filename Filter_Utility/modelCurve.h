/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		JS
 * 	@date		2016
 * 	@brief		This filter stores data elements in a ring buffer and models a curve through the data
 */

#ifndef MODELCURVE_H_
#define MODELCURVE_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include <fliplib/PipeEventArgs.h>		///< Eventprocessing
#include "fliplib/SynchronePipe.h"
#include <geo/geo.h>
#include <geo/array.h>
#include "common/geoContext.h"

//#include "model.h"


namespace precitec {
namespace filter {



class BasicStatistic
{
public:
	BasicStatistic();
	void reset();
	void addValue(double value);
	double getMedian();
	double getMean();

private:
	std::vector<double> _data;
	void sortIt();
	void exchange(double & d1, double & d2);
};


//Datenklasse fuer TempBuffer
class SingleDataSet
{
public:
	SingleDataSet();
	SingleDataSet(const SingleDataSet & anotherSet);
	SingleDataSet(double data_, int data_rank_, double pos_, int pos_rank_);
	double data;
	int data_rank;
	double pos;
	int pos_rank;
};

class TempBuffer
{
public:
	TempBuffer();
	void reset();
	void addOneDataSet(SingleDataSet set);
	SingleDataSet getOneDataSet(int i);
	int getCurrentSize();
	bool isSorted();
	bool is360();
	void sort();
	void setTicksPer360(int number);
//	void setWidths(int widthMean, int widthMedian);
	void calcStartEndValues(int & start, int & end);
	double getMinPos();
	double getMaxPos();
	void reduceToStartEnd(int start, int end);
	bool isInRange(double posTicks1, double posTicks2, double angleMax);
	void makePosModulo();

//	double calcMeanWithMinRank(int minRank);
	void setDataForBadRank(int badRank, double dataToSet);
//	void doMeanOnRange(int firstSet, int lastSet, int meanSize);
//	void doMedianOnRange(int firstSet, int lastSet, int medianSize);
	void interpolateLinearOnRange(int startLastValid, int endFirstValid);
//	bool lowPassAndInterpolate(unsigned int mode);

//	void doLowPass();
//	void doLowPass(unsigned int mode);

	std::vector<double> getDataY();	//den Ergebniswert
	std::vector<double> getDataX(); //den Encoder- oder sonst was Wert

private:
	std::vector<SingleDataSet> _dataSet;
	void exchange(int i, int j);
	int _ticksPer360;
//	void doLowPassOnRange(int firstSet, int lastSet, int lowPassSize, bool useMean);
	int round(double d);

};




/**
 * @brief This filter stores data elements in a buffer. They can be accessed in a different filter graph using the buffer player filter.
 */
class FILTER_API ModelCurve : public fliplib::TransformFilter
{
public:

	/**
	 * @brief CTor.
	 */
	ModelCurve();
	/**
	 * @brief DTor.
	 */
	virtual ~ModelCurve();

	// Declare constants
	static const std::string m_oFilterName;			///< Filter name.
	static const std::string PIPENAME_MIN;			///< Name Out-Pipe 1
	static const std::string PIPENAME_MAX;			///< Name Out-Pipe 1
	static const std::string PIPENAME_OUTLIER;		///< Name Out-Pipe 1
	static const std::string PIPENAME_KOEFF1;		///< Name Out-Pipe 1
	static const std::string PIPENAME_KOEFF2;	    ///< Name Out-Pipe 1
	static const std::string PIPENAME_CURVE;		///< Name Out-Pipe 1


	static const std::string m_oParamSlot;			    ///< Parameter: Slot, into which the data is written.
	static const std::string m_oParamMode;			    ///< Parameter
	static const std::string m_oParamTicks;			    ///< Parameter
	static const std::string m_oParamConsensSet;	    ///< Parameter
	static const std::string m_oParamErrorThreshold;	///< Parameter
	static const std::string m_oParamIterations;	    ///< Parameter
	static const std::string m_oParamLowerThreshold;	///< Parameter
	static const std::string m_oParamUpperThreshold;	///< Parameter
	static const std::string m_oParamDegree;			///< Parameter
	static const std::string m_oParamEraseKernelSize;	///< Parameter
	static const std::string m_oParamEraseKernelDistance;	///< Parameter
	static const std::string m_oParamTickSize;		    ///< Parameter
	static const std::string m_oParamDrawFactor;        ///< Parameter
	static const std::string m_oParamDrawMax;           ///< Parameter
	static const std::string m_oParamDrawMin;           ///< Parameter
	static const std::string m_oParamYAxisFactor;       ///< Parameter
	static const std::string m_oParamPeriodLength;      ///< Parameter
	static const std::string m_oParamModelType;         ///< Parameter
	static const std::string m_oParamEraseStart;         ///< Parameter
	static const std::string m_oParamHold;               ///< Parameter
	static const std::string m_oParamLearnParam1;        ///< Parameter
	static const std::string m_oParamLearnParam2;        ///< Parameter

	/**
	 * @brief Set filter parameters.
	 */
	void setParameter();

	/**
	 * @brief Arm the filter. This means here, that the length of the seam is determined and memory is allocated for all the data elements.
	 */
	virtual void arm(const fliplib::ArmStateBase& state);

	void paint();


	/**
	 * @brief Generates a sinus function for test purposes
	 * @param xVec  xValues
	 * @param yVec  sine values
	 * @param yMax  max value of the sine curve
	 */
	void generateSinus(std::vector<double> &xVec, std::vector<double> &yVec,double &yMax);

	/**
	* @brief eliminates the highest and lowest % percent values out of the data buffer.
	* @param sortVec
	* @param yVec
	* @param upThresh  max value of the new buffer
	* @param lowThresh min value of the new value
	* @param percent   
	* @return -1 error, 1 ok
	*/
	int bufferCorrection(std::vector<double> &sortVec,std::vector<double> &yVec, double &upThresh, double &lowThresh, double &percent);

	/**
	* @brief eliminates the highest and lowest % percent values out of the data buffer.
	* @param yVec data buffer
	* @param min  min of the data buffer 
	* @param max  max of the data bufefr
	* @param percent in double values: f.e. 90 percent means shrink the vales by 0.9 ?
	* @return 
	*/
	double bufferMeanCorrection(std::vector<double> &yVec,double &min, double &max, double &percent);

	/**
	* @brief computes the mean value of a vector.
	* @param yVec data buffer
	* @return mean value
	*/
	double calcMeanValue(std::vector<double> &rYVec);

protected:

	/**
	 * @brief In-pipe registration.
	 * @param p_rPipe Reference to pipe that is getting connected to the filter.
	 * @param p_oGroup Group number (0 - proceed is called whenever a pipe has a data element, 1 - proceed is only called when all pipes have a data element).
	 */
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);

	/**
	 * @brief Processing routine.
	 * @param p_pSender pointer to
	 * @param p_rEvent
	 */
	void proceedGroup( const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEvent );

protected:

	const fliplib::SynchronePipe< interface::GeoDoublearray >*	m_pPipeInData;			///< Data in-pipe.
	const fliplib::SynchronePipe< interface::GeoDoublearray >*	m_pPipeInPos;			///< Position in-pipe.


	fliplib::SynchronePipe< interface::GeoDoublearray >       	m_oPipeOutMin;	     ///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >       	m_oPipeOutMax;	     ///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >       	m_oPipeOutOutlier;	 ///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >       	m_oPipeOutKoeff1;	 ///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >       	m_oPipeOutKoeff2;	  ///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray >       	m_oPipeOutCurve;	 ///< Out pipe


	geo2d::Doublearray								  			m_oMinOut;			///< Output value.
	geo2d::Doublearray								  			m_oMaxOut;			///< Output value.
	geo2d::Doublearray								  			m_oOutlierOut;		///< Output value.
	geo2d::Doublearray								  			m_oKoeff1Out;		///< Output value.
	geo2d::Doublearray								  			m_oKoeff2Out;		    ///< Output value.
	geo2d::Doublearray								  			m_oCurveOut;		///< Output value.
	




	interface::SmpTrafo											m_pTrafo;				///< ROI trafo.
	TempBuffer													_ownBuffer;				///< internal buffer for storing the data from the inpipe

	int															m_oCount;				///< Bildzaehler, d.h. Aufrufe von Proceed
	bool														m_bPaintFlag;			///<Flag fuer Paint Routine
	bool														m_oProcessingFlag;      ///<Flag for processing


	unsigned int												m_oSlot;				///< Parameter: Slot, into which the data is written.
	unsigned int												m_oMode;				///< Parameter: Mode, switch for different processing modes.
	unsigned int												m_oTicks;				///< Parameter: Number of ticks equal to 360 degrees.
	unsigned int 												m_oConsensSet;			///< Parameter: consens set
	double 														m_oErrorThreshold;		///< Parameter: outlier threshold
	unsigned int 												m_oIterations;			///< Parameter: iterations for the RANSAC procedure
	double 														m_oLowerThreshold;		///< Parameter: lower threshold for valid values.
	double 														m_oUpperThreshold;		///< Parameter: upper threshold for valid values.
	unsigned int 												m_oDegree;				///< Parameter: degree for the mathematical model.
	int															m_oEraseKernelSize;     ///< Parameter: Window size for erasing data
	int															m_oEraseKernelDistance; ///< Parameter: Distance between the erasing windows
	unsigned int 												m_oTickSize;			///< Parameter: ticksize for the processing start.
	double														m_oDrawFactor;			///< Parameter: factor to scale the values for drawing.
	double														m_oDrawMax;				///< Parameter: max axis value for drawing.
	double														m_oDrawMin;				///< Parameter: min axis value for drawing.
	double														m_oYAxisFactor;         ///< Parameter: y axis offset in the drawing.
	double                                                      m_oPeriodLength;        ///< Parameter: start period length for the lm algorithm.  
	int                                                         m_oModelType;           ///< Model Type of Fit 
	int                                                         m_oEraseStart;           ///< Where to start in the array to delete entries
	int                                                         m_oHold;                 ///< Which coefficient to fix in the fit
	double                                                      m_oLearnParam1;          ///< learning parameter1 in the gradient descnet part of the on linear fit
	double                                                      m_oLearnParam2;          ///< learning parameter1 in the gradient descnet part of the on linear fit

	

	std::shared_ptr< interface::GeoDoublearray > 				m_pData;				///< Pointer to the actual data array. Is valid after arm.
	std::shared_ptr< interface::GeoDoublearray > 				m_pPos;					///< Pointer to the actual pos array. Is valid after arm.

	int 														m_oTriggerDelta;		///< Trigger distance [um]

	geo2d::TPoint<double>										m_oHwRoi;														
											

//	interface::GeoDoublearray									m_bufferData;
//	interface::GeoDoublearray									m_bufferPos;


	std::vector<double>											m_oVecX;				///< vector with the processing data X
	std::vector<double>											m_oVecY;				///< vector with the processing data Y	
	double														m_oMaxPos;				///< max position in the processing data	
	double														m_oMinPos;				///< min position in the processing data
	double														m_oMaxData;				///< max data in the processing data	
	double														m_oMinData;				///< min data in the processing data
	double														m_oMeanData;			///< mean value
	double														m_oOutlier;				///< outliers above the threshold
	double														m_oKoeff1;              ///< Coefficient 1 from model function
	double														m_oKoeff2;              ///< Coefficient 2 from model function
	double                                                      m_oKoeff3;              ///< Coefficient 3 from model function
	double                                                      m_oConsens;             ///< Consens data  
	double                                                      m_oError;               ///< Error
	double                                                      m_oCost;                ///< cost



	std::vector<double>											m_oResultCurve;			///< resulted fitted curve
	std::vector<double>											m_oVecXOrig;			///< original x Daten 

}; // class RingBufferRecorder


} // namespace filter
} // namespace precitec

#endif /* RINGBUFFERRECORDER_H_ */

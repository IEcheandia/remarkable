

#include <sstream>
#include <iostream>
#include <fstream>

#include <vector>

#include <module/moduleLogger.h>

#include "model.h"
#include "generalfit.h"
#include "fitlm.h"

int   polynomGrad;
double string_to_double(const std::string& str);
int    string_to_int(const std::string& str);


std::vector<double> cubicFit(const double x);
std::vector<double> npolyFit(const double x);





// Gauss als Model Funktion
// 3 Parameter: a[3]: a[0]- amplitude, a[1]- center, a[2]- sigma
void fgauss(const double x, std::vector<double> &a, double &y, std::vector<double> &dyda)
{
	//int na = a.size();
	double fac, ex, arg;
	y = 0.;
	arg = (x - a[1]) / a[2];
	ex = exp(-SQR(arg));
	fac = a[0] * ex*2.*arg;
	y = a[0] * ex;
	dyda[0] = ex;
	dyda[1] = fac / a[2];
	dyda[2] = fac*arg / a[2];

}

// Modell einer allgemeinen Sinus Funktion
// y = a0 + a1*sin(a2*x + a3)
// oder
// y = b0 + b1*sin(x) + b2*cos(x) mit vorgegebener Frequenz --> x ist eigentlich omega*x
// 4 Parameter: a[0]- offset, a[1]- amplitude, a[2]- Freqquenz, a[3] - Phase
void fsinus(const double x, std::vector<double> &a, double &y, std::vector<double> &dyda)
{
	//int i; // na = a.size();
	double farg, arg;
	y = 0.;
	arg = (a[2] * x) + a[3];
	farg = sin(arg);
	y = a[0] + a[1] * farg;

	dyda[0] = 1;
	dyda[1] = farg;
	dyda[2] = a[1] * cos(arg)*x;
	dyda[3] = a[1] * cos(arg) * 1;

}


// Calculate the values of a sine function with the parameters theta togetehr with min and max of the function
std::vector<double> Model::calculateSinus(std::vector<double> &x, std::vector<double> &theta, double &minimum, double & maximum)
{
	int m = x.size();
	std::vector<double> vresult(m);
	maximum = -100000000000.0;
	minimum = 1000000000000.0;
	for (int i = 0; i<m; ++i)
	{
		vresult[i] = theta[0] + theta[1] * sin(theta[2] * x[i] + theta[3]);
		if (vresult[i] < minimum)
		{
			minimum = vresult[i];
		}
		if (vresult[i] > maximum)
		{
			maximum = vresult[i];
		}

	}

	return vresult;
}







/*
*   call the sine fit
*/
std::vector<double> Model::fitSine(std::vector<double> &xVec, std::vector<double> &yVec,double period,double offset, double amplitude, int hold)
{

	std::vector<double> coeffs; //size = zero
	if (xVec.size() > 0 && yVec.size()>0)
	{

		const int npts = xVec.size();        //Anzahl der Punkte
		if (npts < 5)
		{
			std::cout << "no data available" << std::endl;
			return coeffs;
		}
		else
		{
			//std::cout<<"Data size in fit sine: "<<npts<<std::endl;
		}

		std::vector<double> ssig(npts);
		ssig.assign(npts, 1.0);                  //Unsicherheit der Punkte alle auf 1.0 setzen

		// aa guess parameters:
		// 0 offset, 1 Amplitude, 2 Frequenz, 3 Phase
		// ohne die richtige Startfrequenz geht nix
		std::vector<double> aa(4);

		//period
		aa[2] = 0.0157;
		if (period > 0)
			aa[2] = PI2 / period;

		//offset
		aa[0] = 0.0;
		if (offset > 0.0)
			aa[0] = offset;

		//amplitude
		aa[1] = 20.0;
		if (amplitude > 0.0)
			aa[1] = amplitude;

		//phase
		aa[3] = 0.0;



		//std::cout <<"Start Koeffizienten Sine Fit: " <<aa[0]<< ", "<<aa[1]<<", "<<aa[2]<<", "<<aa[3]<<std::endl;

		/*
		ofstream file;
		file.open("values.txt",ios::app);
		int k=0;

		for(int m=0;m<npts;m++)
		{
			file << xVec[m] << ";" << yVec[m] << std::endl;
		    k++;
		}

		file.close();
		std::cout <<" "<<k<<" Werte gespeichert"<<std::endl;
        */

		const double TOL = 1.e-3;
		FitLM lmModelObj(xVec, yVec, ssig, aa, fsinus, TOL);

		//hold period fix
		if(hold>-1)
		{
			if( (hold>=0) && (hold < int(aa.size())) )
			    lmModelObj.hold(hold,aa[hold]);
		}


		int done = 10;
		int iteration = 1000;
		double lambda1 = 0.1;
		double lambda2 = 2;
		lmModelObj.setLearnParams(done, iteration, lambda1, lambda2);


		//for(int i =0;i<1000;++i)
		lmModelObj.fit();

		std::vector<double> coeffs;
		coeffs = lmModelObj.getCoefficients();

		//std::stringstream strs;
		//std::cout <<"Sinus Koeffizienten nach LM Fit: " << coeffs[0] << "," << coeffs[1] << "," << coeffs[2] << "," << coeffs[3];
		//std::cout << " Chi Square: " << chisquare<<std::endl;

		return coeffs;
	}
	else
	{
		std::cout<<"No data available"<<std::endl;
		return coeffs;

	}


}






/*
*   Hilfsfunktion zum loopen ueber den polyFit mittels NE Verfahren
*/
std::vector<double> Model::fitNormalEquationPoly(std::vector<double> &xVec, std::vector<double> &yVec)
{
	std::vector<double> coefficients;   //Polynom Koeefizienten

	if (yVec.size() > 0 && xVec.size()>0)
	{

		const int npts = xVec.size();        //Anzahl der Punkte

		std::vector<double> ssig(npts);
		ssig.assign(npts, 1.0);                  //Unsicherheit der Punkte alle auf 1.0 setzen

		GeneralFit gfObj(xVec, yVec, ssig, npolyFit);
		gfObj.fit();
		coefficients = gfObj.getCoefficients();
	}
	else
		coefficients.clear();

	return coefficients;

}




std::vector<double> cubicFit(const double x)
{

	std::vector<double> ans(4);
	ans[0] = 1;
	for (int i = 1; i<4; i++)
	{
		ans[i] = x * ans[i - 1];
	}
	return ans;


}


std::vector<double> npolyFit(const double x) //geaendert auf Methode ----
{


	const int degree = polynomGrad;

	std::vector<double> p(degree);
	p[0] = 1.0;

	for (int j = 1; j<degree; j++)
	{
		p[j] = x * p[j - 1];
	}
	return p;
}



double Model::calculateError(std::vector<double> &polyVec, std::vector<double>& dataVec, int& consensNumber, double threshold)
{
	int status = 1;
	double distance = 0.0;
	double singleError = 0.0;

	int size = polyVec.size();

	if (size>0)
	{
		for (int i = 0; i<size; ++i)
		{
			singleError = std::abs(polyVec[i] - dataVec[i]);
			distance += (polyVec[i] - dataVec[i])*(polyVec[i] - dataVec[i]);

			if (singleError <= threshold)
			{
				consensNumber++;
			}

		}
		distance /= size;
		return distance;
	}
	else
		status = -1;

	return status;
}


/*
* Entfernt Daten aus einem Datensatz (vector):
* Es werden Daten aus einem Fenster entfernt, welches ueber den Datensatz geschoben wird
* dataVec: Eingangsdatensatz
* windowSize: Groesse des Blocks welcher jeweils entfernt wird
* intervaldistance: Abstand mit dem das Entfernungsfenster ueber den Datensatz geschoben wird
* start: start im Datensatz bei dem mit dem entfernen der Daten begonnen wird
* return: copy des reduzierten Datensatzes
*/
std::vector<double> Model::eraseNumbers(std::vector<double> &dataVec, int windowSize, int intervalDistance, int start)
{
	int sizeData = dataVec.size();
	std::vector<double> resultVec;

	//unsigned int initsize = sizeData;


	//startproblem
	int          eraseDataStart = 0;
	if ((start>intervalDistance) && start < (intervalDistance + windowSize) &&( (start-intervalDistance)<sizeData ) )
	{
		dataVec.erase(dataVec.begin(), dataVec.begin() + start - intervalDistance);
		sizeData  = dataVec.size();
		eraseDataStart += start - intervalDistance;
		start -= (start-intervalDistance);
	}


	int k = start;
	//int i = 0;
	int  eraseDataMitte = 0;
	while ((k+windowSize) <= sizeData)
	{
		dataVec.erase(dataVec.begin()+k, dataVec.begin() + k + windowSize);
		sizeData = dataVec.size();
		k += intervalDistance;
		eraseDataMitte += windowSize;

	}

	int  eraseDataEnd = 0;
	if ( ((k + windowSize)>sizeData) && (k<sizeData) )
	{
		dataVec.erase(dataVec.begin() + k, dataVec.end());
		eraseDataEnd += sizeData - k;
		sizeData = dataVec.size();
	}

	//int eraseData = eraseDataStart + eraseDataMitte + eraseDataEnd;


	resultVec = dataVec;

	return resultVec;
}





/*
* Ueber eine vorgegeben Anzahl von Iterationen werden Fits ueber den Daten durchgefuehrt
* in dem jeweils Daten entfernt werden.
* Nach jedem Fit wird der Konsens berechnet.
* Nach den Iterationen stehen mit dem besten Konsens verbundene Koeffizienten
* in der Membervariablen m_oBestCoeff
*
*/
void Model::processRansac()
{

	//original Datensatzlaenge
	m_oDataLength = m_dXVec.size();


	std::vector<double> xVec;
	std::vector<double> yVec;


	//start im Datensatz um Daten zu entfernen
	int start = m_oEraseEntryStart;//  2; // m_oIntervalDistance;//   20;
	int offset = 0;
	int windowSize = m_oWindowSize; // 30;
	int intervalDistance = m_oIntervalDistance; // 40;

	if((m_oGrad>1)&&(m_oGrad<8))
	{
	    polynomGrad = m_oGrad;
	}
    else
    {
        polynomGrad = 5;
    }

	if (m_oDataLength>0)
	{

		int ctr = m_oIterationNumber;
		int consensAnzahl = 0;
		int maxConsens = -1;

		if (ctr < 1)
		{
			xVec = m_dXVec;
			yVec = m_dYVec;
			double error = 0.0;
			double min = 0.0;
			double max = 0.0;

			if (yVec.size() > 0 && xVec.size()>0)
			{
				if (m_oType==ePoly)  // polyfit
				{
					m_oCoeff = fitNormalEquationPoly(xVec, yVec);
					m_oFittedY = polyval(m_oCoeff, xVec);
				}
				else if (m_oType==eSine) //sine fit
				{
					m_oCoeff    = fitSine(xVec,yVec,m_oPeriod,m_oOffsetStart, m_oAmplitudeStart,m_oHoldCoeff);
					if( m_oCoeff.size()>3)
					{
					    m_oFittedY =  calculateSinus(xVec, m_oCoeff, min, max);
					}
					else
					{
						precitec::wmLog(precitec::LogType::eWarning, "Keine Sinus Koeffizienten berechnet");
					}
					/*
					ofstream file;
					file.open("results.txt",ios::app);
					int k=0;

					for(int m=0;m<xVec.size();m++)
					{
					    file << xVec[m] << ";" << m_oFittedY[m] << std::endl;
						 k++;
					}
					file.close();
					std::cout <<" "<<k<<" Werte nach fit gespeichert"<<std::endl;
                    */

					//std::cout<<" min, max of sine: "<<min<<", "<<max<<std::endl;
				}
				else if(m_oType==eGauss)//gauss fit
				{
					precitec::wmLog(precitec::LogType::eWarning, "Datenverarbeitung Gauss Fit nicht implementiert");
				}


				consensAnzahl = 0;

				error = calculateError(m_oFittedY, yVec,consensAnzahl, m_oError);

				if ((consensAnzahl > 0) && (xVec.size()>0))
					consensAnzahl = static_cast<int>((100.0 / static_cast<double>(xVec.size()))  * static_cast<double>(consensAnzahl));

				maxConsens = consensAnzahl;  //auch hier Prozentangabe !!!

				m_oMaxConsens = maxConsens;

				m_oBestCoeff = m_oCoeff; // In one fit are the coefficient the best coefficients
                m_oMin = min;
                m_oMax = max;

                if(m_oType==ePoly)
                {
				    std::stringstream instream;
				    instream<<"Fehler des Poly Fit: "<<error<< ",Konsens: "<<m_oMaxConsens<<std::endl;
				    std::string outstring(instream.str());
				    precitec::wmLog(precitec::LogType::eInfo, "Eine Iterartion, %s",outstring.c_str());
                }
                if(m_oType==eSine)
                {
                    std::stringstream instream;
                	instream<<"Fehler des Sinus Fit: "<<error<< ",Konsens: "<<m_oMaxConsens<<std::endl;
                	std::string outstring(instream.str());
                	precitec::wmLog(precitec::LogType::eInfo, "Eine Iterartion, %s",outstring.c_str());
                }
                if(m_oType==eGauss)
                {
                	precitec::wmLog(precitec::LogType::eWarning, "Datenverarbeitung Gauss Fit nicht implementiert");
                }


			}
	    }
		else
		{

			double errorMin = 1000000.0;
			double error = 0.0;
			double min = 0.0;
			double max = 0.0;
			//Ransac Iterationen -- start im original vector verschieben
			//std::vector<double> oldFitted;
			for (int j = 0; j < ctr; j++)
			{
				xVec = m_dXVec;
				yVec = m_dYVec;

				offset = j*start;

				if ((offset< m_oDataLength - windowSize) && (m_oProcessingMode >= 1) && (offset<(windowSize + intervalDistance)) )  //if the offset gets higher the result gets the same
				{
					//Verkuerze Datensatz :
					//eraseNumbers(std::vector<double> &dataVec,int windowSize, int intervalDistance, int start)
					eraseNumbers(xVec, windowSize, intervalDistance, offset);
					eraseNumbers(yVec, windowSize, intervalDistance, offset);
					//std::cout<<"buffer size after eraseNumbers-- "<<xVec.size()<<std::endl;
				}
				else
				{
					j = ctr;
				}
				if (yVec.size() > 0 && xVec.size() > 0)
				{
					if(m_oType==ePoly)
					{
						m_oCoeff = fitNormalEquationPoly(xVec, yVec);
					    m_oFittedY = polyval(m_oCoeff, xVec);
					}
					else if(m_oType==eSine)
					{

						   m_oCoeff    = fitSine(xVec,yVec,m_oPeriod,m_oOffsetStart,m_oAmplitudeStart,m_oHoldCoeff);
						   if( m_oCoeff.size()>3)
						   {
						       m_oFittedY =  calculateSinus(xVec, m_oCoeff, min, max);
						   }
						   else
						   {
							   precitec::wmLog(precitec::LogType::eWarning, "Keine Sinus Koeffizienten berechnet");
						   }
                            /*
						   	ofstream file;
						   	file.open("results1.txt",ios::app);
						   	int k=0;
		   					for(int m=0;m<xVec.size();m++)
		   					{
		   					    file << xVec[m] << ";" << m_oFittedY[m] << std::endl;
		   						 k++;
		   					}
		   					file.close();
		   					std::cout <<" "<<k<<" Werte nach fit gespeichert"<<std::endl;
                            */


					}

					consensAnzahl = 0;
					error = calculateError(m_oFittedY, yVec,consensAnzahl, m_oError);
					if (error < errorMin)
						errorMin = error;


					//besser in prozent auf den Datensatz
					if ((consensAnzahl >= 0) && (xVec.size()>0))
					    consensAnzahl =  static_cast<int>(  (100.0/static_cast<double>(xVec.size()) )  * static_cast<double>(consensAnzahl) );

					//merken des max. Konses und der damit verbundenen Koeffizienten
					if (consensAnzahl > maxConsens)
					{
						maxConsens = consensAnzahl;
						m_oBestCoeff = m_oCoeff;    //best parameterset
					}

				} //yVec.size()
				//std::cout<<"Loop: "<<j<<", Error, Konsens, MaxKonsens: "<<error<<", "<<consensAnzahl<<", "<<maxConsens<<std::endl;

			}//for

			//Now: Consens in correlation to the original data:
			consensAnzahl = 0;
			if(m_oType==ePoly )
			{
				m_oFittedY = polyval(m_oBestCoeff, m_dXVec);
			}
			else if(m_oType==eSine)
			{
				m_oFittedY =  calculateSinus(m_dXVec, m_oBestCoeff, min, max);
			}

			calculateError(m_oFittedY, m_dYVec,consensAnzahl, m_oError);
			if ((consensAnzahl >= 0) && (xVec.size()>0))
			    consensAnzahl =  static_cast<int>(  (100.0/static_cast<double>(m_dXVec.size()) )  * static_cast<double>(consensAnzahl) );



            maxConsens=consensAnzahl;

			/*Testfile check data
			ofstream file;
			file.open("results2.txt",ios::app);
			int k=0;
			for(int m=0;m<xVec.size();m++)
			{
			    file << xVec[m] << ";" << m_oFittedY[m] << std::endl;
				k++;
			}
			file.close();
			std::cout <<" "<<k<<" Werte nach fit gespeichert"<<std::endl;
			*/


			m_oMaxConsens = maxConsens;

			std::stringstream instream2;
			if(m_oType==ePoly)
			{
			    instream2 <<ctr<< ", Kleinster Fehler des Poly Fit: " << errorMin << ",Konsens: " << m_oMaxConsens << std::endl;
			    std::string outstring2(instream2.str());
			    precitec::wmLog(precitec::LogType::eInfo, "Verarbeitung ueber mehrere Iterationen: %s", outstring2.c_str());
			}
			else if(m_oType==eSine)
			{
				instream2 <<ctr<< ", Kleinster Fehler des Sinus Fit: " << errorMin << ",Konsens: " << m_oMaxConsens << std::endl;
				std::string outstring2(instream2.str());
			    precitec::wmLog(precitec::LogType::eInfo, "Verarbeitung ueber mehrere Iterationen: %s", outstring2.c_str());
			}
			else if(m_oType ==eGauss)
			{
				precitec::wmLog(precitec::LogType::eWarning, "Verarbeitung Gauss Fit nicht implementiert");

			}

		}//else


	}//m_oDataLength

}



/*
* Mit den in den Ransac Iterationen gefundenen Polynomkoeffizienten
* werden in den Originaldaten die Ausreisser entfernt
*
*/
void Model::processCleanSignal()
{

	//std::stringstream strs, strs2, strs3;
	//QString QStr;
	double meanValue = 0.0;
	double min = 0.0;
	double max = 0.0;

	 m_oOutlierCounter = 0;
	if (m_oBestCoeff.size()> 0)
	{
		//m_oError
		//rechne Kurve
		if(m_oType==ePoly)
		{

		    m_oFittedY = polyval(m_oBestCoeff,m_dXVec);
		}
		else if (m_oType==eSine)
		{
		    m_oFittedY = calculateSinus(m_dXVec, m_oBestCoeff, min, max);
		}
		else if (m_oType==eGauss)
		{
			precitec::wmLog(precitec::LogType::eWarning, "Gauss Fit not implemented");
		}

		if(m_oIterationNumber) // makes only sense if there was a RANSAC processing -- sense ??
		{
		    if( m_oFittedY.size()>0 )
		    {
		        //Ausreisser im Signal werden durch die gefitteten Werte ersetzt
		        m_oOutlierCounter = cleanSignal(m_oFittedY, m_dYVec, m_dXVec, m_oError);
		        meanValue = calcMinMaxMean(m_oFittedY, m_dXVec, min, max);

		        m_oMeanValue = meanValue;
		        m_oMin = min;
		        m_oMax = max;
		    }
		}

	}

}


/*
* Mit dem besten fit nach den Iterationen werden im Origianldatensatz die Ausreisser enfernt
* vecFitted: best fit
* yVec: Originaldaten
* threshold: Fehlergrenze
* start: start im Datensatz bei dem mit dem entfernen der Daten begonnen wird
* return: Anzahl der Ausreisser
*/
int Model::cleanSignal(std::vector<double>& vecFitted, std::vector<double>& yVec, std::vector<double>& xVec, double threshold)
{

	int number = 0;
	for (unsigned int i = 0; i<xVec.size(); ++i)
	{
		if (std::abs(yVec[i] - vecFitted[i]) > threshold)
		{
			yVec[i] = vecFitted[i];
			number++;
		}
	}

	//nochmal fitten - jetzt auch mit NE Verfahren
	//m_oCoeff   = m_processData.fitData( xVec, yVec,m_oGrad);

	if(m_oType==ePoly)
	{
	    m_oCoeff = fitNormalEquationPoly(xVec, yVec);
	    m_oFittedY = polyval(m_oCoeff, xVec);
	}
	else if(m_oType==eSine)
	{
		double min=0.0;
		double max=0.0;
		m_oCoeff    = fitSine(xVec,yVec,m_oPeriod,m_oOffsetStart,m_oAmplitudeStart,m_oHoldCoeff);
		m_oFittedY =  calculateSinus(xVec, m_oCoeff, min, max);

	}
	else if(m_oType==eGauss)
	{
		std::cout<<"Gauss not implemented.."<<std::endl;
		precitec::wmLog(precitec::LogType::eWarning, "Gauss Fit Clean Signal not implemented");
	}

	return number;

}


/*
* Berechen min. max und Mittelwert eines Signals
* vecFitted: best fit
* yVec: Originaldaten
* threshold: Fehlergrenze
* start: start im Datensatz bei dem mit dem Entfernen der Daten begonnen wird
* return: Anzahl der Ausreisser
*/
double Model::calcMinMaxMean(std::vector<double> &m_oYVec, std::vector<double> &m_oXVec, double &min, double &max)
{
	double mean = 0.0;
	min = 1000000000.0;
	max = -1000000000.0;
	unsigned int i = 0;

	for (i = 0; i<m_oYVec.size(); ++i)
	{
		if (m_oYVec[i]<min)
			min = m_oYVec[i];
		if (m_oYVec[i]>max)
			max = m_oYVec[i];

		mean += m_oYVec[i];
	}

	if(i>0)
	{
	    mean /= static_cast<double>(i);
	}
	else
		mean = 0; //sinnvoll

	return mean;
}

std::vector<double> Model::getCurve()
{
	if (m_oFittedY.size() > 0)
	{
		return(m_oFittedY);
	}
	else
	{
		//std::cout<<"------------curve empty--------------"<<std::endl;
		m_oFittedY.clear();
	}
		return m_oFittedY;
}


double Model::getMin()
{
	return m_oMin;
}

double Model::getMax()
{
	return m_oMax;
}

double Model::getOutlier()
{
	return m_oOutlierCounter;
}


double Model::getMean()
{
	return m_oMeanValue;
}

double Model::getKoeff1()
{
	if (m_oCoeff.size() > 2)
		return  static_cast<double>(m_oCoeff[0]);
	else
		return 0.0;
}

double Model::getKoeff2()
{
	if (m_oCoeff.size() > 2)
		return static_cast<double>(m_oCoeff[1]);
	else
		return 0.0;
}

double Model::getKoeff3()
{
	if (m_oCoeff.size() > 3)
		return static_cast<double>(m_oCoeff[2]);
	else
		return 0.0;
}

std::vector<double> Model::getCoefficients()
{
	return m_oCoeff;
}


double Model::getConsens()
{
	return m_oMaxConsens;
}


double Model::getError()
{
	return m_oBestError;
}

double Model::getCost()
{

	return m_oBestCost;
}



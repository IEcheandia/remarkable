/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		f.agrawal, SB, HS
 * 	@date		2010
 * 	@brief		Triggers an automatic cycle. Simulates hardware signals.
 */

#ifndef AutoRunner_H_
#define AutoRunner_H_
#include "Poco/Thread.h"
#include "Poco/Timespan.h"
#include "Poco/Activity.h"
#include "viInspectionControl/VI_InspectionControl.h"
#include <vector>


namespace precitec {
namespace viInspectionControl {

    
using Poco::Activity;
using Poco::Thread;


	class AutoRunner
	{
	public:
		AutoRunner(VI_InspectionControl & _control, std::string _strBasepath, std::vector<SequenceType> & _sequencesList)
		: control(_control),
			m_oSequences(_sequencesList),
			_activity(this, &AutoRunner::run),
			_autoRunnerMode(0),
			_runTime(1000),
			_breakTime(1000),
			_seamBreakTime(500),
			_loopCount(1),
			_sequenceFolder(_strBasepath)
		{
			_controlFirstSeamSeries = control.m_oSeamseries;
			_controlFirstSeam = control.m_oSeamNr;
		}

		~AutoRunner()
		{
		}

		Activity<AutoRunner>& activity()
		{
			return _activity;
		}

		int getLoopCount() const
		{
			return _loopCount;
		}

		void setLoopCount(int newValue )
		{
			if( newValue >= 0 )
			{
				_loopCount=newValue;
			}
		}

		int getRunTime() const
		{
			return _runTime;
		}

		void setRunTime(int newValue )
		{
			if( newValue >= 0 )
			{
				_runTime=newValue;
			}
		}

		int getBreakTime() const
		{
			return _breakTime;
		}

		void setBreakTime(int newValue )
		{
			if( newValue >= 0 )
			{
				_breakTime=newValue;
			}
		}

		void setSeamBreakTime(int newValue)
		{
			if (newValue >= 0)
			{
				_seamBreakTime = newValue;
			}
		}

		int getSeamBreakTime() const
		{
			return _seamBreakTime;
		}
		
		void setSequenceFolder(std::string const newString)
		{
			//if (newValue >= 0)   // Check folder in HDisk
			//{
				_sequenceFolder = newString;
			//}
		}

		std::string getSequenceFolder() const
		{
			return _sequenceFolder;
		}

		void setAutoRunnerMode(int newValue)
		{
			if (newValue >= 0)
			{
				_autoRunnerMode = newValue;
			}
		}

		int getAutoRunnerMode() const
		{
			return _autoRunnerMode;
		}
		void adjustStartSeam()
		{
			_controlFirstSeam = control.m_oSeamNr;
		}

		void adjustStartSeamSeries()
		{
			_controlFirstSeamSeries = control.m_oSeamseries;
		}
		
		void adjustSequenceList()
        {
            
        }

	protected:

		void run()
		{
			int loopcnt=0;
			int oProductStart = control.m_oProductType;

			while (!_activity.isStopped())
			{
				// did the user interrupt the activity?
				if(_activity.isStopped())
					break;

				// are we done?
				loopcnt++;
				if(loopcnt>=_loopCount)
				{
					_activity.stop();
				}
                
                Thread::sleep( 20 );
                std::cout << "AutoRunner: Loop: " << loopcnt << " of " << _loopCount << std::endl;
				
                if (_autoRunnerMode == 0)
                {
                    // start automatic mode
                    wmLog( eDebug, "AutoRunner: startAutomatic() - ProductType: %i, ProductNumber: %i\n", control.m_oProductType, control.m_oProductNumber);
                    control.m_oChangeToStandardMode = false;
                    control.TriggerAutomatic(true, control.m_oProductType, control.m_oProductNumber, "no info");
                    Thread::sleep( 20 );

                    // wait for the ack to arrive (timeout after 5sec)
                    int oCounter = 100;
                    while ( !control.inspectCycleAckn() && oCounter-- > 0 )
                    {
                        Thread::sleep( 50 );
                    }

                    // if the acknowledge did not arrive, we have to inform the user and we cannot cycle through the seam series and seams ...
                    if ( oCounter <= 0 )
                    {
                        wmLog( eDebug, "AutoRunner: startAuto - cycle acknowledge was never send by the workflow ...\n" );
                    }
                    else
                    {
                        autoRun();
                    }

                    // stop automatic mode
                    wmLog( eDebug, "AutoRunner: stopAutomatic().\n" );
                    control.TriggerAutomatic(false, control.m_oProductType, control.m_oProductNumber, "no info");
                    Thread::sleep( 20 );

                    // increase product type - loops from the initial product type until it was increased n times and then it is set again to the initial type ...
                    if ( control.m_oProductType < (oProductStart + control.m_oNoProducts - 1) )
                        control.m_oProductType++;
                    else
                        control.m_oProductType = oProductStart;

                    // break time between cycles
                    wmLog( eDebug, "AutoRunner: break time: %i\n", _breakTime );
                    Thread::sleep(_breakTime);
               }
                else
                {
                    bool saveProductNumberFromUserEnable = control.m_oProductNumberFromUserEnable;
                    control.m_oProductNumberFromUserEnable = true;
                    control.m_oProductNumber = m_oSequences[0].products[0].productNumber;
                    control.m_oProductType = oProductStart;

                    autoRunCalculated();
                    
                    control.m_oProductNumberFromUserEnable = saveProductNumberFromUserEnable;
                    
                }
			} // while

			control.m_oProductType = oProductStart; //< Restore intial product type.

		} // run
		
		void autoRun()
        {
			// loop through the seam series
			for ( unsigned int _seamSeriesCount = 0; _seamSeriesCount < control.m_oNoSeamSeries; ++_seamSeriesCount)
			{
				control.m_oSeamseries = _controlFirstSeamSeries + _seamSeriesCount; // set seam series to be processed

				control.TriggerInspectInfo(true, control.m_oSeamseries);
				Thread::sleep( 20 );

				// loop through the seams
				for ( unsigned int _seamCount = 0; _seamCount < control.m_oNoSeams; ++_seamCount)
				{
					control.m_oSeamNr = _controlFirstSeam + _seamCount; // set seam to be processed

					// start inspect
					wmLog( eDebug, "AutoRunner: startInspect (seam series: %i, seam: %i)\n", _seamSeriesCount, control.m_oSeamNr );
					control.TriggerInspectInfo(true, control.m_oSeamseries);
					Thread::sleep( 20 );
					control.TriggerInspectStartStop(true, control.m_oSeamNr);
					Thread::sleep( 20 );
                    Thread::sleep(_runTime);

					// stop inspect
					wmLog( eDebug, "AutoRunner: stopInspect (seam series: %i, seam: %i)\n", _seamSeriesCount, control.m_oSeamNr );
					control.TriggerInspectStartStop(false, control.m_oSeamNr);
					Thread::sleep( 20 );
					control.TriggerInspectInfo(false, control.m_oSeamseries);
					Thread::sleep( 20 );

					// in case of multiple seams, wait a bit ...
					if ( (_seamCount < (control.m_oNoSeams-1)) && (control.m_oNoSeams > 1) )
					{
						wmLog( eDebug, "AutoRunner: seam break time: %i\n", _breakTime );
						Thread::sleep(_seamBreakTime);
					}
				} // for
				control.TriggerInspectInfo(false, control.m_oSeamseries);
				Thread::sleep( 20 );

				// in case of multiple seamsSeries, wait a bit ...
				if ( (_seamSeriesCount < (control.m_oNoSeamSeries-1)) && (control.m_oNoSeamSeries > 1) )
				{
					//printf("AutoRunner: seam break time: %d\n", _breakTime );
					Thread::sleep(_seamBreakTime);
				}

				control.m_oSeamNr = _controlFirstSeam;
			} // for
			control.m_oSeamseries = _controlFirstSeamSeries;
        }

		void autoRunCalculated()
        {
			// loop through the sequences
			for(std::size_t i_Seq = 0; i_Seq < m_oSequences.size(); ++i_Seq)
            {
                wmLog( eDebug, "AutoRunner: startSequence: %i \n", i_Seq);
                int oProductStart = control.m_oProductType;
                // loop through the Products
                for(std::size_t i_Prod = 0; i_Prod <  m_oSequences[i_Seq].products.size(); ++i_Prod)
                {
                    control.m_oProductNumber = m_oSequences[i_Seq].products[i_Prod].productNumber;
                    wmLog( eDebug, "AutoRunner: startProduct: %i %i\n", i_Prod, m_oSequences[i_Seq].products[i_Prod].productNumber);
                
                    // start automatic mode
                    wmLog( eDebug, "AutoRunner: startAutomatic() - ProductType: %i, ProductNumber: %i\n", control.m_oProductType, control.m_oProductNumber);
                    control.m_oChangeToStandardMode = false;
                    control.TriggerAutomatic(true, control.m_oProductType, control.m_oProductNumber, "no info");
                    Thread::sleep( 20 );

                                         // wait for the ack to arrive (timeout after 5sec)
                    int oCounter = 100;
                    while ( !control.inspectCycleAckn() && oCounter-- > 0 )
                    {
                        Thread::sleep( 50 );
                    }

                    // if the acknowledge did not arrive, we have to inform the user and we cannot cycle through the seam series and seams ...
                    if ( oCounter <= 0 )
                    {
                        wmLog( eDebug, "AutoRunner: startAuto - cycle acknowledge was never send by the workflow ...\n" );
                        break;
                    }
                    
                    // loop through the seam series
                    std::size_t countOfSeamSeries = m_oSequences[i_Seq].products[i_Prod].seamSeries.size();
                    for(std::size_t i_Series = 0; i_Series <  countOfSeamSeries; ++i_Series)
                    {
                        control.m_oSeamseries = m_oSequences[i_Seq].products[i_Prod].seamSeries[i_Series].seamSerieNumber; // set seam series to be processed

                        control.TriggerInspectInfo(true, control.m_oSeamseries);
                        Thread::sleep( 20 );

                        // loop through the seams
                        std::size_t countOfSeams = m_oSequences[i_Seq].products[i_Prod].seamSeries[i_Series].seams.size();
                        for(std::size_t i_Seam = 0; i_Seam <  countOfSeams; ++i_Seam) 
                        {
                            control.m_oSeamNr = m_oSequences[i_Seq].products[i_Prod].seamSeries[i_Series].seams[i_Seam].seamNumber; // set seam to be processed
                            int iImageCount= m_oSequences[i_Seq].products[i_Prod].seamSeries[i_Series].seams[i_Seam].imageCount;
                            
                            // start inspect
                            wmLog( eDebug, "AutoRunner: startInspect (seam series: %i, seam: %i, images %i)\n", control.m_oSeamseries, control.m_oSeamNr, iImageCount  );
                            
                            control.TriggerInspectInfo(true, control.m_oSeamseries);
                            Thread::sleep( 20 );
                            control.TriggerInspectStartStop(true, control.m_oSeamNr);
                            Thread::sleep( 20 );
                            
                            int runTimeCalculated =  1000;
                            bool timeoutError = false;
                            system::ElapsedTimer m_oTimer;
                            while (control.getTriggerDistNanoSecs() == 0)
                            {
                                
                                if ((long)m_oTimer.elapsed().count() > 2000)
                                {
                                    timeoutError = true;
                                    break;
                                }
                            }
                            const long elapsed = (long)m_oTimer.elapsed().count();
                            if (timeoutError )
                            {
                                wmLog( eDebug, "AutoRunner: Timeout - get no Trigger distance of seam: %i\n",  control.m_oSeamNr);
                            }
                            else
                            {
                                runTimeCalculated =  ( ( (iImageCount-1) * control.getTriggerDistNanoSecs() )- elapsed) / 1000000;
                            }
                            //wmLog( eDebug, "AutoRunner: Trigger Dist(nsec): %i , elapsed time(nsec): %i, runTimer(msec) %i\n", control.getTriggerDistNanoSecs(), elapsed, runTimeCalculated );
                            Thread::sleep(runTimeCalculated);
                            // stop inspect
                            control.TriggerInspectStartStop(false, control.m_oSeamNr);
                            const long runTimeMessured = (int)(m_oTimer.elapsed().count() / 1000000);
                            wmLog( eDebug, "AutoRunner: stopInspect (seam series: %i, seam: %i, runTimer(msec) %i)\n", control.m_oSeamseries, control.m_oSeamNr, runTimeMessured );
                            Thread::sleep( 20 );
                            control.TriggerInspectInfo(false, control.m_oSeamseries);
                            Thread::sleep( 20 );
                            control.setTriggerDistNanoSecs(0);
                            // in case of multiple seams, wait a bit ...
                            if ( (countOfSeams  > 1) )
                            {
                                wmLog( eDebug, "AutoRunner: seam break time: %i\n", _seamBreakTime );
                                Thread::sleep(_seamBreakTime);
                            }
                        } // for seams
                        control.TriggerInspectInfo(false, control.m_oSeamseries);
                        Thread::sleep( 20 );

                        // in case of multiple seamsSeries, wait a bit ...
                        if ( (countOfSeamSeries > 1) )
                        {
                            wmLog( eDebug, "AutoRunner: seam series break time: %i\n", _breakTime );
                            Thread::sleep(_breakTime);
                        }

                        control.m_oSeamNr = _controlFirstSeam;
                    } // for seam series
                    control.m_oSeamseries = _controlFirstSeamSeries;
                    
                    // stop automatic mode
                    wmLog( eDebug, "AutoRunner: stopAutomatic().\n" );
                    control.TriggerAutomatic(false, control.m_oProductType, control.m_oProductNumber, "no info");
                    Thread::sleep( 20 );
                    
                    // increase product type - loops from the initial product type until it was increased n times and then it is set again to the initial type ...
                    if ( control.m_oProductType < (oProductStart + control.m_oNoProducts - 1) )
                        control.m_oProductType++;
                    else
                        control.m_oProductType = oProductStart;

                    // break time between cycles
                    wmLog( eDebug, "AutoRunner: break time: %i\n", _breakTime );
                    Thread::sleep(_breakTime);

                } // for product
            } // for sequence
        }

        
	private:
		VI_InspectionControl & control;
        std::vector<SequenceType> & m_oSequences;
        Activity<AutoRunner> _activity;
        int                    _autoRunnerMode;
		int                    _runTime;
		int                    _breakTime;
		int                    _seamBreakTime;
		int                    _loopCount;
		int                    _controlFirstSeamSeries;  // memory for control.m_oSeamseries
		int                    _controlFirstSeam;        // memory for control.m_oSeamNr
        std::string            _sequenceFolder;
        
	};

}}

#endif /*AutoRunner_H_*/

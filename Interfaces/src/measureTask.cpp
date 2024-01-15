#include "system/types.h"
#include "common/measureTask.h"

#include "module/moduleLogger.h"

namespace precitec
{
	namespace interface
	{
		int MeasureTask::lastImage() const
		{
			return (nrTriggers() - 1);
		}


		/*
		 * If p_oLength is given, nrTrigger & getRealNrTriggers measure the number of trigger given length p_oLength and the tasks triggerDelta m_oTriggerDelta.
		 * The idea is, to compute the number of triggers not for a seaminterval task but for a complete seam summed over the length of all tasks.
		 * InspectManager::startInspect uses this to compute the correct number of triggers for a complete seam.
		 */
		int MeasureTask::nrTriggers(int p_oLength) const
		{
			int p_oCurImgNr = 1;
			bool completeSeam = true;

			int oNrTriggers = (int)( getRealNrTriggers(p_oLength) + ((p_oLength % m_oTriggerDelta) > 0) ); // adjust #triggers when length is not dividable by m_oTriggerDelta with zero remainder
			if (p_oLength < 0)
			{
				p_oLength = m_oLength;
				completeSeam = false;
			}

			// int oNrTriggers = (int)(1.0*p_oLength / m_oTriggerDelta ) + ((p_oLength % m_oTriggerDelta) > 0);
			if ( (m_oSeaminterval == 0) || completeSeam )
			{
				if (p_oCurImgNr > 0) // only for very first seam!
				{
					++oNrTriggers;
				}
			}
			return oNrTriggers;
		}

		double MeasureTask::getRealNrTriggers(int p_oLength) const
		{
			//return (double)(nrTriggers(p_oLength));

			if (p_oLength < 0)
			{
				p_oLength = m_oLength;
			}
			if(m_oTriggerDelta == 0) {
				wmLog(eWarning, "Trigger delta is zero.");
				return 0;
			} // if
			return (double)(1.0*p_oLength / m_oTriggerDelta );
		}

		/// Returns (time) triggerdelta [ms]. Returns zero and Log fatal if (for instance when velocity is zero) and negative timeDelta if triggerDelta is a float!
		double MeasureTask::getTimeDelta() const
		{
			if (m_oVelocity <= 0)
			{
				wmLogTr( eError, "QnxMsg.Workflow.MTVelNeg",
						"Seamseries %i, seam %i: Velocity must not be negative!", m_oSeamseries+1, m_oSeam+1);

				return 0;
			}

			long msDelta = (m_oTriggerDelta * 1000);  // um -> nm
			if ( msDelta < m_oVelocity )
			{
				wmLogTr( eError, "QnxMsg.Workflow.MTVelTooHigh",
						"Seamseries %i, seam %i: Velocity too high or trigger distance too small!", m_oSeamseries+1, m_oSeam+1);

				return 0;
			}
			return double( msDelta ) / m_oVelocity; //nm/(um/s)=ms
		}



		auto MeasureTask::setMissingParameters() -> eTaskError
		{
			m_oParamsOK = false;
			if (m_oTriggerDelta == 0)
			{
				return MeasureTask::ErrorTriggerDeltaMissing;
			}
			/* Length 0 stupid but possible and allowed as of 2012/10
			if (m_oLength == 0)
			{
				return MeasureTask::ErrorLengthMissing;
			}
			*/
			if (m_oVelocity == 0) // velocity is necessary for the trigger frequency when bursting the grabber
			{
				return MeasureTask::ErrorVelocityMissing;
			}

			if (m_oNbTriggers > 0) // live mode, where number of triggers is given by windows
			{
				m_oParamsOK = true;
			} else
			{
				m_oNbTriggers = (int)( m_oLength / m_oTriggerDelta );
			}
			return MeasureTask::ParametersOK;
		}

	} // namespaces
} // namesaces




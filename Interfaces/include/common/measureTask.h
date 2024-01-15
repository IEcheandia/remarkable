/**
 * @file
 * @brief  The measureTask object offers the basic data structure for measure tasks such as
 * seamseries, seam, trigger information and relates tasks for images to graphs in use. As we do NOT allow concurrent
 * and overlapping tasks (as of 2012/02), a measure task should cover exaclty one seam interval and each seam interval should
 * have an associate measure task!
 * The measureTask object now also includes all information necessary for multi image errors
 *
 * @author KIR, AB
 * @date   extended and revised, 02/2012 and 03/2012
 */

#ifndef MEASURETASK_H_
#define MEASURETASK_H_

#include <utility>
#include <string>
#include <vector>
#include <iostream>
#include <tuple>
#include <cstdint>
#include "geo/range.h"
#include "Poco/Version.h"
#include "Poco/SharedPtr.h"
#include "Poco/UUID.h"
#include "system/types.h"
#include "system/exception.h"
#include "system/templates.h"
#include "message/messageBuffer.h"
#include "message/serializer.h"
#include "InterfacesManifest.h"

namespace precitec
{
namespace interface
{

/* WICHTIG!!!! Im weiteren, wenn es mal andere Sensoren als nur die Kamera gibt, muss der Sensorwert unbedingt eine Triggernummer
 * mit erhalten. Die Fehlerlogik (Produkt/GLoabLFilter) muss Zugriff sowohl auf Triggernummer (oder Zeitstempel oder was auch immer
 * dies sein wird) als auch auf die Messaufgabe haben, damit sie aus diesen Daten ggf. Ortsinformationen berechnen kann und nicht
 * gezwungen ist, mit einer Fehlerauswertung u.U. unnoetigerweise bis ans Ende einer Naht zu warten, respektive bis zum naechsten
 * signalizeStop.
 * Jetzt wird das erst mal die Bildnummer des ImageContextes des NIOs sein...
 */


	class TaskSequence;

	class INTERFACES_API MeasureTask  : public system::message::Serializable
	{
		public:
			enum eTaskError { ParametersOK = 0, ErrorVelocityMissing, ErrorLengthMissing, ErrorTriggerDeltaMissing, ErrorNoNrTriggers, ErrorBadTrigger,
				WarningInaccurateTriggerDelta, WarningInaccurateNrTriggers, NumErrors };

			MeasureTask (
					const Poco::UUID&	p_rTaskID				= Poco::UUID(), 
					const Poco::UUID&	p_rGraphID				= Poco::UUID(), 
					int					p_oSeamseries			= 0, 
					int					p_oSeam					= 0, 
					int					p_oSeaminterval			= 0, 
					int					p_oLevel				= 0, 
					int					p_oTriggerDelta			= 0, 
					int					p_oLength				= 0, 
					int					p_oVelocity				= 0, 
					int					p_oStart			= 0, 
					int					p_oNbTriggers			= 0, 
					const PvString&		p_oGraphName			= " - ",
					const PvString&		p_oName					= " - ", 
					const Poco::UUID&	p_rParametersatzID		= Poco::UUID(), 
					const Poco::UUID&	p_rHwParametersatzID	= Poco::UUID(), 
                    int                 p_oDirection            = 0,           ///< approach dircetion from upper/lower
                    int                 p_oThicknessLeft        = 0,           ///< [um] Thinckness left of blank
                    int                 p_oThicknessRight       = 0,           ///< [um] Thinckness right of blank
                    int                 p_oTargetDifference     = 0,
                    int                 p_oRoiX                 = 0,
                    int                 p_oRoiY                 = 0,
                    int                 p_oRoiW                 = 0,
                    int                 p_oRoiH                 = 0,
                    int                 p_oIntervalLevel        = 0,
					int					p_oNbSeamTriggers		= 0, 
					bool				p_oParamsOK				= 0, 
					bool				p_oHasAdditionalTrigger	= false
                      )
			:
				m_oTaskID				(p_rTaskID), 
				m_oGraphID				(p_rGraphID), 
				m_oSeamseries			(p_oSeamseries), 
				m_oSeam					(p_oSeam), 
				m_oSeaminterval			(p_oSeaminterval), 
				m_oLevel				(p_oLevel),
				m_oTriggerDelta			(p_oTriggerDelta), 
				m_oLength				(p_oLength), 
				m_oVelocity				(p_oVelocity), 
				m_oStart				(p_oStart), 
				m_oNbTriggers			(p_oNbTriggers), 
				m_oGraphName			(p_oGraphName), 
				m_oName					(p_oName), 
				m_oParametersatzID		(p_rParametersatzID),
				m_oHwParametersatzID	(p_rHwParametersatzID),
                m_oDirection            (p_oDirection),           ///< approach dircetion from upper/lower
                m_oThicknessLeft        (p_oThicknessLeft),       ///< [um] Thinckness left of blank
                m_oThicknessRight       (p_oThicknessRight),      ///< [um] Thinckness right of blank
                m_oTargetDifference     (p_oTargetDifference),
                m_oRoiX                 (p_oRoiX),
                m_oRoiY                 (p_oRoiY),
                m_oRoiW                 (p_oRoiW),
                m_oRoiH                 (p_oRoiH),
                m_oIntervalLevel        (p_oIntervalLevel),
				m_oNbSeamTriggers		(p_oNbSeamTriggers),
				m_oParamsOK				(p_oParamsOK),
				m_oHasAdditionalTrigger	(p_oHasAdditionalTrigger)
			{
			}

			/// Copy constructor
			//MeasureTask(const MeasureTask &rhs) = default;   // VS 2010 does not yet understand this C++11 declaration...

			const Poco::UUID&  taskID()             const { return m_oTaskID; }
			const Poco::UUID&  graphID()            const { return m_oGraphID; }
			int                seamseries()         const { return m_oSeamseries; }
			int                seam()               const { return m_oSeam; }
			int                seaminterval()       const { return m_oSeaminterval; }
			int                level()              const { return m_oLevel; }
			int                triggerDelta()       const { return m_oTriggerDelta; }  // [um]
			int                length()             const { return m_oLength; }        // [um]
			int                velocity()           const { return m_oVelocity; }      // [mm/s]
			int                start()         const { return m_oStart; }
			bool               parametersOK()       const { return m_oParamsOK; }
			int                nrSeamTriggers()     const { return m_oNbSeamTriggers; }

			PvString           graphName()          const { return m_oGraphName; }
			PvString           name()               const { return m_oName; }
			const Poco::UUID&  parametersatzID()    const { return m_oParametersatzID; }
			const Poco::UUID&  hwParametersatzID()  const { return m_oHwParametersatzID; }
			int                movingDirection()    const { return m_oDirection; }            // [um]
			int                thicknessLeft()      const { return m_oThicknessLeft; }        // [um]
			int                thicknessRight()     const { return m_oThicknessRight; }       // [um]
			int                targetDifference()   const { return m_oTargetDifference; }      // [um]
			int                roiX()               const { return m_oRoiX; }
            int                roiY()               const { return m_oRoiY; }
            int                roiW()               const { return m_oRoiW; }
            int                roiH()               const { return m_oRoiH; }
            int                intervalLevel()      const { return m_oIntervalLevel; }



			int nrTriggers(int p_oLength=-1) const;
			double getRealNrTriggers(int p_oLength=-1) const;		///< returns real (not truncated) number of triggers
			int lastImage() const;									///< Returns nrTriggers() - 1
			double getTimeDelta() const;							///< triggerSpaceDelta -> triggerTimeDelta [ms].
			auto setMissingParameters() -> eTaskError;				///< Compute missing parameters from those given. Returns error in case of insufficient data.

			void setNrSeamTriggers(const int p_oNr) { m_oNbSeamTriggers = p_oNr; }
			void setTriggerDelta(const int delta) { m_oTriggerDelta = delta; }

		private:
			Poco::UUID	m_oTaskID;
			Poco::UUID	m_oGraphID;
			int			m_oSeamseries;			///< Seamseries
			int			m_oSeam;				///< Seam
			int			m_oSeaminterval;		///< Seaminterval (=Nahtbereich)
			int			m_oLevel;				///< Level of measuretask (0=seamseries, 1=seam, 2=seam interval; should always be 2!!!). can be deleted
			int			m_oTriggerDelta;		///< space delta between two successive triggers [um]
			int			m_oLength;   			///< Length of seam interval (internally [um], user level [mm]), negative = infinite
			int			m_oVelocity;			///< Welding/ cutting velocity (internally [um/s], user level [mm/s])
			int			m_oStart;			///< First image (overall start at 0) of seam interval
			int			m_oNbTriggers;			///< Number of triggers for active seam interval. < 0 for infinite (when we have infinite length...)
			PvString	m_oGraphName;
			PvString	m_oName;				///< Name of measure task
			Poco::UUID	m_oParametersatzID;		/// ID des Parametersatzes
			Poco::UUID	m_oHwParametersatzID;	/// ID des HW-Parametersatzes

            int         m_oDirection;           ///< approach dircetion from upper/lower
            int         m_oThicknessLeft;       ///< [um] Thinckness left of blank
            int         m_oThicknessRight;      ///< [um] Thinckness right of blank
            int         m_oTargetDifference;
            int         m_oRoiX;
            int         m_oRoiY;
            int         m_oRoiW;
            int         m_oRoiH;
            int         m_oIntervalLevel;

			// need not to be messaged
	
			int		m_oNbSeamTriggers;			/// sum of trigger of seaminterval for one seam
			bool	m_oParamsOK;				///< all params given/ set missing parameters executed successfully? Default is false.
			bool	m_oHasAdditionalTrigger;	/// seamlength not integer dividible by triggerdistance


		public:
			virtual void serialize ( system::message::MessageBuffer &buffer ) const
			{
				marshal(buffer, m_oTaskID);
				marshal(buffer, m_oGraphID);
				marshal(buffer, m_oSeamseries);
				marshal(buffer, m_oSeam);
				marshal(buffer, m_oSeaminterval);
				marshal(buffer, m_oLevel);
				marshal(buffer, m_oTriggerDelta);
				marshal(buffer, m_oLength);
				marshal(buffer, m_oVelocity);
				marshal(buffer, m_oStart);
				marshal(buffer, m_oNbTriggers);
				marshal(buffer, m_oGraphName);
				marshal(buffer, m_oName);
				marshal(buffer, m_oParamsOK);
				marshal(buffer, m_oParametersatzID);
				marshal(buffer, m_oHwParametersatzID);
				marshal(buffer, m_oDirection);
				marshal(buffer, m_oThicknessLeft);
				marshal(buffer, m_oThicknessRight);
				marshal(buffer, m_oTargetDifference);
                marshal(buffer, m_oRoiX);
                marshal(buffer, m_oRoiY);
                marshal(buffer, m_oRoiW);
                marshal(buffer, m_oRoiH);
                marshal(buffer, m_oIntervalLevel);
			}

			virtual void deserialize( system::message::MessageBuffer const&buffer )
			{
				deMarshal(buffer, m_oTaskID);
				deMarshal(buffer, m_oGraphID);
				deMarshal(buffer, m_oSeamseries);
				deMarshal(buffer, m_oSeam);
				deMarshal(buffer, m_oSeaminterval);
				deMarshal(buffer, m_oLevel);
				deMarshal(buffer, m_oTriggerDelta);
				deMarshal(buffer, m_oLength);
				deMarshal(buffer, m_oVelocity);
				deMarshal(buffer, m_oStart);
				deMarshal(buffer, m_oNbTriggers);
				deMarshal(buffer, m_oGraphName);
				deMarshal(buffer, m_oName);
				deMarshal(buffer, m_oParamsOK);
				deMarshal(buffer, m_oParametersatzID);
				deMarshal(buffer, m_oHwParametersatzID);
				deMarshal(buffer, m_oDirection);
				deMarshal(buffer, m_oThicknessLeft);
				deMarshal(buffer, m_oThicknessRight);
                deMarshal(buffer, m_oTargetDifference);
                deMarshal(buffer, m_oRoiX);
                deMarshal(buffer, m_oRoiY);
                deMarshal(buffer, m_oRoiW);
                deMarshal(buffer, m_oRoiH);
                deMarshal(buffer, m_oIntervalLevel);
			}

			inline void swap( MeasureTask &p_rFrom) {
				m_oTaskID.swap(p_rFrom.m_oTaskID);
				m_oGraphID.swap(p_rFrom.m_oGraphID);
				std::swap(m_oSeamseries, p_rFrom.m_oSeamseries);
				std::swap(m_oSeam, p_rFrom.m_oSeam);
				std::swap(m_oSeaminterval, p_rFrom.m_oSeaminterval);
				std::swap(m_oLevel, p_rFrom.m_oLevel);
				std::swap(m_oTriggerDelta, p_rFrom.m_oTriggerDelta);
				std::swap(m_oLength, p_rFrom.m_oLength);
				std::swap(m_oVelocity, p_rFrom.m_oVelocity);
				std::swap(m_oStart, p_rFrom.m_oStart);
				std::swap(m_oNbTriggers, p_rFrom.m_oNbTriggers);
				std::swap(m_oGraphName, p_rFrom.m_oGraphName);
				std::swap(m_oName, p_rFrom.m_oName);
				std::swap(m_oParamsOK, p_rFrom.m_oParamsOK);
				m_oParametersatzID.swap(p_rFrom.m_oParametersatzID);
				m_oHwParametersatzID.swap(p_rFrom.m_oHwParametersatzID);
				std::swap(m_oDirection, p_rFrom.m_oDirection);
				std::swap(m_oThicknessLeft, p_rFrom.m_oThicknessLeft);
				std::swap(m_oThicknessRight, p_rFrom.m_oThicknessRight);
				std::swap(m_oTargetDifference, p_rFrom.m_oTargetDifference);
				std::swap(m_oRoiX, p_rFrom.m_oRoiX);
				std::swap(m_oRoiY, p_rFrom.m_oRoiY);
				std::swap(m_oRoiW, p_rFrom.m_oRoiW);
				std::swap(m_oRoiH, p_rFrom.m_oRoiH);
				std::swap(m_oIntervalLevel, p_rFrom.m_oIntervalLevel);
			}

			friend std::ostream& operator <<(std::ostream &p_rOstream, const MeasureTask& p_rMt)
			{
				p_rOstream << "MeasureTask\n===========\n" << std::endl;
				p_rOstream << "  seamseries: " << p_rMt.m_oSeamseries << "; seam: " << p_rMt.m_oSeam << "; seaminterval: " << p_rMt.m_oSeaminterval << "\n";
				p_rOstream << "  triggerdelta: " << p_rMt.m_oTriggerDelta << "[um]; length: " << p_rMt.m_oLength << "[um]; " << p_rMt.m_oVelocity << "[um/s]" << "\n";
				p_rOstream << "  nb of triggers: " << p_rMt.m_oNbTriggers << "; level: " << p_rMt.m_oLevel << "\n";
				p_rOstream << "  GraphName: " << p_rMt.m_oGraphName << "; ID: " << p_rMt.m_oGraphID.toString() << "\n";
                p_rOstream << "  moveDirection: " << p_rMt.m_oDirection << "\n";
                p_rOstream << "  thickness left: " << p_rMt.m_oThicknessLeft << "; thickness right: " << p_rMt.m_oThicknessRight<< "; target difference : " << p_rMt.m_oTargetDifference << "\n";
                p_rOstream << "  roi x: " << p_rMt.m_oRoiY << " y: "<< p_rMt.m_oRoiX << " width: " << p_rMt.m_oRoiW << "heihgt: "<< p_rMt.m_oRoiH;
                p_rOstream << "  interval level: " << p_rMt.m_oIntervalLevel << "\n";
				return p_rOstream;
			}

	};


	typedef std::vector<MeasureTask>								MeasureTaskList;
	typedef std::vector<MeasureTask*>								MeasureTaskPtrList;
	typedef Poco::SharedPtr<MeasureTask>							SmpMeasureTask;
	typedef std::tuple<std::int32_t, std::int32_t, std::int32_t>	tSeamIndex;	/// SumError map reference/ index.

	inline tSeamIndex	getSeamIndex(const MeasureTask& rMeasureTask) { return tSeamIndex(rMeasureTask.seamseries(), rMeasureTask.seam(), rMeasureTask.seaminterval()); }

}	// precitec
}	// interface


#endif /*MEASURETASK_H_*/


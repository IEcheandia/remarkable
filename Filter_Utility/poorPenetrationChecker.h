/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2016
* 	@brief		This filter checks if a poor penetration candidate is a real poor penetration
*/

#ifndef POORPENETRATIONCHECKER_H_
#define POORPENETRATIONCHECKER_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include <geo/geo.h>

#include "poorPenetrationPaint.h"
#include "overlay/overlayPrimitive.h"


namespace precitec
{
	namespace filter
	{

		class FILTER_API PoorPenetrationChecker : public fliplib::TransformFilter
		{
		public:
			/**
			* CTor.
			*/
			PoorPenetrationChecker();
			/**
			* @brief DTor.
			*/
			virtual ~PoorPenetrationChecker();

			// Declare constants
			static const std::string m_oFilterName;			///< Filter name.
			static const std::string m_oPipeOutName;		///< Pipe: Data out-pipe.

			static const std::string m_oParamActiveParams;			///< Parameter: ActiveParams
			static const std::string m_oParamMinWidth;				///< Parameter: MinWidth
			static const std::string m_oParamMaxWidth;				///< Parameter: MaxWidth
			static const std::string m_oParamMinLength;				///< Parameter: MinLength
			static const std::string m_oParamMaxLength;				///< Parameter: MaxLength
			static const std::string m_oParamMinGradient;			///< Parameter: MinGradient
			static const std::string m_oParamMaxGradient;			///< Parameter: MaxGradient
			static const std::string m_oParamMinGreyvalGap;			///< Parameter: MinGreyvalGap
			static const std::string m_oParamMaxGreyvalGap;			///< Parameter: MaxGreyvalGap
			static const std::string m_oParamMinRatioInnerOuter;	///< Parameter: MinRatioInnerOuter
			static const std::string m_oParamMaxRatioInnerOuter;	///< Parameter: MaxRatioInnerOuter
			static const std::string m_oParamMinStandardDeviation;	///< Parameter: MinStandardDeviation
			static const std::string m_oParamMaxStandardDeviation;	///< Parameter: MaxStandardDeviation
			static const std::string m_oParamMinDevelopedLength;	///< Parameter: MinDevelopedLength
			static const std::string m_oParamMaxDevelopedLength;	///< Parameter: MaxDevelopedLength

			/**
			* @brief Set filter parameters.
			*/
			void setParameter();

			void paint();

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
			void proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rE);

		protected:

			interface::SmpTrafo			m_oSpTrafo;					///< roi translation

			const fliplib::SynchronePipe< interface::GeoPoorPenetrationCandidatearray >* m_pPipeInData; ///< Data in-pipe.
			fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutData; ///< Data out-pipe.

			int m_oActiveParams;
			int m_oMinWidth;			
			int m_oMaxWidth;			
			int m_oMinLength;			
			int m_oMaxLength;			
			int m_oMinGradient;			
			int m_oMaxGradient;			
			int m_oMinGreyvalGap;			
			int m_oMaxGreyvalGap;			
			int m_oMinRatioInnerOuter;			
			int m_oMaxRatioInnerOuter;			
			int m_oMinStandardDeviation;			
			int m_oMaxStandardDeviation;			
			int m_oMinDevelopedLength;			
			int m_oMaxDevelopedLength;			

			std::vector<std::vector<InfoLine>> _allInfoLines;

			std::string convertIntToString(int i);
			std::string convertUuidToString(Poco::UUID id);

			static std::vector<geo2d::PoorPenetrationCandidate> _allPP;
			static int _currentImage;

			static Poco::Mutex m_oMutex;

			int getNumberOfStoredPP(geo2d::PoorPenetrationCandidate cand);

			PoorPenetrationOverlay _overlay;

		}; // class PoorPenetrationChecker

	} // namespace filter
} // namespace precitec

#endif /* POORPENETRATIONCHECKER_H_ */

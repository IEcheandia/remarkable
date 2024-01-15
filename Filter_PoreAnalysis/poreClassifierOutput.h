/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		HS
* 	@date		2013
* 	@brief		Filter that classifies pore candidates into pores and non-pores. Number of pores is the result. A NIO is thrown if a pore was classified.
*/

#ifndef PORECLASSIFIEROUTPUT_H_
#define PORECLASSIFIEROUTPUT_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include "filter/armStates.h"
#include "geo/range.h"
#include "geo/array.h"
#include "event/results.h"
#include "math/3D/projectiveMathStructures.h"
#include "poreClassifierTypes.h"
// stdlib includes
#include <array>

namespace precitec
{
	namespace filter
	{
		/**
		* @brief Filter that checks if image contains a signal and set the pore count.
		*/
		class FILTER_API PoreClassifierOutput : public fliplib::TransformFilter
		{
		public:

			/**
			* @brief CTor.
			*/
			PoreClassifierOutput();

		private:

			// Declare constants

			static const std::string m_oFilterName;				///< Filter name.
			static const std::string m_oPipePoreCountName;		///< Pipe: name out-pipe.
			static const std::string m_oPipePoreSizeMaxName;	///< Pipe: name out-pipe.
			static const std::string m_oPipePoreSizeMinName;	///< Pipe: name out-pipe.
			static const std::string m_oParamSizeMin;			///< Parameter name
			static const std::string m_oParamSizeMax;			///< Parameter name
			static const std::string m_oParamBoundingBoxDXMin;	///< Parameter name
			static const std::string m_oParamBoundingBoxDXMax;	///< Parameter name
			static const std::string m_oParamBoundingBoxDYMin;	///< Parameter name
			static const std::string m_oParamBoundingBoxDYMax;	///< Parameter name
			static const std::string m_oParamPcRatioMin;		///< Parameter name
			static const std::string m_oParamPcRatioMax;		///< Parameter name
			static const std::string m_oParamGradientMin;		///< Parameter name
			static const std::string m_oParamGradientMax;		///< Parameter name
			static const std::string m_oParamSurfaceMin;		///< Parameter name
			static const std::string m_oParamSurfaceMax;		///< Parameter name
			static const std::string m_oParamParamScaling;		///< Parameter name
			static const std::string m_oParamResultType;		///< Parameter: User-defined result type.
			static const std::string m_oParamNioType;			///< Parameter: User-defined nio type.

			/**
			* @brief Set filter parameters.
			*/
			void setParameter();

			/**
			* @brief In-pipe registration.
			* @param p_rPipe Reference to pipe that is getting connected to the filter.
			* @param p_oGroup Group number (0 - proceed is called whenever a pipe has a data element, 1 - proceed is only called when all pipes have a data element).
			*/
			bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);

			/**
			* @brief Arms filter. Enables the filter to react on different arm signals, like seam start.
			* @param p_rArmState Arm state or reason, like seam start.
			*/
			void arm(const fliplib::ArmStateBase& p_rArmState);

			/**
			* @brief Processing routine.
			* @param p_pSender pointer to
			* @param p_rEvent
			*/
			void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEvent);

			/**
			* @brief Classifies a blob as pore if all features lie within the given feature range
			* @param p_rFeatureRange	Feature range used for classification
			* @return	Number of pores classified, excluding pores with bad rank.
			*/
			class FeatureRange;
			unsigned int classify(const FeatureRange&	p_rFeatureRange, double& oMaxPoreSize, double& oMinPoreSize);

			/**
			* @brief Paint overlay output.
			*/
			void paint();

			typedef std::array<std::string, eNbFeatureTypes> featuretypes_strings_t;

			static const featuretypes_strings_t		m_oFeatureTypeString;
			static int								m_oPoreId;

			typedef fliplib::SynchronePipe<interface::GeoDoublearray>		scalar_pipe_t;
			typedef fliplib::SynchronePipe<interface::GeoBlobarray>			blob_pipe_t;
			//typedef fliplib::SynchronePipe<interface::ResultDoubleArray>	scalar_result_pipe_t;

			const scalar_pipe_t*		m_pPipeInBoundingBoxDX;		///< in-pipe.
			const scalar_pipe_t*		m_pPipeInBoundingBoxDY;		///< in-pipe.
			const scalar_pipe_t*		m_pPipeInPcRatio;			///< in-pipe.
			const scalar_pipe_t*		m_pPipeInGradient;			///< in-pipe.
			const scalar_pipe_t*		m_pPipeInSurface;			///< in-pipe.
			const blob_pipe_t*			m_pPipeInBlob;				///< in-pipe.

			fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutPoreCount; ///< out pipe => number of pores
			fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutPoreSizeMax; ///< out pipe => max pore size
			fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutPoreSizeMin; ///< out pipe => min pore size
			//scalar_result_pipe_t		m_oPipeOutPoreCount;				///< out pipe => number of pores

			interface::GeoDoublearray   m_oInputBoundingBoxDX;      ///< result received from in-pipe
			interface::GeoDoublearray   m_oInputBoundingBoxDY;      ///< result received from in-pipe
			interface::GeoDoublearray   m_oInputPcRatio;            ///< result received from in-pipe
			interface::GeoDoublearray   m_oInputGradient;           ///< result received from in-pipe
			interface::GeoDoublearray   m_oInputSurface;            ///< result received from in-pipe
			interface::GeoBlobarray     m_oInputBlob;               ///< result received from in-pipe

			interface::SmpTrafo			m_oSpTrafo;					///< roi translation
			int							m_oImgNb;					///< image number

			class FeatureRange {
			public:
				geo2d::Range1d 				m_oArea;					///< feature range
				geo2d::Range1d 				m_oBoundingBoxDX;			///< feature range
				geo2d::Range1d 				m_oBoundingBoxDY;			///< feature range
				geo2d::Range1d 				m_oPcRatio;					///< feature range
				geo2d::Range1d 				m_oGradient;				///< feature range
				geo2d::Range1d				m_oSurface;					///< feature range
			};
			FeatureRange				m_oFeatureRange;			///< feature range
			FeatureRange				m_oFeatureRangeScaled;		///< scaled feature range
			int							m_oParamScaling;			///< scaling of parameter ranges

			//interface::ResultType		m_oUserResultType;			///< User defined result type.
			//interface::ResultType		m_oUserNioType;				///< User defined nio type.

			typedef std::vector<PoreClassType>							pore_class_vec_t;
			typedef std::array<pore_class_vec_t, eNbFeatureTypes>		pore_class_feature_array_t;

			std::vector<double>			m_oArea;					///< size (area) feature vector, which is filled from blob list.
			pore_class_feature_array_t	m_oClassByFeature;			///< Classification by feature results for candidate array.
			pore_class_vec_t			m_oClassMerged;				///< Classification results for candidate array.
			std::vector<int>			m_oClassMergedRank;			///< Classification results rank for candidate array.

		}; // class PoreClassifier

	} // namespace filter
} // namespace precitec

#endif /* PORECLASSIFIER_H_ */

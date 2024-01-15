/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2016
* 	@brief		This filter sets an older seam position if the current one jumps to far away
*/

#ifndef MAXJUMP_H_
#define MAXJUMP_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include <geo/geo.h>

namespace precitec {
	namespace filter {

		class SinglePair
		{
		public:
			SinglePair();
			SinglePair(double value, int rank);

			double getValue();
			int getRank();

			void setValue(double value);
			void setRank(int rank);

		private:
			double _value;
			int _rank;
		};

		class LastValueContainer
		{
		public:
			LastValueContainer();
			void reset();
			void storePair(std::size_t index, SinglePair pair);
			void storePair(std::size_t index, double value, int rank);
			SinglePair getSingleValue(int index);

		private:
			std::vector<SinglePair> _container;

		};

		class FILTER_API MaxJump : public fliplib::TransformFilter
		{
		public:

			/**
			* CTor.
			*/
			MaxJump();
			/**
			* @brief DTor.
			*/
			virtual ~MaxJump();

			// Declare constants
			static const std::string m_oFilterName;			///< Filter name.
			static const std::string m_oPipeOutName;		///< Pipe: Data out-pipe.
			static const std::string m_oPipeOutCounterName;	///< Pipe: Data out-pipe.
			static const std::string m_oParamMode;			///< Parameter: Mode
			static const std::string m_oParamStartImage;			///< Parameter: StartImage
			static const std::string m_oParamMaxJumpWidth;			///< Parameter: MaxJump.
			static const std::string m_oParamImageAddOn;			///< Parameter: AddOn per Image.

			/**
			* @brief Set filter parameters.
			*/
			void setParameter();
			virtual void arm(const fliplib::ArmStateBase& state);


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

			const fliplib::SynchronePipe< interface::GeoDoublearray >* m_pPipeInData; ///< Data in-pipe.
			fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutData; ///< Data out-pipe.
			fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutCounterData; ///< Data out-pipe.

			int m_oMode;
			int m_oStartImage;
			int m_oMaxJumpWidth;
			int m_oImageAddOn;	

			int _imageNumber;
			int _curMaxJumpWidth;
			int _occurrenceCounter;

			LastValueContainer _lastValueContainer;
		}; // class MaxJump

	} // namespace filter
} // namespace precitec

#endif /* MAXJUMP_H_ */

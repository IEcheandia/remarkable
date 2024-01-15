/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2016
* 	@brief		This filter sets an older seam position (left/right) if the current one jumps to far away
*/

#ifndef MAXJUMP2_H_
#define MAXJUMP2_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include <geo/geo.h>

namespace precitec {
	namespace filter {

		class SingleTriple
		{
		public:
			SingleTriple();
			SingleTriple(double leftValue, double rightValue, int rank);

			double getLeftValue();
			double getRightValue();
			int getRank();

			void setLeftValue(double value);
			void setRightValue(double value);
			void setRank(int rank);

		private:
			double _leftValue;
			double _rightValue;
			int _rank;
		};

		class LastTripleContainer
		{
		public:
			LastTripleContainer();
			void reset();
			void storeTriple(int index, SingleTriple triple);
			void storeTriple(int index, double leftValue, double rightValue, int rank);
			SingleTriple getSingleTriple(int index);

		private:
			std::vector<SingleTriple> _container;

		};

		class FILTER_API MaxJump2 : public fliplib::TransformFilter
		{
		public:

			/**
			* CTor.
			*/
			MaxJump2();
			/**
			* @brief DTor.
			*/
			virtual ~MaxJump2();

			// Declare constants
			static const std::string m_oFilterName;			///< Filter name.
			static const std::string m_oPipeOutLeftName;		///< Pipe: Data out-pipe.
			static const std::string m_oPipeOutRightName;		///< Pipe: Data out-pipe.
			static const std::string m_oPipeOutCounterName;	///< Pipe: Data out-pipe.
			static const std::string m_oParamMode;			///< Parameter: Mode
			static const std::string m_oParamStartImage;			///< Parameter: StartImage
			static const std::string m_oParamMaxJumpWidth;			///< Parameter: MaxJump.
			static const std::string m_oParamMaxJumpDiff;			///< Parameter: MaxJumpDiff.
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
			void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE);

		protected:

			const fliplib::SynchronePipe< interface::GeoDoublearray >* m_pPipeInLeftData; ///< Data in-pipe.
			const fliplib::SynchronePipe< interface::GeoDoublearray >* m_pPipeInRightData; ///< Data in-pipe.
			fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutLeftData; ///< Data out-pipe.
			fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutRightData; ///< Data out-pipe.
			fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutCounterData; ///< Data out-pipe.

			int m_oMode;
			int m_oStartImage;
			int m_oMaxJumpWidth;
			int m_oMaxJumpDiff;
			int m_oImageAddOn;

			int _imageNumber;
			int _curMaxJumpWidth;
			int _curMaxJumpDiff;
			int _occurrenceCounter;

			LastTripleContainer _lastTripleContainer;
		}; // class MaxJump

	} // namespace filter
} // namespace precitec

#endif /* MAXJUMP2_H_ */

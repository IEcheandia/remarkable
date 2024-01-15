/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2016
* 	@brief		This filter sets an older seam position if the current one jumps to far away
*/

// project includes
#include "maxJump.h"
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include <event/resultType.h>
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
	namespace filter {

		const std::string MaxJump::m_oFilterName("MaxJump");
		const std::string MaxJump::m_oPipeOutName("DataOut");
		const std::string MaxJump::m_oPipeOutCounterName("CounterOut");

		const std::string MaxJump::m_oParamMode("Mode");
		const std::string MaxJump::m_oParamStartImage("StartImage");
		const std::string MaxJump::m_oParamMaxJumpWidth("MaxJumpWidth");
		const std::string MaxJump::m_oParamImageAddOn("ImageAddOn");

		MaxJump::MaxJump() : TransformFilter(MaxJump::m_oFilterName, Poco::UUID{"8D239A0E-2955-4F61-ABE3-E22C067B90BF"}), m_pPipeInData(nullptr),
			m_oPipeOutData(this, MaxJump::m_oPipeOutName), m_oPipeOutCounterData(this, MaxJump::m_oPipeOutCounterName),
			m_oMode(0), m_oStartImage(2), m_oMaxJumpWidth(50), m_oImageAddOn(0)
		{
			parameters_.add(m_oParamMode, fliplib::Parameter::TYPE_int, m_oMode);
			parameters_.add(m_oParamStartImage, fliplib::Parameter::TYPE_int, m_oStartImage);
			parameters_.add(m_oParamMaxJumpWidth, fliplib::Parameter::TYPE_int, m_oMaxJumpWidth);
			parameters_.add(m_oParamImageAddOn, fliplib::Parameter::TYPE_int, m_oImageAddOn);

			_imageNumber=0;
			_occurrenceCounter = 0;
			_curMaxJumpWidth = 0;
			_lastValueContainer.reset();

            setInPipeConnectors({{Poco::UUID("15D23F75-F6D6-462A-8F3D-6721FE56664E"), m_pPipeInData, "DataIn", 0, ""}});
            setOutPipeConnectors({{Poco::UUID("34EF5C59-CA44-4DDC-AAC9-8436361E823A"), &m_oPipeOutData, m_oPipeOutName, 0, ""},
            {Poco::UUID("474D14C6-4E84-4D00-A37D-5BD40AA27052"), &m_oPipeOutCounterData, m_oPipeOutCounterName, 0, ""}});
            setVariantID(Poco::UUID("B83FFF85-5059-4BAE-BAA7-6AE9CF67E6EA"));
		}

		MaxJump::~MaxJump()
		{
		}

		void MaxJump::setParameter()
		{
			TransformFilter::setParameter();

			m_oMode = parameters_.getParameter(MaxJump::m_oParamMode).convert<int>();
			m_oStartImage = parameters_.getParameter(MaxJump::m_oParamStartImage).convert<int>();
			m_oMaxJumpWidth = parameters_.getParameter(MaxJump::m_oParamMaxJumpWidth).convert<int>();
			m_oImageAddOn = parameters_.getParameter(MaxJump::m_oParamImageAddOn).convert<int>();
		} // setParameter.

		void MaxJump::arm(const fliplib::ArmStateBase& state)
		{
			if (state.getStateID() == eSeamStart)
			{
				_imageNumber = 0;
				_occurrenceCounter = 0;
				_curMaxJumpWidth = m_oMaxJumpWidth;
				_lastValueContainer.reset();
			}
		}

		bool MaxJump::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
		{
			m_pPipeInData = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);

			return BaseFilter::subscribe(p_rPipe, p_oGroup);
		} // subscribe

		void MaxJump::proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rE)
		{
			poco_assert_dbg(m_pPipeInData != nullptr); // to be asserted by graph editor

			if (_imageNumber == 0) _curMaxJumpWidth = m_oMaxJumpWidth;
			_imageNumber++;

			// data
			const interface::GeoDoublearray &rGeoDoubleArrayIn = m_pPipeInData->read(m_oCounter);

			// operation
			geo2d::Doublearray oOut;
			geo2d::Doublearray oOutCounter;

			double lastValue = 0.0, oOutValue = 0.0, oOutValueCounter = 0.0, oInValue = 0.0;
			int lastRank = eRankMax, oOutRank = eRankMax, oInRank = eRankMax;
			SinglePair pair;

			unsigned int oSizeOfArray = rGeoDoubleArrayIn.ref().size();
			oOut.assign(oSizeOfArray);
			oOutCounter.assign(1);

			for (unsigned int oIndex = 0; oIndex < oSizeOfArray; oIndex++)
			{
				// get the data
				oInValue = std::get<eData>(rGeoDoubleArrayIn.ref()[oIndex]);
				oInRank = std::get<eRank>(rGeoDoubleArrayIn.ref()[oIndex]);

				if (_imageNumber < m_oStartImage) // zu frueh fuer Korrektur
				{  // nix zu korrigieren, nur speichern
					oOutValue = oInValue;
					oOutRank = oInRank;
				}
				else
				{
					pair = _lastValueContainer.getSingleValue(oIndex);
					lastValue = pair.getValue();
					lastRank = pair.getRank();
					if (std::abs(lastValue - oInValue) > _curMaxJumpWidth) // neuer Wert ist mehr als MaxJump vom alten entfernt => setzen auf alten Wert
					{
						_curMaxJumpWidth += m_oImageAddOn;
						_occurrenceCounter++;
						oOutValue = lastValue;
						oOutRank = lastRank;
					}
					else // Sprung ist nicht zu gross, wert kann so weiter gegeben werden
					{
						_curMaxJumpWidth = m_oMaxJumpWidth;
						oOutValue = oInValue;
						oOutRank = oInRank;
					}
				}
				_lastValueContainer.storePair(oIndex, oOutValue, oOutRank); // das, was jetzt rausgeht, speichern

				oOut[oIndex] = std::tie(oOutValue, oOutRank);
				oOutValueCounter = _occurrenceCounter;
				oOutCounter[0] = std::tie(oOutValueCounter, oOutRank);
			} // for

			const interface::GeoDoublearray oGeoDoubleOut(rGeoDoubleArrayIn.context(), oOut, rGeoDoubleArrayIn.analysisResult(), rGeoDoubleArrayIn.rank());
			const interface::GeoDoublearray oGeoDoubleCounterOut(rGeoDoubleArrayIn.context(), oOutCounter, rGeoDoubleArrayIn.analysisResult(), rGeoDoubleArrayIn.rank());
			preSignalAction();
			m_oPipeOutData.signal(oGeoDoubleOut);
			m_oPipeOutCounterData.signal(oGeoDoubleCounterOut);

		} // proceedGroup

		SinglePair::SinglePair()
		{
			_value = 0.0;
			_rank = 0;
		}

		SinglePair::SinglePair(double value, int rank)
		{
			_value = value;
			_rank = rank;
		}

		double SinglePair::getValue()
		{
			return _value;
		}

		int SinglePair::getRank()
		{
			return _rank;
		}

		void SinglePair::setValue(double value)
		{
			_value = value;
		}

		void SinglePair::setRank(int rank)
		{
			_rank = rank;
		}

		LastValueContainer::LastValueContainer()
		{
			reset();
		}

		void LastValueContainer::reset()
		{
			_container.clear();
		}

		void LastValueContainer::storePair(std::size_t index, double value, int rank)
		{
			while (_container.size() <= index) _container.push_back(SinglePair());
			_container[index].setValue(value);
			_container[index].setRank(rank);
		}

		void LastValueContainer::storePair(std::size_t index, SinglePair pair)
		{
			storePair(index, pair.getValue(), pair.getRank());
		}

		SinglePair LastValueContainer::getSingleValue(int index)
		{
			int size = _container.size();
			if (index >= size) index = size - 1;
			return _container[index];
		}



	} // namespace filter
} // namespace precitec

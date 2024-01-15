/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2016
* 	@brief		This filter sets an older seam position (left/right) if the current one jumps to far away
*/

// project includes
#include "maxJump2.h"
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include <event/resultType.h>
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
	namespace filter {

		const std::string MaxJump2::m_oFilterName("MaxJump2");
		const std::string MaxJump2::m_oPipeOutLeftName("DataLeftOut");
		const std::string MaxJump2::m_oPipeOutRightName("DataRightOut");
		const std::string MaxJump2::m_oPipeOutCounterName("CounterOut");

		const std::string MaxJump2::m_oParamMode("Mode");
		const std::string MaxJump2::m_oParamStartImage("StartImage");
		const std::string MaxJump2::m_oParamMaxJumpWidth("MaxJumpWidth");
		const std::string MaxJump2::m_oParamMaxJumpDiff("MaxJumpDiff");
		const std::string MaxJump2::m_oParamImageAddOn("ImageAddOn");

		MaxJump2::MaxJump2() : TransformFilter(MaxJump2::m_oFilterName, Poco::UUID{"B1BD1A01-A1A6-4E53-BDD6-5B552C7C9373"}), m_pPipeInLeftData(nullptr), m_pPipeInRightData(nullptr),
			m_oPipeOutLeftData(this, MaxJump2::m_oPipeOutLeftName), m_oPipeOutRightData(this, MaxJump2::m_oPipeOutRightName),
			m_oPipeOutCounterData(this, MaxJump2::m_oPipeOutCounterName),
			m_oMode(0), m_oStartImage(2), m_oMaxJumpWidth(50), m_oMaxJumpDiff(50), m_oImageAddOn(0)
		{
			parameters_.add(m_oParamMode, fliplib::Parameter::TYPE_int, m_oMode);
			parameters_.add(m_oParamStartImage, fliplib::Parameter::TYPE_int, m_oStartImage);
			parameters_.add(m_oParamMaxJumpWidth, fliplib::Parameter::TYPE_int, m_oMaxJumpWidth);
			parameters_.add(m_oParamMaxJumpDiff, fliplib::Parameter::TYPE_int, m_oMaxJumpDiff);
			parameters_.add(m_oParamImageAddOn, fliplib::Parameter::TYPE_int, m_oImageAddOn);

			_imageNumber = 0;
			_occurrenceCounter = 0;
			_curMaxJumpWidth = 0;
			_curMaxJumpDiff = 0;
			_lastTripleContainer.reset();

            setInPipeConnectors({{Poco::UUID("6F1EE607-36D7-44DA-A717-4495A341C1E7"), m_pPipeInLeftData, "DataLeftIn", 1, "LeftData"},
            {Poco::UUID("23FAA673-7AF5-4E7E-9252-A74651475845"), m_pPipeInRightData, "DataRightIn", 1, "RightData"}});
            setOutPipeConnectors({{Poco::UUID("0F4CCBBE-9405-45CF-9DFD-C5D128B3F1A7"), &m_oPipeOutLeftData, m_oPipeOutLeftName, 0, ""},
            {Poco::UUID("5E2EE942-80E2-4F07-9DBC-A8E9A07C69F1"), &m_oPipeOutRightData, m_oPipeOutRightName, 0, ""},
            {Poco::UUID("C3A5AB3B-3EE6-4E56-8E89-D6843F5FA45F"), &m_oPipeOutCounterData, m_oPipeOutCounterName, 0, ""}});
            setVariantID(Poco::UUID("EF8C45A8-34D1-4FE2-82C2-720B94192409"));
		}

		MaxJump2::~MaxJump2()
		{
		}

		void MaxJump2::setParameter()
		{
			TransformFilter::setParameter();

			m_oMode = parameters_.getParameter(MaxJump2::m_oParamMode).convert<int>();
			m_oStartImage = parameters_.getParameter(MaxJump2::m_oParamStartImage).convert<int>();
			m_oMaxJumpWidth = parameters_.getParameter(MaxJump2::m_oParamMaxJumpWidth).convert<int>();
			m_oMaxJumpDiff = parameters_.getParameter(MaxJump2::m_oParamMaxJumpDiff).convert<int>();
			m_oImageAddOn = parameters_.getParameter(MaxJump2::m_oParamImageAddOn).convert<int>();
		} // setParameter.

		void MaxJump2::arm(const fliplib::ArmStateBase& state)
		{
			if (state.getStateID() == eSeamStart)
			{
				_imageNumber = 0;
				_occurrenceCounter = 0;
				_curMaxJumpWidth = m_oMaxJumpWidth;
				_curMaxJumpDiff = m_oMaxJumpDiff;
				_lastTripleContainer.reset();
			}
		}

		bool MaxJump2::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
		{
			if (p_rPipe.tag() == "LeftData")
				m_pPipeInLeftData = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
			if (p_rPipe.tag() == "RightData")
				m_pPipeInRightData = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);

			return BaseFilter::subscribe(p_rPipe, p_oGroup);
		} // subscribe

		void MaxJump2::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
		{
			poco_assert_dbg(m_pPipeInLeftData != nullptr); // to be asserted by graph editor
			poco_assert_dbg(m_pPipeInRightData != nullptr); // to be asserted by graph editor

			if (_imageNumber == 0)
			{
				_curMaxJumpWidth = m_oMaxJumpWidth;
				_curMaxJumpDiff = m_oMaxJumpDiff;
			}

			_imageNumber++;

			// data
			const interface::GeoDoublearray &rGeoDoubleArrayLeftIn = m_pPipeInLeftData->read(m_oCounter);
			const interface::GeoDoublearray &rGeoDoubleArrayRightIn = m_pPipeInRightData->read(m_oCounter);

			// operation
			geo2d::Doublearray oOutLeft;
			geo2d::Doublearray oOutRight;
			geo2d::Doublearray oOutCounter;

			double lastValueLeft = 0.0,  oOutValueLeft = 0.0, oOutValueCounter = 0.0, oInValueLeft = 0.0;
			double lastValueRight = 0.0, oOutValueRight = 0.0,                        oInValueRight = 0.0;
			int lastRank = eRankMax, oOutRank = eRankMax, oInRank = eRankMax, oInRankLeft = eRankMax, oInRankRight = eRankMax;
			SingleTriple triple;

			int leftSize = rGeoDoubleArrayLeftIn.ref().size();
			int rightSize = rGeoDoubleArrayRightIn.ref().size();
			unsigned int oSizeOfArray = leftSize < rightSize ? leftSize : rightSize;
			oOutLeft.assign(oSizeOfArray);
			oOutRight.assign(oSizeOfArray);
			oOutCounter.assign(1);

			for (unsigned int oIndex = 0; oIndex < oSizeOfArray; oIndex++)
			{
				// get the data
				oInValueLeft = std::get<eData>(rGeoDoubleArrayLeftIn.ref()[oIndex]);
				oInValueRight = std::get<eData>(rGeoDoubleArrayRightIn.ref()[oIndex]);
				oInRankLeft = std::get<eRank>(rGeoDoubleArrayLeftIn.ref()[oIndex]);
				oInRankRight = std::get<eRank>(rGeoDoubleArrayRightIn.ref()[oIndex]);
				oInRank = oInRankLeft < oInRankRight ? oInRankLeft : oInRankRight;

				if (_imageNumber < m_oStartImage ) // zu frueh fuer Korrektur
				{  // nix zu korrigieren, nur speichern
					oOutValueLeft = oInValueLeft;
					oOutValueRight = oInValueRight;
					oOutRank = oInRank;
				}
				else
				{
					triple = _lastTripleContainer.getSingleTriple(oIndex);
					lastValueLeft = triple.getLeftValue();
					lastValueRight = triple.getRightValue();
					lastRank = triple.getRank();

					double lastWidth = std::abs(lastValueRight - lastValueLeft);
					double curWidth = std::abs(oInValueRight - oInValueLeft);

					if ( (std::abs(lastValueLeft - oInValueLeft) > _curMaxJumpWidth) ||
						 (std::abs(lastValueRight - oInValueRight) > _curMaxJumpWidth) ||
						 (std::abs(lastWidth - curWidth) > _curMaxJumpDiff) ) // neue Werte sind zu weit weg => setzen auf alten Wert
					{
						_curMaxJumpWidth += m_oImageAddOn;
						_curMaxJumpDiff += m_oImageAddOn;
						_occurrenceCounter++;
						oOutValueLeft = lastValueLeft;
						oOutValueRight = lastValueRight;
						oOutRank = lastRank;
					}
					else // Sprung ist nicht zu gross, wert kann so weiter gegeben werden
					{
						_curMaxJumpWidth = m_oMaxJumpWidth;
						_curMaxJumpDiff = m_oMaxJumpDiff;
						oOutValueLeft = oInValueLeft;
						oOutValueRight = oInValueRight;
						oOutRank = oInRank;
					}
				}
				if (oOutRank) // no save Rank 0
				{
					_lastTripleContainer.storeTriple(oIndex, oOutValueLeft, oOutValueRight, oOutRank); // das, was jetzt rausgeht, speichern
				}
				else
				{
					_imageNumber--;
				}

				oOutLeft[oIndex] = std::tie(oOutValueLeft, oOutRank);
				oOutRight[oIndex] = std::tie(oOutValueRight, oOutRank);
				oOutValueCounter = _occurrenceCounter;
				oOutCounter[0] = std::tie(oOutValueCounter, oOutRank);
			} // for

			const interface::GeoDoublearray oGeoDoubleLeftOut(rGeoDoubleArrayLeftIn.context(), oOutLeft, rGeoDoubleArrayLeftIn.analysisResult(), rGeoDoubleArrayLeftIn.rank());
			const interface::GeoDoublearray oGeoDoubleRightOut(rGeoDoubleArrayRightIn.context(), oOutRight, rGeoDoubleArrayRightIn.analysisResult(), rGeoDoubleArrayRightIn.rank());
			const interface::GeoDoublearray oGeoDoubleCounterOut(rGeoDoubleArrayLeftIn.context(), oOutCounter, rGeoDoubleArrayLeftIn.analysisResult(), rGeoDoubleArrayLeftIn.rank());
			preSignalAction();
			m_oPipeOutLeftData.signal(oGeoDoubleLeftOut);
			m_oPipeOutRightData.signal(oGeoDoubleRightOut);
			m_oPipeOutCounterData.signal(oGeoDoubleCounterOut);

		} // proceedGroup

		SingleTriple::SingleTriple()
		{
			_leftValue = 0.0;
			_rightValue = 0.0;
			_rank = 0;
		}

		SingleTriple::SingleTriple(double leftValue, double rightValue, int rank)
		{
			_leftValue = leftValue;
			_rightValue = rightValue;
			_rank = rank;
		}

		double SingleTriple::getLeftValue()
		{
			return _leftValue;
		}

		double SingleTriple::getRightValue()
		{
			return _rightValue;
		}

		int SingleTriple::getRank()
		{
			return _rank;
		}

		void SingleTriple::setLeftValue(double value)
		{
			_leftValue = value;
		}

		void SingleTriple::setRightValue(double value)
		{
			_rightValue = value;
		}

		void SingleTriple::setRank(int rank)
		{
			_rank = rank;
		}

		LastTripleContainer::LastTripleContainer()
		{
			reset();
		}

		void LastTripleContainer::reset()
		{
			_container.clear();
		}

		void LastTripleContainer::storeTriple(int index, double leftValue, double rightValue, int rank)
		{
			while ((int)_container.size() <= index) _container.push_back(SingleTriple());
			_container[index].setLeftValue(leftValue);
			_container[index].setRightValue(rightValue);
			_container[index].setRank(rank);
		}

		void LastTripleContainer::storeTriple(int index, SingleTriple triple)
		{
			storeTriple(index, triple.getLeftValue(), triple.getRightValue(), triple.getRank());
		}

		SingleTriple LastTripleContainer::getSingleTriple(int index)
		{
			int size = _container.size();
			if (index >= size) index = size - 1;
			return _container[index];
		}



	} // namespace filter
} // namespace precitec

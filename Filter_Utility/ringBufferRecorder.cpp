/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		2015
 * 	@brief		This filter stores data elements in a ring buffer. They can be accessed in a different filter graph using the buffer player filter.
 */

// WM includes
#include "ringBufferRecorder.h"
#include <filter/buffer.h>
#include <fliplib/PipeEventArgs.h>
#include <fliplib/Parameter.h>
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include <filter/productData.h>
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
using namespace interface;
namespace filter {

const std::string RingBufferRecorder::m_oFilterName 		( "RingBufferRecorder" );
const std::string RingBufferRecorder::m_oParamSlot			( "Slot" );				///< Parameter: Slot, into which the data is written.
const std::string RingBufferRecorder::m_oParamMode			( "Mode" );				///< Parameter
const std::string RingBufferRecorder::m_oParamTicks			( "Ticks" );			///< Parameter
const std::string RingBufferRecorder::m_oParamWidth			( "Width" );			///< Parameter
const std::string RingBufferRecorder::m_oParamWidthMedian	( "WidthMedian" );		///< Parameter

RingBufferRecorder::RingBufferRecorder() :
	TransformFilter			( RingBufferRecorder::m_oFilterName, Poco::UUID{"CE39CE03-C163-433E-B4AF-D55EEBB0D74C"} ),
	m_pPipeInData			( nullptr ),
	m_pPipeInPos			( nullptr ),
	m_oSlot					( 1 ),
	m_oCount				( 0 )
{
	parameters_.add( m_oParamSlot, fliplib::Parameter::TYPE_uint, m_oSlot );
	parameters_.add( m_oParamMode, fliplib::Parameter::TYPE_uint, m_oMode );
	parameters_.add( m_oParamTicks, fliplib::Parameter::TYPE_uint, m_oTicks );
	parameters_.add( m_oParamWidth, fliplib::Parameter::TYPE_uint, m_oWidth );
	parameters_.add( m_oParamWidthMedian, fliplib::Parameter::TYPE_uint, m_oWidthMedian );

    setInPipeConnectors({{Poco::UUID("4ECA47C8-2406-459B-81DE-804F3658D6FA"), m_pPipeInData, "Data", 1, "data"},
    {Poco::UUID("03B71FB6-B06E-45B7-9AF6-466B879B8A4A"), m_pPipeInPos, "Position", 1, "pos", fliplib::PipeConnector::ConnectionType::Optional}});
    setVariantID(Poco::UUID("B762A95B-19B9-4F09-8D26-54E9931886B7"));
}

RingBufferRecorder::~RingBufferRecorder()
{
}

void RingBufferRecorder::setParameter()
{
	TransformFilter::setParameter();
	m_oSlot = parameters_.getParameter(RingBufferRecorder::m_oParamSlot).convert<unsigned int>();
	m_oMode = parameters_.getParameter(RingBufferRecorder::m_oParamMode).convert<unsigned int>();
	m_oTicks = parameters_.getParameter(RingBufferRecorder::m_oParamTicks).convert<unsigned int>();
	m_oWidth = parameters_.getParameter(RingBufferRecorder::m_oParamWidth).convert<unsigned int>();
	m_oWidthMedian = parameters_.getParameter( RingBufferRecorder::m_oParamWidthMedian ).convert<unsigned int>();
} // setParameter.

void RingBufferRecorder::arm(const fliplib::ArmStateBase& state)
{
	if (state.getStateID() == eSeamStart)
	{
		_ownBuffer.reset(); // Puffer zuruecksetzen bei neuer Naht
		_ownBuffer.setTicksPer360(m_oTicks);
		_ownBuffer.setWidths(m_oWidth, m_oWidthMedian);
		// get product information
		const analyzer::ProductData* pProductData = externalData<analyzer::ProductData>();
		m_oTriggerDelta = pProductData->m_oTriggerDelta;

		// reset / initialize the memory for the slot that was selected by the user ...
		Buffer& rData = BufferSingleton::getInstanceData();
		if (pProductData->m_oNumTrigger < 100000)
		{
			rData.init(m_oSlot, pProductData->m_oSeamSeries, pProductData->m_oSeam, pProductData->m_oNumTrigger);
		}
		else
		{
			rData.init(m_oSlot, pProductData->m_oSeamSeries, pProductData->m_oSeam, 100000);
		}
		m_pData = rData.get( m_oSlot, pProductData->m_oSeamSeries, pProductData->m_oSeam );

		m_bufferData.ref().getData().resize(0);
		m_bufferPos.ref().getData().resize(0);
		m_bufferData.ref().getRank().resize(0);
		m_bufferPos.ref().getRank().resize(0);

		Buffer& rPos  = BufferSingleton::getInstancePos();
		if ( pProductData->m_oNumTrigger < 100000 )
			rPos.init( m_oSlot, pProductData->m_oSeamSeries, pProductData->m_oSeam, pProductData->m_oNumTrigger );
		else
			rPos.init( m_oSlot, pProductData->m_oSeamSeries, pProductData->m_oSeam, 100000 );
		m_pPos = rPos.get( m_oSlot, pProductData->m_oSeamSeries, pProductData->m_oSeam );

		m_oCount = 0;
	} // if

	if (state.getStateID() == eSeamEnd)
	{
		_ownBuffer.setTicksPer360(m_oTicks);
		_ownBuffer.setWidths(m_oWidth, m_oWidthMedian);

		std::size_t startIndex = 0;
		std::size_t endIndex = 0;

		if (!_ownBuffer.is360()) // keine 360 Grad gedreht
		{
			// Analysefehler ausloesen!
			// Tja, was machen?
		}

		if (!_ownBuffer.isSorted()) _ownBuffer.sort(); // sollte sortiert sein, da Punkte mit steigendem Encoder reinkommen

		if (m_oMode >= 10) // Taschen-Rundteile vorher speziell behandeln
		{
			// vorne und hinten evtl. Taschen wegschneiden
			_ownBuffer.cutUselessAreasAtBeginAndEnd();

			// Taschenbereiche interpolieren, Rest low-passen
			_ownBuffer.lowPassAndInterpolate(m_oMode);
		}

		// Berechnet, von wo bis wo der RingPuffer nur Sinn macht
		_ownBuffer.calcStartEndValues(startIndex, endIndex);

		// reduziert den Puffer auf den interessanten Bereich
		_ownBuffer.reduceToStartEnd(startIndex, endIndex);

		// Positionen auf Null bis ticksPer360 setzen
		_ownBuffer.makePosModulo();

		// nochmals sortieren
		if (!_ownBuffer.isSorted()) _ownBuffer.sort();

		// fuehrt die eigentlich Mittelung ueber die Raender hinweg durch
		_ownBuffer.doLowPass(m_oMode);

		// Ring-Puffer in Slot ablegen, damit alles wie bisher aussieht
		const std::size_t curSize = _ownBuffer.getCurrentSize();
		for (std::size_t i=0; i<curSize; i++)
		{
			OneDataSet oneDataSet = _ownBuffer.getOneDataSet(i);

			m_pData->ref()[i] = std::tie( oneDataSet.data, oneDataSet.data_rank );
			m_bufferData.ref().getData().push_back(oneDataSet.data);
			m_bufferData.ref().getRank().push_back(oneDataSet.data_rank);

			m_pPos->ref()[i] = std::tie( oneDataSet.pos, oneDataSet.pos_rank );
			m_bufferPos.ref().getData().push_back(oneDataSet.pos);
			m_bufferPos.ref().getRank().push_back(oneDataSet.pos_rank);
		}
	}

	if(m_oVerbosity >= eHigh)
	{
		wmLog(eDebug, "Filter '%s' armed at armstate %i\n", m_oFilterName.c_str(),  state.getStateID());

	} // if
} // arm

bool RingBufferRecorder::subscribe( fliplib::BasePipe& p_rPipe, int p_oGroup )
{
	if ( p_rPipe.tag() == "data" )
		m_pPipeInData = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray > * >(&p_rPipe);
	if ( p_rPipe.tag() == "pos" )
		m_pPipeInPos  = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe

void RingBufferRecorder::proceedGroup( const void* p_pSender, fliplib::PipeGroupEventArgs& e )
{
	poco_assert_dbg( m_pPipeInData != nullptr); // to be asserted by graph editor

	// checks, should never happen ...
	if ( m_pData == std::shared_ptr< interface::GeoDoublearray >() || m_pPos == std::shared_ptr< interface::GeoDoublearray >() )
	{
		wmLog( eError, "Buffer is not initialized, cannot write any elements!\n");

        preSignalAction();
		return;
	}
	if ( (int)(m_pData->ref().size()) <= m_oCount || (int)(m_pPos->ref().size()) <= m_oCount )
	{
		wmLog( eError, "Buffer is too small, cannot write more elements!\n");

        preSignalAction();
		return;
	}

	// data
	const GeoDoublearray &rGeoDoubleArrayIn  = m_pPipeInData->read(m_oCounter);

	//m_pData->ref()[m_oCount] = rGeoDoubleArrayIn.ref()[0];

	double currentData = rGeoDoubleArrayIn.ref().getData()[0];
	int currentDataRank = rGeoDoubleArrayIn.ref().getRank()[0];

	//m_bufferData.ref().getData().push_back(currentData);
	//m_bufferData.ref().getRank().push_back(currentDataRank);

	// position information - do we have actual information from the in-pipe, or do we simply calculate the position based on the time?
	int oRank = eRankMax;
	double currentPos;
	int currentPosRank;
	if ( m_pPipeInPos == nullptr )
	{
		double oPos = m_oCount * m_oTriggerDelta;
		//m_pPos->ref()[m_oCount] = std::tie( oPos, oRank );
		currentPos = oPos;
		currentPosRank = oRank;
	}
	else
	{
		const GeoDoublearray &rGeoDoublePosIn  = m_pPipeInPos->read(m_oCounter);
		//m_pPos->ref()[m_oCount] = rGeoDoublePosIn.ref()[0];
		currentPos = rGeoDoublePosIn.ref().getData()[0];
		currentPosRank = rGeoDoublePosIn.ref().getRank()[0];
	}

	//m_bufferPos.ref().getData().push_back(currentPos);
	//m_bufferPos.ref().getRank().push_back(currentPosRank);

	_ownBuffer.addOneDataSet(OneDataSet(currentData, currentDataRank, currentPos, currentPosRank));

	// debug
	//double oValue = sin( double( 0.1 * m_oCount) );
	//m_pData->ref()[m_oCount] = std::tie( oValue, oRank );

	// some debug output
	if( m_oVerbosity >= eHigh )
	{
		wmLog( eInfo, "BufferRecorder: Storing %f (r:%d) at %f in slot %d element %d!\n", std::get<eData>(m_pData->ref()[m_oCount]), std::get<eRank>(m_pData->ref()[m_oCount]), std::get<eData>(m_pPos->ref()[m_oCount]), m_oSlot, m_oCount );
	}

	// ok, we are done here, lets increase write-index
	m_oCount++;

    preSignalAction();
} // proceedGroup

///////////////////////////////////////////////////////////////////////
// OwnBuffer
///////////////////////////////////////////////////////////////////////

OneDataSet::OneDataSet()
{
	data = 0;
	data_rank = 0;
	pos = 0;
	pos_rank = 0;
}

OneDataSet::OneDataSet(const OneDataSet & anotherSet)
{
	data = anotherSet.data;
	data_rank = anotherSet.data_rank;
	pos = anotherSet.pos;
	pos_rank = anotherSet.pos_rank;
}

OneDataSet::OneDataSet(double data_, int data_rank_, double pos_, int pos_rank_)
{
	data=data_;
	data_rank = data_rank_;
	pos = pos_;
	pos_rank = pos_rank_;
}

OwnBuffer::OwnBuffer()
{
	reset();
}

void OwnBuffer::reset()
{
	_dataSet.clear();
}

void OwnBuffer::addOneDataSet(OneDataSet set)
{
	_dataSet.push_back(set);
}

OneDataSet OwnBuffer::getOneDataSet(std::size_t i)
{
	if ( (i<0) || (i>=_dataSet.size()) ) return OneDataSet();
	return _dataSet[i];
}

bool OwnBuffer::isSorted()
{
	for (std::size_t i=1; i<_dataSet.size(); i++) if (_dataSet[i-1].pos > _dataSet[i].pos) return false;
	return true;
}

bool OwnBuffer::is360()
{
	double min = getMinPos();
	double max = getMaxPos();

	double totalDist = max - min;

	return (totalDist > _ticksPer360);
}

void OwnBuffer::sort()
{ // Bubble Sort
	std::size_t size = _dataSet.size();
	if (size <= 1) return;
	for (std::size_t i = 0; i<size - 1; i++)
	{
		for (std::size_t j = 0; j<size - 1 - i; j++)
		{
			if (_dataSet[j].pos > _dataSet[j+1].pos) exchange(j, j+1);
		}
	}
}

void OwnBuffer::exchange(std::size_t i, std::size_t j)
{
	if (i==j) return;
	if (i>=_dataSet.size()) return;
	if (j>=_dataSet.size()) return;

	OneDataSet dataSet = _dataSet[i];
	_dataSet[i] = _dataSet[j];
	_dataSet[j] = dataSet;
}

void OwnBuffer::setTicksPer360(int number)
{
	_ticksPer360 = number;
}

void OwnBuffer::setWidths(int widthMean, int widthMedian)
{
	_widthMean = widthMean;
	_widthMedian = widthMedian;
}

std::size_t OwnBuffer::getCurrentSize()
{
	return _dataSet.size();
}

void OwnBuffer::calcStartEndValues(std::size_t & startIndex, std::size_t & endIndex)
{
	std::size_t size = _dataSet.size();
	startIndex = 0;
	endIndex = size - 1;

	double minPos = getMinPos();
	double maxPos = getMaxPos();

	if (maxPos - minPos <= _ticksPer360) return;

	double startPos =  (maxPos - minPos - _ticksPer360) / 2 + minPos;
	double endPos = startPos + _ticksPer360;

	for (std::size_t i = 0; i<size; i++)
	{
		if (_dataSet[i].pos >= startPos)
		{
			startIndex = i;
			break;
		}
	}

	for (std::size_t i = size - 1; i >= 0; i--)
	{
		if (_dataSet[i].pos <= endPos)
		{
			endIndex = i;
			break;
		}
	}
}

double OwnBuffer::getMinPos()
{
	std::size_t size = _dataSet.size();
	if (size == 0) return 0;
	double minPos = _dataSet[0].pos;
	for (std::size_t i=1; i<size; i++)
	{
		if (_dataSet[i].pos < minPos) minPos = _dataSet[i].pos;
	}
	return minPos;
}

double OwnBuffer::getMaxPos()
{
	std::size_t size = _dataSet.size();
	if (size == 0) return 0;
	double maxPos = _dataSet[size-1].pos;
	for (std::size_t i=0; i<size-1; i++)
	{
		if (_dataSet[i].pos > maxPos) maxPos = _dataSet[i].pos;
	}
	return maxPos;
}

void OwnBuffer::reduceToStartEnd(std::size_t start, std::size_t end)
{
	std::size_t currentSize = _dataSet.size();
	if (end<=start) return; // Reduzierung unzulaessig
	if (end>=currentSize) return; //Reduzierung nicht moeglich, Ende ausserhalb
	if (start<0) return; //Reduzierung nicht moeglich, Start ausserhalb

	if (start==0) // Reduzierung nur hinten
	{
		_dataSet.resize(end + 1);
		return;
	}

	currentSize = end - start + 1;

	for (std::size_t i=0; i<currentSize; i++)
	{
		_dataSet[i]=_dataSet[i+start];
	}
	_dataSet.resize(currentSize);
}

bool OwnBuffer::isInRange(double posTicks1, double posTicks2, double angleMax)
{
	if (std::abs(angleMax)<0.000001) return ( std::abs(posTicks1-posTicks2)<0.000001 );

	double ticksPerOneDegree = ((double)(_ticksPer360)) / 360.0;

	double posAngle1 = posTicks1 / ticksPerOneDegree;
	double posAngle2 = posTicks2 / ticksPerOneDegree;

	while (posAngle1 <    0) posAngle1 = posAngle1 + 360;
	while (posAngle2 <    0) posAngle2 = posAngle2 + 360;
	while (posAngle1 >= 360) posAngle1 = posAngle1 - 360;
	while (posAngle2 >= 360) posAngle2 = posAngle2 - 360;

	if (std::abs(posAngle1 - posAngle2) <= angleMax) return true;
	posAngle1 += 360.0;
	if (std::abs(posAngle1 - posAngle2) <= angleMax) return true;
	posAngle1 -= 360.0;
	posAngle2 += 360.0;
	if (std::abs(posAngle1 - posAngle2) <= angleMax) return true;
	return false;
}

void OwnBuffer::doLowPass(unsigned int mode)
{
	int lowPassWidth=1;

	if (mode==0) // 0  0  0  0  0  0  0  0
	{
		lowPassWidth = _widthMean;
	}

	if (mode==1) // 1  1  1  1  1  1  1  1
	{
		lowPassWidth = _widthMedian;
	}

	if (mode==2) // 2  2  2  2  2  2  2  2
	{ // 2x low-passen: erst Median, dann Mittelwert
		doLowPass(1);
		doLowPass(0);
		return;
	}

	if (mode == 10) // 10  10  10  10  10  10  10
	{
		doLowPass(0);
		return;
	}

	if (mode == 11)
	{
		doLowPass(1);
		return;
	}

	if (mode == 12)
	{
		doLowPass(2);
		return;
	}

	std::size_t size = _dataSet.size();
	std::vector<OneDataSet> _dataSetNew(_dataSet.size());

	StatisticCalculator statisticData;
	StatisticCalculator statisticRank;

	for (std::size_t i = 0; i<size; i++)
	{
		statisticData.reset();
		statisticRank.reset();

		for (std::size_t j = 0; j<size; j++)
		{
			if (isInRange(_dataSet[i].pos, _dataSet[j].pos, lowPassWidth))
			{
				statisticData.addValue(_dataSet[j].data);
				statisticRank.addValue(_dataSet[j].data_rank);
			}
		}

		if (mode == 0)
		{
			_dataSetNew[i].data = statisticData.getMean();
			_dataSetNew[i].data_rank = (int)(0.5+statisticRank.getMean());
		}
		else
		{
			_dataSetNew[i].data = statisticData.getMedian();
			_dataSetNew[i].data_rank = (int)(0.5+statisticRank.getMedian());
		}

		_dataSetNew[i].pos = _dataSet[i].pos;
		_dataSetNew[i].pos_rank = _dataSet[i].pos_rank;
	}

	for (std::size_t i = 0; i<size; i++) _dataSet[i] = _dataSetNew[i];
}

void OwnBuffer::doLowPass()
{
	doLowPass(1);
}

int OwnBuffer::round(double d)
{
	return (int)(d+0.5);
}

void OwnBuffer::makePosModulo()
{
	for (int i=0; i<(int)_dataSet.size(); i++)
	{
		int pos_int = (int)(_dataSet[i].pos + 0.5);
		_dataSet[i].pos = pos_int % _ticksPer360;
	}
}

// Ab hier fuer RundTeil mit Taschen

double OwnBuffer::calcMeanWithMinRank(int minRank)
{
	double sum = 0;
	int number = 0;
	for (std::size_t i=0; i<getCurrentSize(); i++)
	{
		OneDataSet oneSet = getOneDataSet(i);
		if (oneSet.data_rank >= minRank) // Nur Mess-Punkte betrachten, die den minRank haben
		{
			sum += oneSet.data;
			number++;
		}
	}
	if (number==0) return 0.0;
	return sum / number;
}

void OwnBuffer::setDataForBadRank(int badRank, double dataToSet)
{
	for (std::size_t i = 0; i < getCurrentSize(); i++)
	{
		OneDataSet & oneSet = _dataSet[i];
		if (oneSet.data_rank <= badRank)
		{
			oneSet.data = dataToSet;
		}
	}
}

void OwnBuffer::doMeanOnRange(int firstSet, int lastSet, int meanSize)
{
	doLowPassOnRange(firstSet, lastSet, meanSize, true);
}

void OwnBuffer::doMedianOnRange(int firstSet, int lastSet, int medianSize)
{
	doLowPassOnRange(firstSet, lastSet, medianSize, false);
}

void OwnBuffer::doLowPassOnRange(int firstSet, int lastSet, int lowPassSize, bool useMean)
{
	if (firstSet<0) return;
	if (lastSet>=(int)_dataSet.size()) return;
	if (lowPassSize<=1) return;

	std::vector<OneDataSet> dataSetNew(_dataSet.size()); // Kopie vom Datensatz anlegen, nur in den neuen wird geschrieben
	for (int i=0; i<(int)_dataSet.size(); i++)
	{
		dataSetNew[i]=OneDataSet(_dataSet[i]);
	}

	StatisticCalculator statisticData;
	StatisticCalculator statisticRank;

	for (int i=firstSet; i<=lastSet; i++)
	{
		statisticData.reset();
		statisticRank.reset();

		for (int j=-lowPassSize/2; j<=lowPassSize/2; j++)
		{
			int index = i+j;
			if (index<firstSet) continue;
			if (index>lastSet) continue;
			statisticData.addValue(_dataSet[index].data);
			statisticRank.addValue(_dataSet[index].data_rank);
		}

		if (useMean)
		{
			dataSetNew[i].data = statisticData.getMean();
			dataSetNew[i].data_rank = round(statisticRank.getMean());
		}
		else // useMedian
		{
			dataSetNew[i].data = statisticData.getMedian();
			dataSetNew[i].data_rank = round(statisticRank.getMedian());
		}
	}

	for (int i=0; i<(int)_dataSet.size(); i++) // neuen Satz zurueckkopieren
	{
		_dataSet[i]=OneDataSet(dataSetNew[i]);
	}
}

void OwnBuffer::interpolateLinearOnRange(int startLastValid, int endFirstValid)
{
	// Sicherheitschecks, dann evtl. nix machen
	if (startLastValid < 0) return;
	if (endFirstValid >= (int)_dataSet.size()) return;

	if (endFirstValid-startLastValid <= 1) return; // Ebenfalls nix machen (ist ja nix da)

	double startData = _dataSet[startLastValid].data;
	double startRank = _dataSet[startLastValid].data_rank;
	double startPos  = _dataSet[startLastValid].pos;

	double dataDiff = _dataSet[endFirstValid].data      - _dataSet[startLastValid].data;
	double rankDiff = _dataSet[endFirstValid].data_rank - _dataSet[startLastValid].data_rank;
	double posDiff  = _dataSet[endFirstValid].pos       - _dataSet[startLastValid].pos;

	double dataDiffPerTick = dataDiff / posDiff;
	double dataRankDiffPerTick = rankDiff / posDiff;

	for (int i=startLastValid+1; i<endFirstValid; i++) // jetzt alle Luecken fuellen
	{
		posDiff = _dataSet[i].pos - startPos; // neudef.
		_dataSet[i].data = startData + posDiff * dataDiffPerTick;
		_dataSet[i].data_rank = round(startRank + posDiff * dataRankDiffPerTick);
	}
}

bool OwnBuffer::cutUselessAreasAtBeginAndEnd()
{
	std::size_t firstValidIndex, lastValidIndex;
	std::size_t index = 0;
    bool indexFound = false;
	if ( _dataSet.size() == 0 )
	{
		return false;
	}

	////////////////////
	// Start analysieren
	if(_dataSet[0].data_rank <= 0) // Taschenbereich vorne
	{
		index=1;
		while (index<_dataSet.size())
		{
			if (_dataSet[index].data_rank>0) // gueltigen Wert gefunden
			{
				firstValidIndex = index;
                indexFound = true;
				break;
			}
			index++;
		}
		if (index >= _dataSet.size()) return false; // Startbereich ist komplett bis Ende ungueltig
	}
	else
	{ // Schweissbereich vorne
		firstValidIndex = 0;
        indexFound = true;
	}

    if (!indexFound)
    {
        return false;
    }

	///////////////////
	// Ende analysieren
	index = _dataSet.size()-1;
    indexFound = false;
	if(_dataSet[index].data_rank <= 0) // Taschenbereich hinten
	{
		index--;
		while (index>=0)
		{
			if (_dataSet[index].data_rank>0) // gueltigen Wert gefunden
			{
				lastValidIndex = index;
                indexFound = true;
				break;
			}
			index--;
		}
		if (index < 0) return false; // Endbereich ist komplett bis Anfang ungueltig

	}
	else
	{ // Schweissbereich hinten
		lastValidIndex = _dataSet.size()-1;
        indexFound = true;
	}

    if (!indexFound)
    {
        return false;
    }

	//////////////////////////////////////////////////////////
	// Anfang und Ende sind analysiert, jetzt Schluesse ziehen
	if (firstValidIndex<lastValidIndex) // alles gut, so soll es sein, erster gueltiger Wert ist kleiner dem letzten
	{
		std::vector<OneDataSet> dataSetNew(_dataSet.size());
		for (int i=0; i<(int)_dataSet.size(); i++)
		{
			dataSetNew[i]=OneDataSet(_dataSet[i]);
		}

		_dataSet.clear();

		for (std::size_t i=firstValidIndex; i<=lastValidIndex; i++)
		{
			_dataSet.push_back(dataSetNew[i]);
		}

		return true;
	}
	else // mit den Daten stimmt was nicht
	{
		return false;
	}
}

bool OwnBuffer::lowPassAndInterpolate(unsigned int mode)
{ // geht davon aus, dass vorne und hinten jeweils nicht-Taschen-Bereiche sind! Rank vorne und hinten muss also != 0 sein!
	bool onValidArea = true;

	int lastChangeIndex = 0;
	int index = 1;

	while (index < (int)_dataSet.size())
	{
		bool isValid = _dataSet[index].data_rank > 0;

		if ( (onValidArea && !isValid) || (onValidArea && (index == (int)_dataSet.size()-1)) )  // Wechsel von gueltig auf nicht-gueltig, Bereich bis dahin lowpassen (oder am Ende)
		{
			if ( (mode == 11) || (mode == 12) ) doMedianOnRange(lastChangeIndex, index-1, _widthMedian);
			if ( (mode == 10) || (mode == 12) ) doMeanOnRange(lastChangeIndex, index-1, _widthMean);
			onValidArea = false;
			lastChangeIndex = index-1;
		}
		else
		if ( (!onValidArea && isValid) ) // Wechsel von nicht-gueltig auf gueltig, Bereich bis dahin interpolieren
		{
			interpolateLinearOnRange(lastChangeIndex, index);
			onValidArea = true;
			lastChangeIndex = index;
		}

		index++;
	}
	return true;
}

StatisticCalculator::StatisticCalculator()
{
	reset();
}

void StatisticCalculator::reset()
{
	_data.clear();
}

void StatisticCalculator::addValue(double value)
{
	_data.push_back(value);
}

double StatisticCalculator::getMedian()
{
	std::size_t size = _data.size();
	if (size==0) return 0;
	if (size==1) return _data[0];

	sortIt();
	if (size%2==0) // Anzahl gerade
	{
		return (_data[size/2] + _data[size/2 -1]) / 2.0;
	}
	else // Anzahl ungerade
	{
		return _data[size/2];
	}
}

double StatisticCalculator::getMean()
{
	std::size_t size = _data.size();
	if (size==0) return 0;
	if (size==1) return _data[0];

	double sum = 0;
	for (std::size_t i=0; i<size; i++) sum += _data[i];
	return sum / size;
}

void StatisticCalculator::sortIt()
{ // Bubble Sort
	std::size_t size = _data.size();
	if (size <= 1) return;
	for (std::size_t i = 0; i<size - 1; i++)
	{
		for (std::size_t j = 0; j<size - i - 1; j++)
		{
			if (_data[j]>_data[j+1]) exchange(_data[j], _data[j+1]);
		}
	}
}

void StatisticCalculator::exchange(double & d1, double & d2)
{
	double d = d1;
	d1 = d2;
	d2 = d;
}


} // namespace filter
} // namespace precitec

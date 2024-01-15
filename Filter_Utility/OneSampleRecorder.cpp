/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2013
 * 	@brief		This filter stores data elements in a buffer. They can be accessed in a different filter graph using the buffer player filter.
 */

// WM includes
#include "OneSampleRecorder.h"
#include <filter/buffer.h>
#include <fliplib/PipeEventArgs.h>
#include <fliplib/Parameter.h>
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include "Poco/Mutex.h"
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
using namespace interface;
namespace filter {

Poco::Mutex OneSampleRecorder::m_oMutex;

double OneSampleRecorder::m_oSampleValue1[MAX_SLOTS_FOR_SAMPLE_RECORDER];
ValueRankType OneSampleRecorder::m_oRank1[MAX_SLOTS_FOR_SAMPLE_RECORDER];
interface::SmpTrafo OneSampleRecorder::m_oTrafo1[MAX_SLOTS_FOR_SAMPLE_RECORDER];
interface::ResultType OneSampleRecorder::m_oAnalysisResult1[MAX_SLOTS_FOR_SAMPLE_RECORDER];
int OneSampleRecorder::m_oImageNumber1[MAX_SLOTS_FOR_SAMPLE_RECORDER];

double OneSampleRecorder::m_oSampleValue2[MAX_SLOTS_FOR_SAMPLE_RECORDER];
ValueRankType OneSampleRecorder::m_oRank2[MAX_SLOTS_FOR_SAMPLE_RECORDER];
interface::SmpTrafo OneSampleRecorder::m_oTrafo2[MAX_SLOTS_FOR_SAMPLE_RECORDER];
interface::ResultType OneSampleRecorder::m_oAnalysisResult2[MAX_SLOTS_FOR_SAMPLE_RECORDER];
int OneSampleRecorder::m_oImageNumber2[MAX_SLOTS_FOR_SAMPLE_RECORDER];

//int OneSampleRecorder::m_oImageCounter[MAX_SLOTS_FOR_SAMPLE_RECORDER];

const std::string OneSampleRecorder::m_oFilterName 		( "OneSampleRecorder" );

OneSampleRecorder::OneSampleRecorder() :
	TransformFilter(OneSampleRecorder::m_oFilterName, Poco::UUID{"2DC49F45-8512-418E-9644-E97DAF3FDBA6"}),
	m_oSlotNumber(1),
	m_oInitialSampleValue(0.0),
	m_pPipeInData( nullptr )
{
	// Setze das Sample auf einen definierten Wert, damit dieser schon zur Verfuegung steht, bevor dieser Filter ueberhaupt schon ausgefuehrt wurde.
	// Offensichtlich wird das hier erst ausgefuehrt, wenn dieser Konstruktor ausgefuehrt wird... aber das ist hoffentlich frueh genug.
	for (int i = 0; i<MAX_SLOTS_FOR_SAMPLE_RECORDER; i++) m_oSampleValue1[i] = m_oInitialSampleValue;
	for (int i = 0; i<MAX_SLOTS_FOR_SAMPLE_RECORDER; i++) m_oRank1[i] = filter::eRankMin;
	for (int i = 0; i<MAX_SLOTS_FOR_SAMPLE_RECORDER; i++) m_oAnalysisResult1[i] = AnalysisOK;
	for (int i = 0; i<MAX_SLOTS_FOR_SAMPLE_RECORDER; i++) m_oTrafo1[i] = Trafo::createIdent();
	for (int i = 0; i<MAX_SLOTS_FOR_SAMPLE_RECORDER; i++) m_oImageNumber1[i] = -1;
	for (int i = 0; i<MAX_SLOTS_FOR_SAMPLE_RECORDER; i++) m_oSampleValue2[i] = m_oInitialSampleValue;
	for (int i = 0; i<MAX_SLOTS_FOR_SAMPLE_RECORDER; i++) m_oRank2[i] = filter::eRankMin;
	for (int i = 0; i<MAX_SLOTS_FOR_SAMPLE_RECORDER; i++) m_oAnalysisResult2[i] = AnalysisOK;
	for (int i = 0; i<MAX_SLOTS_FOR_SAMPLE_RECORDER; i++) m_oTrafo2[i] = Trafo::createIdent();
	for (int i = 0; i<MAX_SLOTS_FOR_SAMPLE_RECORDER; i++) m_oImageNumber2[i] = -2;

	//for (int i = 0; i<MAX_SLOTS_FOR_SAMPLE_RECORDER; i++) m_oImageCounter[i] = 0;

	parameters_.add("SlotNumber", fliplib::Parameter::TYPE_Int32, m_oSlotNumber);

    setInPipeConnectors({{Poco::UUID("2DC49F45-8512-418E-9644-E97DAF3FDBA6"), m_pPipeInData, "Sample", 0, ""}});
    setVariantID(Poco::UUID("2F8F01AE-ACF9-43D1-8DAD-314B11D2CC2B"));
} // CTor.



OneSampleRecorder::~OneSampleRecorder()
{
} // DTor.



void OneSampleRecorder::setParameter()
{
	TransformFilter::setParameter();

	m_oSlotNumber = parameters_.getParameter("SlotNumber").convert<int>();

} // setParameter.


void OneSampleRecorder::GetSampleValue(int index, int currentNumber, double& p_rSample, ValueRankType& p_rRank, interface::SmpTrafo& p_rTrafo, interface::ResultType& p_rResult)
{
	if (index < 0) index = 0;
	if (index > MAX_SLOTS_FOR_SAMPLE_RECORDER - 1) index = MAX_SLOTS_FOR_SAMPLE_RECORDER - 1;
	{
		Poco::Mutex::ScopedLock oLock(m_oMutex);

		if (m_oImageNumber1[index] == currentNumber) // ist Neuerer aus aktuellem Bild? Dann aelteren zurueckgeben
		{
			p_rSample = m_oSampleValue2[index];
			p_rRank = m_oRank2[index];
			p_rTrafo = m_oTrafo2[index];
			p_rResult = m_oAnalysisResult2[index];
			assert(!p_rTrafo.isNull());
			return;
		}

		p_rSample = m_oSampleValue1[index];
		p_rRank = m_oRank1[index];
		p_rTrafo = m_oTrafo1[index];
		p_rResult = m_oAnalysisResult1[index];
		assert(!p_rTrafo.isNull());

		// erster Versuch, ging so nicht wegen Rueckspruengen in der Simulation
		//if (m_oImageNumber1[index] == currentNumber)
		//// Der Slot1 wurde beim aktuellen Bild erst gefuellt.  Damit ist das nicht der, der zurueckgegeben werden sollte.
		//{
		//	p_rSample = m_oSampleValue2[index];
		//	p_rRank = m_oRank2[index];
		//	p_rTrafo = m_oTrafo2[index];
		//	p_rResult = m_oAnalysisResult2[index];
		//	assert(!p_rTrafo.isNull());
		//	return;
		//}

		//if (m_oImageNumber2[index] == currentNumber)
		//// Der Slot2 wurde beim aktuellen Bild erst gefuellt.  Damit ist das nicht der, der zurueckgegeben werden sollte.
		//{
		//	p_rSample = m_oSampleValue1[index];
		//	p_rRank = m_oRank1[index];
		//	p_rTrafo = m_oTrafo1[index];
		//	p_rResult = m_oAnalysisResult1[index];
		//	assert(!p_rTrafo.isNull());
		//	return;
		//}

		//// Weder Slot1 noch Slot2 wurden beim aktuellen Bild gefuellt, also neueren zurueckgeben

		//if (m_oImageNumber1[index] < m_oImageNumber2[index]) // Slot 1 ist aelter, also Slot2 zurueck
		//{
		//	p_rSample = m_oSampleValue2[index];
		//	p_rRank = m_oRank2[index];
		//	p_rTrafo = m_oTrafo2[index];
		//	p_rResult = m_oAnalysisResult2[index];
		//	assert(!p_rTrafo.isNull());
		//}
		//else
		//{
		//	p_rSample = m_oSampleValue1[index];
		//	p_rRank = m_oRank1[index];
		//	p_rTrafo = m_oTrafo1[index];
		//	p_rResult = m_oAnalysisResult1[index];
		//	assert(!p_rTrafo.isNull());
		//}
	}
}

void OneSampleRecorder::arm(const fliplib::ArmStateBase& state)
{
	if (state.getStateID() == eSeamStart)
	{
		// muss bei jedem SeamStart neu initialisiert werden:
		Poco::Mutex::ScopedLock oLock(m_oMutex);
		for (int i = 0; i<MAX_SLOTS_FOR_SAMPLE_RECORDER; i++) m_oSampleValue1[i] = m_oInitialSampleValue;
		for (int i = 0; i<MAX_SLOTS_FOR_SAMPLE_RECORDER; i++) m_oRank1[i] = filter::eRankMin;
		for (int i = 0; i<MAX_SLOTS_FOR_SAMPLE_RECORDER; i++) m_oAnalysisResult1[i] = AnalysisOK;
		for (int i = 0; i<MAX_SLOTS_FOR_SAMPLE_RECORDER; i++) m_oTrafo1[i] = Trafo::createIdent();
		for (int i = 0; i<MAX_SLOTS_FOR_SAMPLE_RECORDER; i++) m_oImageNumber1[i] = -1;

		for (int i = 0; i<MAX_SLOTS_FOR_SAMPLE_RECORDER; i++) m_oSampleValue2[i] = m_oInitialSampleValue;
		for (int i = 0; i<MAX_SLOTS_FOR_SAMPLE_RECORDER; i++) m_oRank2[i] = filter::eRankMin;
		for (int i = 0; i<MAX_SLOTS_FOR_SAMPLE_RECORDER; i++) m_oAnalysisResult2[i] = AnalysisOK;
		for (int i = 0; i<MAX_SLOTS_FOR_SAMPLE_RECORDER; i++) m_oTrafo2[i] = Trafo::createIdent();
		for (int i = 0; i<MAX_SLOTS_FOR_SAMPLE_RECORDER; i++) m_oImageNumber2[i] = -2;

		//for (int i = 0; i<MAX_SLOTS_FOR_SAMPLE_RECORDER; i++) m_oImageCounter[i] = 0;
	}

	if(m_oVerbosity >= eHigh)
	{
		wmLog(eDebug, "Filter '%s' armed at armstate %i\n", m_oFilterName.c_str(),  state.getStateID());
	}
} // arm



bool OneSampleRecorder::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	m_pPipeInData = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



void OneSampleRecorder::proceed(const void* p_pSender, fliplib::PipeEventArgs& e)
{
	poco_assert_dbg( m_pPipeInData != nullptr); // to be asserted by graph editor

	// data
	const GeoDoublearray &rGeoDoubleArrayIn = m_pPipeInData->read(m_oCounter);
	m_oCurrentNumber = rGeoDoubleArrayIn.context().imageNumber();

	int index = m_oSlotNumber - 1;
	if (index < 0) index = 0;
	if (index > MAX_SLOTS_FOR_SAMPLE_RECORDER - 1) index = MAX_SLOTS_FOR_SAMPLE_RECORDER - 1;

	{  // fuer ScopedLock
		Poco::Mutex::ScopedLock oLock(m_oMutex);
		//m_oImageCounter[index]++;

		// alt, funktioniert so nicht wegen Rueckspruengen in der Simulation
		//if (m_oImageNumber1[index] < m_oImageNumber2[index]) // immer den aelteren fuellen
		//{
		//	temp = m_oSampleValue1[index] = std::get<eData>(rGeoDoubleArrayIn.ref()[0]);
		//	m_oRank1[index] = static_cast<ValueRankType>(std::get<eRank>(rGeoDoubleArrayIn.ref()[0]));
		//	m_oAnalysisResult1[index] = rGeoDoubleArrayIn.analysisResult();
		//	m_oTrafo1[index] = rGeoDoubleArrayIn.context().trafo();
		//	m_oImageNumber1[index] = m_oCurrentNumber;
		//}
		//else // 2 ist aelter
		//{
		//	temp = m_oSampleValue2[index] = std::get<eData>(rGeoDoubleArrayIn.ref()[0]);
		//	m_oRank2[index] = static_cast<ValueRankType>(std::get<eRank>(rGeoDoubleArrayIn.ref()[0]));
		//	m_oAnalysisResult2[index] = rGeoDoubleArrayIn.analysisResult();
		//	m_oTrafo2[index] = rGeoDoubleArrayIn.context().trafo();
		//	m_oImageNumber2[index] = m_oCurrentNumber;
		//}

		// neuer Versuch
		m_oSampleValue2[index] = m_oSampleValue1[index];
		m_oRank2[index] = m_oRank1[index];
		m_oAnalysisResult2[index] = m_oAnalysisResult1[index];
		m_oTrafo2[index] = m_oTrafo1[index];
		m_oImageNumber2[index] = m_oImageNumber1[index];

		m_oSampleValue1[index] = std::get<eData>(rGeoDoubleArrayIn.ref()[0]);
		m_oRank1[index] = static_cast<ValueRankType>(std::get<eRank>(rGeoDoubleArrayIn.ref()[0]));
		m_oAnalysisResult1[index] = rGeoDoubleArrayIn.analysisResult();
		m_oTrafo1[index] = rGeoDoubleArrayIn.context().trafo();
		m_oImageNumber1[index] = m_oCurrentNumber;
	}  // ScopedLock

	// some debug output
	if( m_oVerbosity >= eHigh )
	{
		wmLog(eInfo, "OneSampleRecorder: Storing %f !\n", m_oRank1[index]);
	}
    preSignalAction();
} // proceedGroup


} // namespace filter
} // namespace precitec

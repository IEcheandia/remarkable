/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			kir, ab, hs
 *  @date			2009
 *  @brief			Filtergraph Visitoren zur Konfiguration / Steuerung der Filter
 */

#ifndef GRAPHVISITORS_H_
#define GRAPHVISITORS_H_

#include "Poco/UUID.h"
#include "fliplib/BaseFilterInterface.h"
#include "fliplib/BaseFilter.h"
#include "fliplib/SourceFilter.h"
#include "fliplib/AbstractFilterVisitor.h"
#include "fliplib/SynchronePipe.h"
#include "common/frame.h"
#include "overlay/overlayCanvas.h"
#include "common/graph.h"

#include "Mod_Analyzer.h"
#include "event/results.proxy.h"
#include "analyzer/resultHandler.h"
#include "filter/armStates.h"


namespace precitec 	{
namespace analyzer	{

/**
 * Jedem Filter wird die Dispose Methode aufgerufen
 *
 */
class MOD_ANALYZER_API FilterDisposer : public fliplib::AbstractFilterVisitor
{
public:
	void control (fliplib::FilterControlInterface& filter);
};

/**
 * Jedem Filter wird die Init Methode aufgerufen
 *
 */
class MOD_ANALYZER_API FilterInitalizer : public fliplib::AbstractFilterVisitor
{
public:
	void control (fliplib::FilterControlInterface& filter);
};


/**
 * Jedem Filter wird die arm(const ArmStateBase& state) Methode aufgerufen
 *
 */
class MOD_ANALYZER_API FilterArm : public fliplib::AbstractFilterVisitor
{
public:
	FilterArm (filter::ArmState p_oArmState);
	void control (fliplib::FilterControlInterface& filter);
private:
	const filter::ArmState m_oArmState;
};

/**
 * Diese Visitor Klasse ueberprueft ob Parameter im Filter aktualiesiert worden sind. Wenn ja, wird der Filter benachrichtigt.
 *
 **/
class MOD_ANALYZER_API ParameterSetter : public fliplib::AbstractFilterVisitor
{
public:
	ParameterSetter() {}

	void control (fliplib::FilterControlInterface& filter);
};

/**
 * Diese Visitor Klasse setzt einen Parametersatz.
 *
 **/
class MOD_ANALYZER_API ParameterSetSetter : public fliplib::AbstractFilterVisitor
{
public:
	ParameterSetSetter(const interface::paramSet_t &p_rParamSet) : m_rParamSet(p_rParamSet) {}

	void control (fliplib::FilterControlInterface& filter);

private:
	const interface::paramSet_t		&m_rParamSet;
};

/**
 * Diese Visitor Klasse liefert die sensor ids aller source filter.
 *
 **/
class MOD_ANALYZER_API SensorIdGetter : public fliplib::AbstractFilterVisitor
{
public:
	void control (fliplib::FilterControlInterface& p_rFilter);

	std::vector<int> getSensorIds() const { return sensorIds_; }
private:
	std::vector<int>	sensorIds_;	// sensor ids of all  source filters
};

/**
 * Diese Visitor Klasse liefert die filter ids aller filter.
 *
 **/
class MOD_ANALYZER_API FilterIdGetter : public fliplib::AbstractFilterVisitor
{
public:
    void control (fliplib::FilterControlInterface& p_rFilter);

    std::vector<Poco::UUID> getFilterIds() const {return m_filterIds;}
private:
    std::vector<Poco::UUID> m_filterIds;
};

/**
*	@brief		Kind of source filters that may be used in a graph. NB: Values are OR-ed.
*/
enum SourceFilterType {
	eNone					= 0, 
	eImage					= 1, 
	eSample					= 2, 
	eImageAndSample			= 3,
	eSourceFilterTypeMin	= eNone,				///< delimiter
	eSourceFilterTypeMax	= eImageAndSample		///< delimiter
}; // SourceFilterType

/**
 * Diese Visitor Klasse liefert die sensor ids und typen aller source filter.
 *
 **/
class MOD_ANALYZER_API SourceFilterTypesGetter : public fliplib::AbstractFilterVisitor
{
public:
	SourceFilterTypesGetter() : m_oSourceFilterUsed(eNone) {}
	void control (fliplib::FilterControlInterface& p_rFilter);
	SourceFilterType getSourceFilterTypes() const { return m_oSourceFilterUsed; }
private:
	SourceFilterType	m_oSourceFilterUsed;
	std::vector<int>	m_oSensorIds;
}; // SourceFilterTypesGetter

/**
 * Diese Visitor Klasse liefert die sensor ids und typen aller source filter.
 *
 **/
class MOD_ANALYZER_API SourceFilterTypesAndIdsGetter : public fliplib::AbstractFilterVisitor
{
public:
	SourceFilterTypesAndIdsGetter()=default;
	void control (fliplib::FilterControlInterface& p_rFilter);
	const std::vector<int>& getImageSourceFilterIds() const { return m_oSensorIdsImage; }
    const std::vector<int>& getSampleSourceFilterIds() const { return m_oSensorIdsSample; }
private:
	std::vector<int>	m_oSensorIdsImage;
    std::vector<int>	m_oSensorIdsSample;
}; // SourceFilterTypesAndIdsGetter
/**
 * Dieser Visitor ruft die Paint Methode aller Filter auf
 *
 **/
class MOD_ANALYZER_API PaintVisitor : public fliplib::AbstractFilterVisitor
{
public:
	PaintVisitor() {}

	void control (fliplib::FilterControlInterface& filter);
};




/**
 * Diese Visitor Klasse setzt ein Canvas.
 *
 **/
class MOD_ANALYZER_API CanvasSetter : public fliplib::AbstractFilterVisitor
{
public:
	CanvasSetter(image::OverlayCanvas *canvas) : m_pCanvas(canvas) {}

	void control (fliplib::FilterControlInterface& filter);

private:
	image::OverlayCanvas *m_pCanvas;
};



/**
 * Diese Visitor Klasse setzt produktdaten.
 *
 **/
class MOD_ANALYZER_API ExternalDataSetter : public fliplib::AbstractFilterVisitor
{
public:
	ExternalDataSetter(const fliplib::ExternalData& p_rExternalData) : m_rExternalData(p_rExternalData) {}
	void control (fliplib::FilterControlInterface& filter);

private:
	const fliplib::ExternalData& m_rExternalData;
};


/**
 * Diese Visitor Klasse verbindet Resultfilter mit einem ResultHandler.
 *
 **/
class MOD_ANALYZER_API ResultHandlerConnector : public fliplib::AbstractFilterVisitor
{
public:
	ResultHandlerConnector(fliplib::SinkFilter &resultHandler) : resultHandler_(resultHandler) {}

	void control (fliplib::FilterControlInterface& filter);

private:
	fliplib::SinkFilter &resultHandler_;
};


/**
 * Diese Visitor Klasse trennt alle Result-/Niofilter von einem ResultHandler.
 *
 **/
class MOD_ANALYZER_API ResultHandlerReleaser : public fliplib::AbstractFilterVisitor
{
public:
    ResultHandlerReleaser(fliplib::SinkFilter &resultHandler) : resultHandler_(resultHandler) {}

	void control (fliplib::FilterControlInterface& filter);

private:
	fliplib::SinkFilter &resultHandler_;
};

/**
 * Diese Visitor Klasse sucht nach allen  Filtern und versucht diese mit der Pipe zu verknuepfen
 *
 * T kann ImageFrame, TapeFrame, SignalFrame etc sein
 *
 **/
template <class T>
class MOD_ANALYZER_API PipeConnector : public fliplib::AbstractFilterVisitor
{
public:
	PipeConnector(
		fliplib::BaseFilterInterface::FilterType filterType,
		fliplib::SynchronePipe< T > * inputPipe) :
		filterType_(filterType),
		inputPipe_(inputPipe)
		{}

	void control (fliplib::FilterControlInterface& filter)
	{
		fliplib::BaseFilter& baseFilter = static_cast< fliplib::BaseFilter& >( filter );
		if (baseFilter.getFilterType() == filterType_)
		{
			baseFilter.connectPipe(inputPipe_, 0);
		}
	}

private:
	fliplib::BaseFilterInterface::FilterType filterType_;
	fliplib::SynchronePipe< T >* inputPipe_;
};

template <class T>
class MOD_ANALYZER_API PipeDisconnector : public fliplib::AbstractFilterVisitor
{
public:
	PipeDisconnector(
		fliplib::BaseFilterInterface::FilterType filterType,
		fliplib::SynchronePipe< T > * inputPipe) :
		filterType_(filterType),
		inputPipe_(inputPipe)
		{}

	void control (fliplib::FilterControlInterface& filter)
	{
		fliplib::BaseFilter& baseFilter = static_cast< fliplib::BaseFilter& >( filter );
		if (baseFilter.getFilterType() == filterType_)
		{
			baseFilter.disconnectPipe(inputPipe_);
		}
	}

private:
	fliplib::BaseFilterInterface::FilterType filterType_;
	fliplib::SynchronePipe< T >* inputPipe_;
};


/**
 * Diese Visitor Klasse sucht nach Source Filtern und versucht diese mit der Pipe zu verknuepfen,
 * wenn die SensorID identisch ist
 *
 * T kann ImageFrame, TapeFrame, SignalFrame etc sein
 *
 **/
template <class T>
class MOD_ANALYZER_API SensorConnector : public fliplib::AbstractFilterVisitor
{
public:
	SensorConnector(
		fliplib::SynchronePipe< T > * inputPipe, int sensorID) :
			inputPipe_(inputPipe),
			sensorID_(sensorID)
		{}

	void control (fliplib::FilterControlInterface& filter)
	{
		fliplib::SourceFilter& baseFilter = static_cast< fliplib::SourceFilter& >( filter );
		if (baseFilter.getFilterType() == fliplib::BaseFilterInterface::SOURCE &&
			(baseFilter.getSensorID() == sensorID_ || baseFilter.getSensorID() == -1))
		{
			int nogroup = 0;
			baseFilter.connectPipe(inputPipe_, nogroup);
				
		}
	}

private:
	fliplib::SynchronePipe< T >* inputPipe_;
	int sensorID_;
};



class MOD_ANALYZER_API SensorDisconnector : public fliplib::AbstractFilterVisitor
{
public:
	SensorDisconnector(fliplib::BasePipe * inputPipe) :
			inputPipe_(inputPipe)
		{}

	void control (fliplib::FilterControlInterface& filter)
	{
		fliplib::SourceFilter& baseFilter = static_cast< fliplib::SourceFilter& >( filter );
		if (baseFilter.getFilterType() == fliplib::BaseFilterInterface::SOURCE)
		{
			//if (baseFilter.findPipe(inputPipe_->name())) //I'll never find it among the output pipes!
                bool disconnected = baseFilter.disconnectPipe(inputPipe_);
				assert(disconnected);
				// silence warning in release build
				(void)disconnected;

		}
	}

private:
	fliplib::BasePipe* inputPipe_;
	int sensorID_;
};



/**
 * Dieser Visitor setzt den sync zaehler aller Filter
 *
 **/
class MOD_ANALYZER_API SetCounterVisitor : public fliplib::AbstractFilterVisitor
{
public:
	SetCounterVisitor(int p_oCount);

	void control (fliplib::FilterControlInterface& p_rFilter);

private:
	fliplib::BasePipe* inputPipe_;
	const int m_oCount;
};

/**
 * This visitor invokes BaseFilter::resetSignalCountGroupEvent with the passed in image number.
 */
class MOD_ANALYZER_API ResetGroupEventCounterVisitor : public fliplib::AbstractFilterVisitor
{
public:
	ResetGroupEventCounterVisitor(int imageNumber)
		: m_imageNumber(imageNumber)
	{
	}

	void control(fliplib::FilterControlInterface &filter) override
	{
		auto &baseFilter = static_cast<fliplib::BaseFilter &>(filter);
		baseFilter.resetSignalCountGroupEvent(m_imageNumber);
	}

private:
	const int m_imageNumber;
};

/**
 * This visitor synchronizes all source filters on the passed in image number.
 */
class MOD_ANALYZER_API SynchronizeSourceFiltersVisitor : public fliplib::AbstractFilterVisitor
{
public:
    SynchronizeSourceFiltersVisitor(int imageNumber)
        : m_imageNumber(imageNumber)
    {
    }

    void control(fliplib::FilterControlInterface &filter) override;
private:
    const int m_imageNumber;
};

/**
 * This visitor invokes logProcessingTime on all filters.
 **/
class MOD_ANALYZER_API LogProcessingTimeVisitor : public fliplib::AbstractFilterVisitor
{
    void control(fliplib::FilterControlInterface &filter) override
    {
        static_cast<fliplib::BaseFilter &>(filter).logProcessingTime();
    }
};

/**
 * This visitor ensures that all filters have at least a given image number.
 **/
class MOD_ANALYZER_API EnsureImageNumberVisitor : public fliplib::AbstractFilterVisitor
{
public:
    EnsureImageNumberVisitor(int imageNumber)
        : m_imageNumber(imageNumber)
    {
    }

    void control(fliplib::FilterControlInterface &filter) override
    {
        static_cast<fliplib::BaseFilter &>(filter).ensureImageNumber(m_imageNumber);
    }

private:
    const int m_imageNumber;
};

class MOD_ANALYZER_API SkipImageProcessingVisitor : public fliplib::AbstractFilterVisitor
{
public:
    SkipImageProcessingVisitor(int imageNumber)
        : m_imageNumber(imageNumber)
    {
    }

    void control(fliplib::FilterControlInterface &filter) override;

private:
    const int m_imageNumber;
};


/**
* Enumerate the filters in graph
*/
class MOD_ANALYZER_API ResetFilterIndexesVisitor : public fliplib::AbstractFilterVisitor
{
public:
	   ResetFilterIndexesVisitor()
    {}

	void control (fliplib::FilterControlInterface& p_rFilter);
    private:
        int m_graphIndex = 0;

};

/**
* Set the flag always enable timings
*/
class MOD_ANALYZER_API SetAlwaysEnableTimingsVisitor : public fliplib::AbstractFilterVisitor
{
public:
	SetAlwaysEnableTimingsVisitor(bool p_alwaysEnableTiming):
	       m_oAlwaysEnableTiming(p_alwaysEnableTiming)
    {}

	void control (fliplib::FilterControlInterface& p_rFilter);

private:
	 const bool m_oAlwaysEnableTiming;
};

class MOD_ANALYZER_API CountProcessedImagesVisitor : public fliplib::AbstractFilterVisitor
{
public:
	CountProcessedImagesVisitor()
    {}
    void control (fliplib::FilterControlInterface& p_rFilter);
    const std::map<int,int> & getProcessedImagesCounter() const 
    {
        return m_oProcessedImagesCounter;
    }

private:
	 std::map<int,int> m_oProcessedImagesCounter; // key: imageNumber; value: number of filter having m_oCounter == imageNumber
};


// free helper function to add parameters to a filter
void addParametersToFilter(const interface::ParameterList& p_rParameterList, fliplib::BaseFilter& p_rFilter);


        

} // namespace analyzer
} // namespace precitec

#endif /*GRAPHVISITORS_H_*/

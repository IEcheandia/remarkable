/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			kir, ab, hs
 *  @date			2009
 *  @brief			Der Graph Assistent stellt Methoden zur Arm / Steuerung der Graphen zur Verfuegung
 */

#ifndef GRAPHASSISTENT_H_
#define GRAPHASSISTENT_H_

#include "Poco/UUID.h"

#include "fliplib/AbstractFilterVisitor.h"
#include "fliplib/FilterGraph.h"
#include "fliplib/SynchronePipe.h"
#include "fliplib/SinkFilter.h"

#include "overlay/overlayCanvas.h"
#include "common/graph.h"
#include "message/db.interface.h"
#include "event/results.proxy.h"
#include "fliplib/FilterControlInterface.h"

#include "graphVisitors.h"


namespace precitec {
namespace analyzer {

	// Der Graph Assistent stellt Methoden zur Arm / Steuerung der Graphen zur Verfuegung
	class MOD_ANALYZER_API GraphAssistent
	{
	public:
		GraphAssistent( fliplib::FilterGraph* graph ) : graph_(graph) {
			poco_check_ptr(graph_);
		}

	public:
		// Initalisiert die Filter
		void init();

		// Gibt FilterResourcen frei
		void dispose();

		// Parameter in den Filtern initalisiert oder sie haben sich geaendert
		void setParameter();

		bool setFilterGraph(fliplib::FilterGraph* graph);

		// Parameter in einem Filter aus DB updaten
		void changeParameterFilterDb(interface::TDb<interface::AbstractInterface>& p_rDbProxy, const Poco::UUID& p_rMeasureTaskId, const Poco::UUID& p_rInstFilterId);

		// schalte einen parametersatz scharf
		void setParameterSet( const Poco::UUID& p_rParamSetId, const interface::paramSetMap_t& p_rParamSetMap);
        void delayedSetParameterSet( const Poco::UUID& p_rParamSetId, const interface::paramSetMap_t& p_rParamSetMap);

		// Der Filter bekommt einen Canvas zugewiesen
		void setCanvas( image::OverlayCanvas* p_pCanvas );
        void delayedSetCanvas( image::OverlayCanvas* p_pCanvas );

		// Der Filter bekommt externe Daten zugewiesen
		void setExternalData(const fliplib::ExternalData& p_rExternalData);
        void delayedSetExternalData(const fliplib::ExternalData& p_rExternalData);

		// liefert die sensor ids aller source filter
		std::vector<int> getSensorIds();

        //returns, ids of all filters
        std::vector<Poco::UUID> getFilterIds();

		// returns, which kind of source filters is used in the graph
		SourceFilterType getSourceFilterTypes();

		// returns, which kind of source filters and sensor ids are used in the graph
		std::tuple<std::vector<int>, std::vector<int>> getSourceFilterTypesAndIds();

		// fordert die Filter auf in den Canvas zu zeichen.
		void paint();

		// Saemtliche Resultatfilter werden mit dem ResultHandler verlinkt
		void setResultHandler(fliplib::SinkFilter &handler);
		void releaseResultHandler(fliplib::SinkFilter &handler);

		template<class T>
		void setInputPipe( int sensorID, fliplib::SynchronePipe<T> * inPipe )
		{
			SensorConnector<T> visitor ( inPipe, sensorID );
			apply( visitor );
		}

		void releaseInputPipe( fliplib::BasePipe * inPipe )
		{
			SensorDisconnector visitor( inPipe );
			apply( visitor );
		}

		void arm(filter::ArmState p_oArmState);
        void delayedArm(filter::ArmState p_oArmState);

		// startet die Verarbeitung des Graphen
		void fire();

        void setCount(int p_oCount);
        void delayedSetCount(int p_oCount);

        /**
         * Synchronizes the graph on @p imageNumber.
         * With parallel processing this will block till all source filters reached @p imageNumber
         * in their synchronization mechanism.
         *
         * In case parallelization is disabled the method does not synchronize and does not block.
         * @param imageNumber The image number to synchronize with
         */
        void synchronizeSourceFilters(int imageNumber);
        
        /**
         * Logs the processing time of all filters in the graph which have a maximum verbosity.
         **/
        void logProcessingTime();
        void delayedLogProcessingTime();

        /**
         * Updates the image number in all filters to at least @p imageNumber.
         *
         * This is intended for the case that not all filters were processed. In order to process
         * the next frame without blocking the Filter's image counter needs to be increased.
         * As the next image might have already been processed in a filter we cannot just set the
         * Filter's image number to @p imageNumber. Instead we take the minimum.
         **/
        void ensureImageNumber(int imageNumber);
        
        std::map<int,int> getProcessedImagesCounter(); 
        void logProcessedImagesCount(precitec::LogType p_oLogType);

        //utility function to log the output of getProcessedImagesCounter
        static void logProcessedImagesCount(precitec::LogType p_oLogType, const std::map<int,int> & p_rProcessedImagesCounter);

        void apply();

	private:
		// Wendet den Visitor fuer alle Filter im Graphen an
		void apply (fliplib::AbstractFilterVisitor& visitor);

        std::unique_ptr<fliplib::AbstractFilterVisitor> prepareSetParameterSet(const Poco::UUID& p_rParamSetId, const interface::paramSetMap_t& p_rParamSetMap);
        std::unique_ptr<fliplib::AbstractFilterVisitor> prepareSetCanvas(image::OverlayCanvas* p_pCanvas) const;
        std::unique_ptr<fliplib::AbstractFilterVisitor> prepareSetExternalData(const fliplib::ExternalData& p_rExternalData) const;
        std::unique_ptr<fliplib::AbstractFilterVisitor> prepareCount(int p_oCount) const;
        std::unique_ptr<fliplib::AbstractFilterVisitor> prepareArm(filter::ArmState p_oArmState) const;

		fliplib::FilterGraph* graph_;

        std::list<std::unique_ptr<fliplib::AbstractFilterVisitor>> m_delayedVisitors;
	};


    /**
     * Sorgt dafuer, dass die Pipe sauber released wird
     *
     * Warning: this does not work correctly. If there are multiple Filters with different sensorIDs
     * the setup will work as expected, but the cleanup is performed for all Filters, no matter whether
     * it's for the sensor or not. This results in pipes still connected in worst case.
     **/
	template<class T>
	class PipeScope
	{
	public:
		PipeScope(fliplib::FilterGraph* graph, int sensorID, fliplib::SynchronePipe<T> * inPipe) :
			assistent_( graph ),
			sensorID_(sensorID),
			inPipe_(inPipe)

		{
			assistent_.setInputPipe<T>( sensorID_, inPipe_ );
		}

		~PipeScope()
		{
			assistent_.releaseInputPipe( inPipe_ );
		}

	private:
		GraphAssistent assistent_;
		int sensorID_;
		fliplib::SynchronePipe<T> * inPipe_;

	};


} // namespace analyzer
} // namespace precitec

#endif /*GRAPHASSISTENT_H_*/

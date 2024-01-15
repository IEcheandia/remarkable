/*!
 *  \n Copyright:	Precitec Vision GmbH & Co. KG
 *  \n Project:		WM
 *  \author			Ralph Kirchner (KiR), Andreas Beschorner (AB)
 *  \date			2011
 *  \file			resultHandler.h
 *  \brief			The resultmanager is connected to the (invisible) out-pipes of the result filters and is responsible to send the data to windows.
 */

#ifndef RESULTHANDLER_H_
#define RESULTHANDLER_H_

#include "Poco/Mutex.h"
#include "Poco/ThreadLocal.h"

#include "geo/range.h"

#include "fliplib/Fliplib.h"
#include "fliplib/AbstractFilterVisitor.h"
#include "fliplib/FilterGraph.h"
#include "fliplib/SynchronePipe.h"
#include "fliplib/SinkFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"
#include "sumError.h"
#include "product.h"

#include "event/results.proxy.h"
#include "event/results.interface.h"
#include "event/results.h"
#include "common/defines.h"

#include "module/moduleLogger.h"

#include "Mod_Analyzer.h"
#include "workflow/stateMachine/abstractState.h"   // for enum State and access to states!

#include <vector>
#include <array>

namespace precitec
{
namespace analyzer
{

	/// Der ResultHandler ist ein Filter der die Resultate der ResultFilter empfaengt und
	/// via ResultProxy an die Oberflaeche schickt. Zusaetzlich wird jedes Result mit dem
	/// TaskContext erweitert.
	class MOD_ANALYZER_API ResultHandler : public fliplib::SinkFilter
	{
	public:
		ResultHandler(interface::TResults<interface::AbstractInterface>  *resultProxy);

		void setProxy(interface::TResults<interface::AbstractInterface>  *resultProxy) { resultProxy_ = resultProxy; }
		interface::TResults<interface::AbstractInterface>* proxy() { return resultProxy_; } // necessary for product
		void setProduct(const analyzer::Product* p_pProduct);
		workflow::State state() { return m_oState; }
		void setState(workflow::State p_oState);
		// NIO-received getter. Resets flag. Slot for parallel processing.
		bool nioReceived(std::size_t p_oSlot);
		interface::ResultDoubleArray sendResult( interface::ResultDoubleArray& p_rRes, interface::ResultDoubleArray lwmTriggerResult = {});

		int lastImageProcessed() const
		{
			return m_lastImageProcessed;
		}
		/**
		 * Sets the last image processed counter. This should be the same value as setCounter.
		 */
		void setLastImageProcessed(int counter)
		{
			// incrementation needs to be one behind the m_counter, thus decrement one
			m_lastImageProcessed = counter -1;
		}
		void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);

        /**
         * Informs the ResultHandler that a new Filter Graph run is started and an image/sample
         * is being processed. After calling startProcessing @link{isProcessing} returns @c true.
         * Once the ResultHandler went into proceedGroup @link{isProcessing} returns @c false again.
         *
         * @see isProcessing
         */
        void startProcessing();

        /**
         * @returns Whether the ResultHandler is waiting for proceedGroup of a filter graph run.
         * This method is per thread.
         */
        bool isProcessing() const;

	protected:
		workflow::State m_oState;     // state machine state.

	private:
		void sendResultAsIOAndValid( interface::ResultDoubleArray &resultArgs );
		void setResultDeviationfromSumErrorLimits( interface::ResultDoubleArray &resultArgs, SmpSumError &sumError );
		void setResultDeviationGivenFromLowestScope( tSumErrorList &summErrorList, interface::ResultDoubleArray &resultArgs );

		interface::TResults<interface::AbstractInterface> *resultProxy_;
		const analyzer::Product* m_pProduct;

		std::array<bool, g_oNbParMax> m_oNioReceived;
		int m_lastImageProcessed;
		Poco::ThreadLocal<bool> m_processing;
	};

}
}

#endif /*RESULTHANDLER_H_*/

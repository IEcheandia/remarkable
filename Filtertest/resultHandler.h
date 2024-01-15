/*!
 *  \n Copyright:	Precitec Vision GmbH & Co. KG
 *  \n Project:		WM
 *  \author			Ralph Kirchner (KiR), Andreas Beschorner (AB)
 *  \date			2011
 *  \file			resultHandler.h
 *  \brief			The resultmanager is connected to the (invisible) out-pipes of the result filters and is responsible to send the data to windows.
 */

#ifndef RESULTHANDLER_H_20131218_INCLUDED
#define RESULTHANDLER_H_20131218_INCLUDED

#include "fliplib/SynchronePipe.h"
#include "fliplib/SinkFilter.h"
#include "Poco/Semaphore.h"
#include <atomic>

const int MAX_ELEMENTS_TO_EXPORT = 500;

namespace fliplib {
//#include "fliplib/PipeEventArgs.h"
	class PipeEventArgs;
}

namespace precitec {
namespace filter {

	/// Der ResultHandler ist ein Filter der die Resultate der ResultFilter empfaengt und
	/// via ResultProxy an die Oberflaeche schickt. Zusaetzlich wird jedes Result mit dem
	/// TaskContext erweitert.
	class ResultHandler : public fliplib::SinkFilter {
	public:
		ResultHandler();
		void waitForResult(int p_oNbResultsExpected);
        std::size_t getResultCnt();
        void setResultFolder(std::string folder) {m_oResultFolder = folder;}
        std::string getResultFolder() const {return m_oResultFolder;}

	protected:
		void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);

	private:
        Poco::Semaphore	            m_WaitForResultsSema;
        std::atomic<int>    		m_oResultCnt;
        int                         m_oNbResultsExpected;
        std::string     m_oResultFolder;
	};
} // namespace filter
} // namespace precitec

#endif /*RESULTHANDLER_H_20131218_INCLUDED*/

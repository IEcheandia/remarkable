#include <fliplib/SinkFilter.h>
#include <filter/sensorFilterInterface.h>

class DummyResultHandler : public fliplib::SinkFilter
{
public:
    DummyResultHandler() : fliplib::SinkFilter("dummy") {}
	void proceed(const void * sender, fliplib::PipeEventArgs & e)
    {
        Q_UNUSED(sender)
        Q_UNUSED(e)
        /*
        for (auto pInPipe : m_oInPipes)
        {
            wmLog( eDebug, "CalibrationResultHandler::proceed[%i]\n", m_oCounter);

            interface::ResultArgs *oRes = new interface::ResultArgs(pInPipe->read(m_oCounter));
            m_oResults.push_back( oRes );
        }
        */

        preSignalAction();
        m_proceedCalled = true;
    }

    bool isProceedCalled() const
    {
        return m_proceedCalled;
    }

private:
    bool m_proceedCalled = false;
    std::vector< precitec::interface::ResultArgs* > m_oResults;		///< Buffer with the results.

};


#pragma once

#include "abstractModule.h"

#include "message/simulationCmd.proxy.h"
#include "event/storageUpdate.proxy.h"
#include "message/db.proxy.h"
#include "event/triggerCmd.proxy.h"
#include "event/results.proxy.h"
#include "event/recorder.proxy.h"
#include "event/systemStatus.proxy.h"
#include "message/calibration.proxy.h"
#include "event/inspectionOut.proxy.h"
#include "event/productTeachIn.proxy.h"
#include "message/weldHead.proxy.h"
#include "event/videoRecorder.proxy.h"

namespace precitec
{
namespace gui
{



using namespace interface;
typedef std::shared_ptr<TSimulationCmd<MsgProxy>> SimulationCmdProxy;
typedef std::shared_ptr<TStorageUpdate<AbstractInterface>> StorageUpdateProxy;
typedef std::shared_ptr<TDb<MsgProxy>>  DbProxy;
typedef std::shared_ptr<TTriggerCmd<EventProxy>> TriggerCmdProxy;
typedef std::shared_ptr<TResults<EventProxy>> ResultsProxy;
typedef std::shared_ptr<TRecorder<EventProxy>> RecorderProxy;
typedef std::shared_ptr<TSystemStatus<EventProxy>> SystemStatusProxy;
typedef std::shared_ptr<TCalibration<MsgProxy>> CalibrationProxy;
typedef std::shared_ptr<TInspectionOut<EventProxy>> InspectionOutProxy;
typedef std::shared_ptr<TProductTeachIn<EventProxy>> ProductTeachInProxy;
typedef std::shared_ptr<TWeldHeadMsg<MsgProxy>> WeldHeadMsgProxy;
typedef std::shared_ptr<TVideoRecorder<EventProxy>> VideoRecorderProxy;



class PostProcessingModule : public AbstractModule
{
    Q_OBJECT

    Q_PROPERTY(precitec::gui::SimulationCmdProxy simulationCmdProxy READ simulationCmdProxy CONSTANT)

    Q_PROPERTY(precitec::gui::StorageUpdateProxy storageUpdateProxy READ storageUpdateProxy CONSTANT)

    Q_PROPERTY(precitec::gui::DbProxy dbProxy READ dbProxy CONSTANT)

    Q_PROPERTY(precitec::gui::WeldHeadMsgProxy weldHeadMsgProxy READ weldHeadMsgProxy CONSTANT)

    Q_PROPERTY(precitec::gui::TriggerCmdProxy triggerCmdProxy READ triggerCmdProxy CONSTANT)

    Q_PROPERTY(precitec::gui::ResultsProxy resultsProxy READ resultsProxy CONSTANT)

    Q_PROPERTY(precitec::gui::RecorderProxy recorderProxy READ recorderProxy CONSTANT)

    Q_PROPERTY(precitec::gui::SystemStatusProxy systemStatusProxy READ systemStatusProxy CONSTANT)

    Q_PROPERTY(precitec::gui::VideoRecorderProxy videoRecorderProxy READ videoRecorderProxy CONSTANT)

    
   

public:
    static PostProcessingModule* instance();

    SimulationCmdProxy simulationCmdProxy() const
    {
        return m_simulationCmdProxy;
    }

    StorageUpdateProxy storageUpdateProxy() const
    {
        return m_storageUpdateProxy;
    }

    DbProxy dbProxy() const
    {
        return m_dbProxy;
    }

    WeldHeadMsgProxy weldHeadMsgProxy() const
    {
        return m_weldHeadMsgProxy;
    }

    TriggerCmdProxy triggerCmdProxy() const
    {
        return m_triggerCmdProxy;
    }

    ResultsProxy resultsProxy() const
    {
        return m_resultsProxy;
    }

    RecorderProxy recorderProxy() const
    {
        return m_recorderProxy;
    }

    SystemStatusProxy systemStatusProxy() const
    {
        return m_systemStatusProxy;
    }

    VideoRecorderProxy videoRecorderProxy() const
    {
        return m_videoRecorderProxy;
    }



    Q_INVOKABLE void initialize();

private:
    explicit PostProcessingModule();

    void init() override;
    void registerPublications() override;

    SimulationCmdProxy m_simulationCmdProxy;
    std::shared_ptr<precitec::interface::TStorageUpdate<precitec::interface::EventProxy>> m_storageUpdateProxy;

    std::shared_ptr<TDb<MsgProxy>> m_dbProxy;
    std::shared_ptr<TTriggerCmd<EventProxy>> m_triggerCmdProxy;
    std::shared_ptr<TResults<EventProxy>> m_resultsProxy;
    std::shared_ptr<TRecorder<EventProxy>> m_recorderProxy;
    std::shared_ptr<TSystemStatus<EventProxy>> m_systemStatusProxy;
    std::shared_ptr<TCalibration<MsgProxy>> m_calibrationProxy;
    std::shared_ptr<TInspectionOut<EventProxy>> m_inspectionOutProxy;
    //std::shared_ptr<TProductTeachIn<EventProxy>> m_productTeachInProxy;
    std::shared_ptr<TWeldHeadMsg<MsgProxy>> m_weldHeadMsgProxy;
    std::shared_ptr<TVideoRecorder<EventProxy>> m_videoRecorderProxy;
};

}
}




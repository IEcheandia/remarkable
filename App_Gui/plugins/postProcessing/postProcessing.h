#pragma once

#ifndef PostProcessing_H
#define PostProcessing_H

#include <QObject>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QObject>
#include <QUrl>
#include <QUuid>
#include <QVariant>

#include "postProcessingTriggerServer.h"
#include "../../Mod_Storage/src/parameterSet.h"
#include <common/graph.h>
#include "../../Mod_Storage/src/seam.h"
#include "event/results.proxy.h"
#include "event/recorder.proxy.h"
#include "event/systemStatus.proxy.h"
#include "event/inspectionOut.proxy.h"
#include "event/videoRecorder.proxy.h"
#include "event/triggerCmd.proxy.h"
#include "event/dbNotification.proxy.h"
#include "message/calibration.proxy.h"
#include "message/db.proxy.h"
#include "message/weldHead.proxy.h"


namespace precitec
{

typedef std::shared_ptr<TVideoRecorder<EventProxy>> VideoRecorderProxy;

namespace gui
{
typedef std::shared_ptr<TDb<MsgProxy>>  DbProxy;
typedef std::shared_ptr<TTriggerCmd<EventProxy>> TriggerCmdProxy;
typedef std::shared_ptr<TResults<EventProxy>> ResultsProxy;
typedef std::shared_ptr<TRecorder<EventProxy>> RecorderProxy;
typedef std::shared_ptr<TSystemStatus<EventProxy>> SystemStatusProxy;
typedef std::shared_ptr<TCalibration<MsgProxy>> CalibrationProxy;
typedef std::shared_ptr<TInspectionOut<EventProxy>> InspectionOutProxy;
typedef std::shared_ptr<TWeldHeadMsg<MsgProxy>> WeldHeadMsgProxy;


namespace components
{
namespace postprocessing 
{
    


class PostProcessing : public QObject {
    Q_OBJECT
    Q_PROPERTY(precitec::storage::Seam* currentSeam READ currentSeam WRITE setCurrentSeam NOTIFY currentSeamChanged);
  
    Q_PROPERTY(precitec::gui::DbProxy dbProxy MEMBER m_dbProxy NOTIFY dbProxyChanged);
    Q_PROPERTY(precitec::gui::TriggerCmdProxy triggerCmdProxy MEMBER m_triggerCmdProxy);
    Q_PROPERTY(precitec::gui::ResultsProxy resultsProxy MEMBER m_resultsProxy);
    Q_PROPERTY(precitec::gui::RecorderProxy recorderProxy MEMBER m_recorderProxy);
    Q_PROPERTY(precitec::gui::SystemStatusProxy systemStatusProxy MEMBER m_systemStatusProxy);
    Q_PROPERTY(precitec::gui::CalibrationProxy calibrationProxy MEMBER m_calibrationProxy);
    Q_PROPERTY(precitec::gui::WeldHeadMsgProxy weldHeadMsgProxy MEMBER m_weldHeadMsgProxy);
    Q_PROPERTY(precitec::VideoRecorderProxy videoRecorderProxy MEMBER m_videoRecorderProxy);
    Q_PROPERTY(precitec::gui::InspectionOutProxy inspectionOutProxy MEMBER m_inspectionOutProxy);
  

    

public:
    
    PostProcessing(QObject *parent = nullptr);    
    
    
    // Add public methods, properties, and signals here
    
    Q_INVOKABLE void changeFilterParamAndTrigger(int productType, int seamseriesNumber, precitec::storage::Seam* seam, const QUuid &uuid, const QVariant &value);
    Q_INVOKABLE void init();
    
    Poco::UUID convertQUuidToPoco(const QUuid& quuid);    
    
    
    precitec::storage::Seam* currentSeam() const
    {
        return m_currentSeam;
    }
    
    void setCurrentSeam(precitec::storage::Seam *seam);

    DbProxy dbProxy() const
    {
        return m_dbProxy;
    }

signals:

    void dbProxyChanged(const TDb<MsgProxy> &dbProxy); 
    void triggerCmdProxyChanged(const TTriggerCmd<EventProxy> &triggerCmdProxy);
    void resultsProxyChanged(const TResults<EventProxy> &resultsProxy);
    void recorderProxyChanged(const TRecorder<EventProxy> &recorderProxy);
    void systemStatusProxyChanged(const TSystemStatus<EventProxy> &systemStatusProxy);
    void calibrationProxyChanged(const TCalibration<MsgProxy> &calibrationProxy);
    void weldHeadMsgProxyChanged(const TWeldHeadMsg<MsgProxy> &weldHeadMsgProxy);
    void videoRecorderProxyChanged(const TVideoRecorder<EventProxy> &videoRecorderProxy);
    void inspectionOutProxyChanged(const TInspectionOut<EventProxy> &inspectionOutProxy);
    void currentSeamChanged();

public slots:
    // Define slots if necessary
    
    
private:
    
   
    ParameterList convertFromDBParamSetToInterfaceParameterList(const std::vector<storage::Parameter*> &parameters);
 //   void changeFilterParamAndTrigger();
    
    precitec::storage::Seam *m_currentSeam = nullptr;
    QMetaObject::Connection m_destroyConnection;    

    std::shared_ptr<TDb<MsgProxy>> m_dbProxy;
    std::shared_ptr<TTriggerCmd<EventProxy>> m_triggerCmdProxy;
    std::shared_ptr<TResults<EventProxy>> m_resultsProxy;
    std::shared_ptr<TRecorder<EventProxy>> m_recorderProxy;
    std::shared_ptr<TSystemStatus<EventProxy>> m_systemStatusProxy;
    std::shared_ptr<TCalibration<MsgProxy>> m_calibrationProxy;
    std::shared_ptr<TInspectionOut<EventProxy>> m_inspectionOutProxy;
    std::shared_ptr<TWeldHeadMsg<MsgProxy>> m_weldHeadMsgProxy;
    std::shared_ptr<TVideoRecorder<EventProxy>> m_videoRecorderProxy;

    PostProcessingTriggerServer m_triggerServer;
};

#endif // PostProcessing_H


}
}
}
}


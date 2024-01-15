#pragma once

#include "event/videoRecorder.interface.h"

#include <QObject>

namespace precitec
{

typedef std::shared_ptr<precitec::interface::TVideoRecorder<precitec::interface::AbstractInterface>> VideoRecorderProxy;

namespace storage
{
class ProductInstanceModel;
}

namespace gui
{

/**
 * Controller for deleting Product Instances which are checked in the ProductInstanceModel.
 * The delete is delegated to the VideoRecorder.
 **/
class DeleteProductInstanceController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(precitec::storage::ProductInstanceModel *model READ productInstanceModel WRITE setProductInstanceModel NOTIFY productInstanceModelChanged)
    Q_PROPERTY(precitec::VideoRecorderProxy videoRecorderProxy READ videoRecorderProxy WRITE setVideoRecorderProxy NOTIFY videoRecorderProxyChanged)
public:
    DeleteProductInstanceController(QObject *parent = nullptr);
    ~DeleteProductInstanceController() override;

    storage::ProductInstanceModel *productInstanceModel() const
    {
        return m_productInstanceModel;
    }
    void setProductInstanceModel(storage::ProductInstanceModel *model);

    VideoRecorderProxy videoRecorderProxy() const
    {
        return m_videoRecorderProxy;
    }
    void setVideoRecorderProxy(const VideoRecorderProxy &proxy);

    Q_INVOKABLE void deleteAllChecked();

Q_SIGNALS:
    void productInstanceModelChanged();
    void videoRecorderProxyChanged();

private:
    storage::ProductInstanceModel *m_productInstanceModel = nullptr;
    QMetaObject::Connection m_productInstanceModelDestroyConnection;
    VideoRecorderProxy m_videoRecorderProxy;

};


}
}

#pragma once
#include "event/systemStatus.interface.h"
#include <QObject>
#include <QUuid>
#include <QMutex>


namespace precitec 
{
namespace gui
{

class SystemStatusServer : public QObject, public precitec::interface::TSystemStatus<precitec::interface::AbstractInterface>
{
    Q_OBJECT
    Q_PROPERTY(OperationState state     READ state          NOTIFY stateChanged)
    Q_PROPERTY(QString graphID          READ graphID        NOTIFY productInfoChanged)
    Q_PROPERTY(QString graphName        READ graphName      NOTIFY productInfoChanged)
    Q_PROPERTY(QUuid   measureTaskID    READ measureTaskID  NOTIFY productInfoChanged)
    Q_PROPERTY(QString measureTaskName  READ measureTaskName NOTIFY productInfoChanged)
    Q_PROPERTY(QString stationID        READ stationID      NOTIFY productInfoChanged)
    Q_PROPERTY(QString productID        READ productID      NOTIFY productInfoChanged)
    Q_PROPERTY(QString productName      READ productName    NOTIFY productInfoChanged)
    Q_PROPERTY(int     seam             READ seam           NOTIFY productInfoChanged)
    Q_PROPERTY(int     seamSeries       READ seamSeries     NOTIFY productInfoChanged)
    Q_PROPERTY(int     visualSeam       READ visualSeam     NOTIFY productInfoChanged)
    Q_PROPERTY(int     visualSeamSeries READ visualSeamSeries NOTIFY productInfoChanged)
    Q_PROPERTY(double  processingTime   READ processingTime NOTIFY productInfoChanged)   
    Q_PROPERTY(bool    productValid     READ isProductValid NOTIFY productInfoChanged)


public:
    /**
     * Enum with same values as precitec::interface::OperationState.
     * Just added to be able to export to QML.
     **/
    enum class OperationState
    {
        Normal,
        Live,
        Automatic,
        Calibration,
        NotReady,
        ProductTeachIn,
        EmergencyStop,
        /**
         * Special value for the case that no state has been retrieved yet.
         **/
        Unknown
    };
    Q_ENUM(OperationState)


    explicit SystemStatusServer(QObject *parent = nullptr);
    virtual ~SystemStatusServer() override;

    OperationState state() const
    {
        return m_state;
    }

    QString stationID() const
    {
        return QString::fromStdString( m_productInfo.m_oStationId.toString().c_str() );
    }

    QUuid measureTaskID() const
    {
        return QUuid{QByteArray::fromStdString( m_productInfo.m_oMeasureTaskId.toString() )};
    }

    QString graphID() const
    {
        return QString::fromStdString( m_productInfo.m_oGraphId.toString() );
    }

    QString graphName() const
    {
        return QString::fromStdString( m_productInfo.m_oGraphName );
    }

    QString productID() const
    {
        return QString::fromStdString( m_productInfo.m_oProductId.toString() );
    }

    QString productName() const
    {
        return QString::fromStdString( m_productInfo.m_oProductName );
    }

    QString measureTaskName() const
    {
        return QString::fromStdString( m_productInfo.m_oMeasureTaskName);
    }

    int seam() const
    {
        return m_seamNumber;
    }

    int seamSeries() const
    {
        return m_productInfo.m_oSeamseries;
    }
    int visualSeam() const
    {
        return seam() + 1;
    }

    int visualSeamSeries() const
    {
        return seamSeries() + 1;
    }

    double processingTime() const
    {
        return m_productInfo.m_oProcessingTime;
    }
    
    bool isProductValid() const
    {
        return !m_productInfo.m_oProductId.isNull();
    }

Q_SIGNALS:

    void stateChanged();
    void productInfoChanged();
    /**
     * Emitted when the system switches from Calibration to another state.
     **/
    void returnedFromCalibration();

    /**
     * Emitted whenever the system updated the Product with the given @p productId.
     **/
    void productLoaded(const QUuid &productId);
    /**
     * Filter parameter for @p measureTaskId got updated.
     **/
    void parameterUpdated(const QUuid &measureTaskId);

    void upsStateChanged();

    void enteredNormalState();
    void enteredNotReadyState();

public:

    void signalSystemError(precitec::interface::ErrorState errorState) override;
    void signalHardwareError(precitec::interface::Hardware hardware) override;
    void acknowledgeError(precitec::interface::ErrorState errorState) override;
    void signalState(precitec::interface::ReadyState state) override;
    void mark(precitec::interface::ErrorType errorType, int position) override;
    void operationState(precitec::interface::OperationState state) override;
    void upsState(precitec::interface::UpsState state) override;
    void workingState(precitec::interface::WorkingState state) override;
    void signalProductInfo(precitec::interface::ProductInfo productInfo) override;
    void productUpdated(Poco::UUID productId) override;
    void filterParameterUpdated(Poco::UUID measureTaskID, Poco::UUID instanceFilterId) override;

protected:

    OperationState m_state = OperationState::Unknown;
    precitec::interface::ProductInfo m_productInfo;

private:
    void handleUpsState();
    QMutex m_upsMutex;
    precitec::interface::UpsState m_ups = precitec::interface::Online;
    QUuid m_upsNotification;
    int m_seamNumber = 0;
};


} // namespace gui
} // namespace precitec

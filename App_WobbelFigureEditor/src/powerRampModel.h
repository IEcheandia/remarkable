#pragma once

#include <QObject>
#include <QAbstractTableModel>
#include <vector>

namespace precitec
{

namespace gui
{
namespace components
{
namespace plotter
{
class DataSet;
}
}
}

namespace scantracker
{

namespace components
{

namespace wobbleFigureEditor
{
class SelectionHandler;

class PowerRampModel : public QAbstractTableModel
{
public:
    struct Row
    {
        int id = 0;
        double power = 0;
        double ringPower = 0;
    };

    enum Roles
    {
        NameRole = Qt::UserRole,
        IdRole,
        PowerRole,
        RingPowerRole,
        ResetPowerRole,
        AddRole,
        ViewRole,
        DeleteRole,
    };

    enum Cols
    {
        NameCol = 0,
        IdCol,
        PowerCol,
        RingPowerCol,
        ResetCol,
        AddCol,
        ViewCol,
        DeleteCol,
        LastCol = DeleteCol,
    };

    Q_ENUM(Cols)

    struct PowerLimit
    {
        double low = 0, high = 0;

        bool check(double val) const
        {
            return val == -1 || (val >= low && val <= high);
        }

        void clamp(double& val) const
        {
            if (val == -1)
            {
                return;
            }

            if (val < low)
            {
                val = low;
            }

            if (val > high)
            {
                val = high;
            }
        }
    };

private:
    Q_OBJECT

    Q_PROPERTY(bool dualChannel MEMBER m_dualChannel NOTIFY dualChannelChanged)
    Q_PROPERTY(bool modifyLaserPower MEMBER m_modifyLaserPower NOTIFY modifyLaserPowerChanged)
    Q_PROPERTY(bool modifyRingPower MEMBER m_modifyRingPower NOTIFY modifyRingPowerChanged)
    Q_PROPERTY(bool asOffset MEMBER m_asOffset NOTIFY asOffsetChanged)
    Q_PROPERTY(bool modifyDefault MEMBER m_modifyDefault NOTIFY modifyDefaultChanged)

    Q_PROPERTY(bool periodAvailable READ periodAvailable NOTIFY periodAvailableChanged)
    Q_PROPERTY(bool periodic WRITE setPeriodic READ periodic NOTIFY periodicChanged)
    Q_PROPERTY(int periodStart WRITE setPeriodStart READ periodStart NOTIFY periodStartChanged)
    Q_PROPERTY(int periodEnd WRITE setPeriodEnd READ periodEnd NOTIFY periodEndChanged)

    Q_PROPERTY(int pointCount READ pointCount NOTIFY pointCountChanged)

    Q_PROPERTY(precitec::gui::components::plotter::DataSet* laserPowers READ laserPowers NOTIFY rampChanged)
    Q_PROPERTY(precitec::gui::components::plotter::DataSet* ringPowers READ ringPowers NOTIFY rampChanged)

    Q_PROPERTY(std::vector<bool> pointIdValidity READ pointIdValidity NOTIFY pointIdValidityChanged)
    Q_PROPERTY(std::vector<bool> powerValidity READ powerValidity NOTIFY powerValidityChanged)
    Q_PROPERTY(std::vector<bool> ringPowerValidity READ ringPowerValidity NOTIFY ringPowerValidityChanged)
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)
    Q_PROPERTY(bool periodValid READ periodValid NOTIFY periodValidChanged)

    QHash<int, QByteArray> m_roles;
    bool m_dualChannel = false;

    int m_laserPointCount = 0;
    std::vector<Row> m_rows;
    bool m_periodic = false;
    int m_periodStart = 0;
    int m_periodEnd = 0;
    bool m_modifyDefault = true;
    bool m_asOffset = false;
    bool m_modifyLaserPower = true;
    bool m_modifyRingPower;
    bool m_periodAvailable = false;

    PowerLimit m_lpLimit;
    PowerLimit m_rpLimit;
    std::vector<bool> m_idValidity;
    std::vector<bool> m_powerValidity;
    std::vector<bool> m_ringPowerValidity;
    bool m_periodValid = true;
    bool m_valid = true;

    precitec::gui::components::plotter::DataSet* m_laserPowers;
    precitec::gui::components::plotter::DataSet* m_ringPowers;

public:
    explicit PowerRampModel(QObject* parent = nullptr);
    ~PowerRampModel() override;

    PowerLimit laserPowerLimits() const { return m_lpLimit; }
    PowerLimit ringPowerLimits() const { return m_rpLimit; }

    bool modifyDefault() const { return m_modifyDefault; }
    bool asOffset() const { return m_asOffset; }
    bool modifyLaserPower() const { return m_modifyLaserPower; }
    bool modifyRingPower() const {return m_modifyRingPower;}

    std::vector<Row> const& getRows() const { return m_rows; }
    bool periodAvailable() const { return m_periodAvailable; }
    void setPeriodic(bool val);
    bool periodic() const { return m_periodic; }
    int periodStart() const { return m_periodStart; }
    int periodEnd() const { return m_periodEnd; }

    void setPeriodStart(int id);
    void setPeriodEnd(int id);

    int pointCount() const { return m_rows.size(); }

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    precitec::gui::components::plotter::DataSet* laserPowers();
    precitec::gui::components::plotter::DataSet* ringPowers();

    std::vector<int> pointIds() const;
    std::vector<bool> pointIdValidity() const {return m_idValidity;}
    std::vector<bool> powerValidity() const {return m_powerValidity;}
    std::vector<bool> ringPowerValidity() const {return m_ringPowerValidity;}

    void updateValidity();
    bool valid() const {return m_valid;}
    bool periodValid() const {return m_periodValid;}
    bool isValidId(int id) const { return id >= 0 && id < m_laserPointCount; }

    Q_INVOKABLE void deletePoint(int rowIdx);
    Q_INVOKABLE void addPoint(int rowIdx);
    Q_INVOKABLE void reset(int laserPointCount);
    Q_INVOKABLE void updateLaserPointCount(int laserPointCount);
    Q_INVOKABLE void setPowerLimits(double powerBottom, double powerTop, double ringBottom, double ringTop);

Q_SIGNALS:
    void periodicChanged();
    void periodStartChanged();
    void periodEndChanged();
    void rampChanged();
    void pointCountChanged();
    void pointIdValidityChanged();
    void powerValidityChanged();
    void ringPowerValidityChanged();
    void validChanged();
    void periodValidChanged();
    void modifyLaserPowerChanged();
    void modifyRingPowerChanged();
    void modifyDefaultChanged();
    void asOffsetChanged();
    void dualChannelChanged();
    void periodAvailableChanged();
};
}
}
}
}

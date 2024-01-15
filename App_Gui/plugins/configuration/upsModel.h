#pragma once
#include <QAbstractListModel>

#include <vector>

namespace precitec
{
namespace gui
{

class UpsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool modified READ isModified NOTIFY modifiedChanged)
    Q_PROPERTY(QModelIndex selectedIndex READ selectedIndex NOTIFY selectedIndexChanged)
public:
    UpsModel(QObject *parent = nullptr);
    ~UpsModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = {}) const override;

    void setNutConfigDir(const QString &config);
    QString nutConfigDir() const
    {
        return m_nutConfigDir;
    }

    bool isModified() const
    {
        return m_modified;
    }

    QModelIndex selectedIndex() const
    {
        return m_selectedIndex;
    }

    Q_INVOKABLE void select(int row);

    Q_INVOKABLE void save();

    /**
     * Selects the index by @p mode and @p driver.
     **/
    void selectByModeAndDriver(int mode, int driver);

Q_SIGNALS:
    void nutConfigDirChanged();
    void modifiedChanged();
    void selectedIndexChanged();

private:
    QModelIndex indexForConfiguration() const;
    void markAsModified();
    enum class Mode
    {
        None,
        Standalone
    };
    enum class Driver
    {
        None,
        BlazerUsb,
        UsbhidUps,
        NutdrvQx,
        BlazerSerOmron,
        BlazerUsbOmron
    };
    struct UpsData
    {
        Mode mode;
        Driver driver;
        QString description;
        QString image;
    };

    Mode initMode() const;
    Driver initDriver() const;

    std::vector<UpsData> m_upses;
    QString m_nutConfigDir = QStringLiteral("/etc/nut/");
    Mode m_mode;
    Driver m_driver;
    bool m_modified = false;
    QModelIndex m_selectedIndex;
};

}
}

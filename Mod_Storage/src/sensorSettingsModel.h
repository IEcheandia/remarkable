#pragma once

#include "resultSetting.h"

#include <QAbstractListModel>

class QIODevice;
class QColor;
class TestSensorSettingsModel;

namespace precitec
{
namespace storage
{

class SensorSettingsModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString configurationDirectory READ configurationDirectory WRITE setConfigurationDirectory NOTIFY configurationDirectoryChanged)

public:
    explicit SensorSettingsModel(QObject *parent = nullptr);
    ~SensorSettingsModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString configurationDirectory() const
    {
        return m_configurationDirectory;
    }
    void setConfigurationDirectory(const QString& dir);

    Q_INVOKABLE QModelIndex indexForResultType(int enumType) const;

    ResultSetting *checkAndAddItem(int enumType, const QString& name, const QColor& color);

    ResultSetting *getItem(int enumType) const;

    Q_INVOKABLE void updateValue(const QModelIndex& modelIndex, const QVariant &data, precitec::storage::ResultSetting::Type target);

    /**
     * Ensures that a ResultSettings for @p enumType exists.
     * This is the same as calling @link checkAndAddItem with the name parameter derived from @p enumType and color black.
     **/
    Q_INVOKABLE void ensureItemExists(int enumType);

    static QString sensorName(int sensorId);

    static QColor sensorColor(int sensorId);

Q_SIGNALS:
    void configurationDirectoryChanged();

private:
    void load();
    void createDefaultSettings();
    void save();
    void addMissingItems();
    ResultSetting *addItem(int enumType, const QString& name, const QColor& color);

    std::vector<ResultSetting*> m_sensorItems;
    QString m_configurationDirectory;

    friend TestSensorSettingsModel;
};

}
}
Q_DECLARE_METATYPE(precitec::storage::SensorSettingsModel*)

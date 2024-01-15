#pragma once
#include <QAbstractListModel>
#include <QUuid>

#include <vector>
#include <optional>

class QJsonDocument;

namespace precitec
{
namespace gui
{
namespace components
{
namespace scheduler
{

/**
 * The SchedulerModel contains all scheduled tasks and supports adding, modifying and deleting specific tasks.
 *
 * It provides the following roles:
 * @li parameters Qt::DisplayRole (QString)
 * @li triggerName Qt::UserRole (QString)
 * @li taskName Qt::UserRole + 1 (QString)
 * @li triggerSettings Qt::UserRole + 2 (QVariantMap)
 * @li taskSettings Qt::UserRole + 3 (QVariantMap)
 **/
class SchedulerModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * Path to the json file containing all scheduled tasks.
     **/
    Q_PROPERTY(QString configFile READ configFile WRITE setConfigFile NOTIFY configFileChanged)
public:
    SchedulerModel(QObject *parent = nullptr);
    ~SchedulerModel() override;

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString configFile() const
    {
        return m_configFile;
    }
    void setConfigFile(const QString &file);

    Q_INVOKABLE void edit(int row, const QVariantMap &triggerSettings, const QVariantMap &taskSettings);
    Q_INVOKABLE void add(const QString &trigger, const QVariantMap &triggerSettings, const QVariant &triggerEvent, const QString &task, const QVariantMap &taskSettings);
    Q_INVOKABLE void remove(int row);

Q_SIGNALS:
    void configFileChanged();

private:
    void loadFromFile();
    void loadFromJson(const QJsonDocument &json);
    void save();
    struct Data
    {
        QUuid uuid;
        QString triggerName;
        QString taskName;
        QVariantMap triggerSettings;
        QVariantMap taskSettings;
        std::optional<int> triggerEvent;
    };
    QString displayText(const Data &data) const;
    std::vector<Data> m_data;
    QString m_configFile;
};

}
}
}
}

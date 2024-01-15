#pragma once

#include <QAbstractListModel>

#include "laserControlPreset.h"

class QFileSystemWatcher;

namespace precitec
{

namespace storage
{

class AttributeModel;
class Attribute;
class Parameter;
class ParameterSet;

}

namespace gui
{

class AbstractLaserControlModel : public QAbstractListModel
{
    Q_OBJECT

    /**
     * The index of the selected preset
     **/
    Q_PROPERTY(int currentPreset READ currentPreset WRITE setCurrentPreset NOTIFY currentPresetChanged)

    Q_PROPERTY(precitec::storage::LaserControlPreset* currentPresetItem READ currentPresetItem NOTIFY currentPresetChanged)

    /**
     * Directorty where the laser control presets are stored
     **/
    Q_PROPERTY(QString laserControlPresetDir READ laserControlPresetDir WRITE setLaserControlPresetDir NOTIFY laserControlPresetDirChanged)

    /**
     * The AttribueModel providing the Attribute description required when creating a missing hardware Parameter.
     **/
    Q_PROPERTY(precitec::storage::AttributeModel *attributeModel READ attributeModel WRITE setAttributeModel NOTIFY attributeModelChanged)

    Q_PROPERTY(bool channel2Enabled READ channel2Enabled WRITE setChannel2Enabled NOTIFY channel2EnabledChanged)

public:
    explicit AbstractLaserControlModel(QObject *parent = nullptr);
    ~AbstractLaserControlModel() override;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int currentPreset() const
    {
        return m_currentPreset;
    }
    virtual void setCurrentPreset(int idx);
    precitec::storage::LaserControlPreset* currentPresetItem() const;

    QString laserControlPresetDir() const
    {
        return m_laserControlPresetDir;
    }
    void setLaserControlPresetDir(const QString &dir);

    precitec::storage::AttributeModel *attributeModel() const
    {
        return m_attributeModel;
    }
    virtual void setAttributeModel(precitec::storage::AttributeModel *model);

    bool channel2Enabled() const
    {
        return m_channel2Enabled;
    }
    void setChannel2Enabled(bool enabled);

Q_SIGNALS:
    void currentPresetChanged();
    void laserControlPresetDirChanged();
    void attributeModelChanged();
    void channel2EnabledChanged();

protected:
    void addPathToWatcher(const QString &path);
    void removePathFromWatcher(const QString &path);
    void clearJsonWatcher();
    void connectPreset(precitec::storage::LaserControlPreset* preset);
    void addPresetInstance(precitec::storage::LaserControlPreset* preset);
    void insertPresetInstance(uint i, precitec::storage::LaserControlPreset* preset);
    void removePresetInstance(uint index);
    void clearPresets();
    precitec::storage::Attribute *findPresetAttribute(precitec::storage::LaserControlPreset::Key key) const;
    precitec::storage::Parameter *findPresetParameter(precitec::storage::ParameterSet* ps, precitec::storage::LaserControlPreset::Key key) const;
    bool updateParameterSet(precitec::storage::ParameterSet* ps, QModelIndex row);
    bool hasPresetParameters(precitec::storage::ParameterSet* ps);
    bool compareParameterSet(precitec::storage::ParameterSet* ps, QModelIndex row);
    bool updatePresetValues(precitec::storage::ParameterSet* ps, precitec::storage::LaserControlPreset* preset);
    bool clearPresetValues(precitec::storage::ParameterSet* ps);

    const std::vector<precitec::storage::LaserControlPreset*>& presets() const
    {
        return m_presets;
    }

    virtual void load();

private:
    int m_currentPreset = -1;
    bool m_channel2Enabled = false;
    QString m_laserControlPresetDir = QString();
    std::vector<precitec::storage::LaserControlPreset*> m_presets;
    QFileSystemWatcher *m_jsonWatcher;
    precitec::storage::AttributeModel *m_attributeModel = nullptr;
    QMetaObject::Connection m_attributeModelDestroyed;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::AbstractLaserControlModel*)

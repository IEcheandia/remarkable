#pragma once

#include "abstractHardwareParameterModel.h"

class QFileSystemWatcher;
class ReferenceImageControllerTest;

namespace precitec
{
namespace storage
{

class Seam;
class Parameter;
class ParameterSet;
class AttributeModel;

}
namespace gui
{

class ReferenceImageController : public AbstractHardwareParameterModel
{
    Q_OBJECT

    Q_PROPERTY(QString imagePath READ imagePath NOTIFY imagePathChanged)

    Q_PROPERTY(QString imageFilePath READ imageFilePath NOTIFY imageFilePathChanged)

    Q_PROPERTY(QString referenceImageDir READ referenceImageDir WRITE setReferenceImageDir NOTIFY referenceImageDirChanged)

    Q_PROPERTY(precitec::storage::Seam* currentSeam READ currentSeam WRITE setCurrentSeam NOTIFY currentSeamChanged)

public:
    explicit ReferenceImageController(QObject* parent = nullptr);
    ~ReferenceImageController() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex{}) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString imagePath() const
    {
        return m_imagePath;
    }

    QString referenceImageDir() const
    {
        return m_referenceImageDir;
    }
    void setReferenceImageDir(const QString& dir);

    precitec::storage::Seam* currentSeam() const
    {
        return m_currentSeam;
    }
    void setCurrentSeam(precitec::storage::Seam* seam);

    void setAttributeModel(precitec::storage::AttributeModel* model) override;

    QString imageFilePath() const;

    Q_INVOKABLE void saveHardwareParameters();

Q_SIGNALS:
    void imagePathChanged();
    void referenceImageDirChanged();
    void currentSeamChanged();
    void imageFilePathChanged(const QString& path);
    void referenceHardwareParametersChanged();

private:
    void updateImagePath();
    void updateHardwareParameters();
    void updateModel();

    precitec::storage::ParameterSet* getParameterSet() const override;
    bool isValid() const override;
    precitec::storage::ParameterSet* getParameterSetDirect() const override;

    QString m_imagePath;
    QString m_referenceImageDir;

    precitec::storage::Seam* m_currentSeam = nullptr;
    QMetaObject::Connection m_seamDestroyed;

    precitec::storage::ParameterSet* m_referenceHardwareParameters = nullptr;

    std::vector<std::tuple<std::string, HardwareParameters::Key, precitec::storage::Parameter*, precitec::storage::Parameter*, HardwareParameters::UnitConversion>> m_parametersDiff;

    QFileSystemWatcher *m_fileWatcher = nullptr;

    friend ReferenceImageControllerTest;
};

}
}
Q_DECLARE_METATYPE(precitec::gui::ReferenceImageController*)

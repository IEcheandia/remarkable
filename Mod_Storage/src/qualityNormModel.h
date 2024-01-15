#pragma once

#include "qualityNorm.h"

#include <QAbstractListModel>

class QIODevice;
class TestQualityNormModel;

namespace precitec
{
namespace storage
{

/**
 * This model provides all QualityNorms.
 * QualityNorms are used to provide default values for the min, max and length of Interval Errors for specific results, based on the thickness values of the Seam
 */

class QualityNormModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString configurationDirectory READ configurationDirectory WRITE setConfigurationDirectory NOTIFY configurationDirectoryChanged)

public:
    explicit QualityNormModel(QObject *parent = nullptr);
    ~QualityNormModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString configurationDirectory() const
    {
        return m_configurationDirectory;
    }
    void setConfigurationDirectory(const QString& dir);

    Q_INVOKABLE QModelIndex indexForQualityNorm(const QUuid& qualityNorm) const;

    Q_INVOKABLE precitec::storage::QualityNorm* qualityNorm(const QUuid& qualityNorm);

    Q_INVOKABLE QUuid idAtIndex(int index);

Q_SIGNALS:
    void configurationDirectoryChanged();

private:
    void load();

    QString m_configurationDirectory;
    std::vector<QualityNorm*> m_qualityNorms;

    friend TestQualityNormModel;
};

}
}
Q_DECLARE_METATYPE(precitec::storage::QualityNormModel*)


#pragma once

#include <QObject>
#include <QUuid>
#include <QString>

namespace precitec
{

namespace storage
{
class Product;
}

namespace gui
{
/**
 * The SeamProduceInstanceCopyHandler copies image files and create a transformed metafile
 * in correspondent seam sub-folder of a product instance
 * from another one (could be other product instance or the same product instance).
 **/
class SeamProduceInstanceCopyHandler : public QObject
{
    Q_OBJECT
public:
    explicit SeamProduceInstanceCopyHandler(QObject *parent = nullptr);
    ~SeamProduceInstanceCopyHandler() override;

    struct SeamProduceInstanceInfo
    {
        QString productName;
        QUuid productUuid;
        quint32 productType = 0;
        QString productInstanceDirectory;
        QUuid productInstanceUuid;
        quint32 serialNumber = 0;
        quint32 seamSeriesNumber = 0;
        quint32 seamNumber = 0;
    };

    void setSource(const SeamProduceInstanceInfo &source);
    void setTarget(const SeamProduceInstanceInfo &target);

    void copy();

private:
    QString checkedSourceSeamProductInstanceDirectory();
    QString checkedTargetSeamProductInstanceDirectory();
    void copySeamProductInstance(const QString sourceSeamProductInstancePath,
                                 const QString targetSeamProductInstancePath);
    static bool removeDirectory(const QString &directoryPath);
    void createTargetMetaFileFromSource(const QString &sourcePath, const QString &targetPath);

    SeamProduceInstanceInfo m_source;
    SeamProduceInstanceInfo m_target;
    const QString m_metaFileName = "sequence_info.xml";
};

}
}

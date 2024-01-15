#pragma once

#include <QAbstractListModel>
#include <QFileInfo>

namespace precitec
{
namespace gui
{

/**
 * Base class for the shared functionality of @link{VideoImportProductModel} and @link{VideoImportProductInstanceModel}.
 * The model provides directory listing in a provided path.
 *
 * The model provides the following roles:
 * @li name (QString, name of directory)
 * @li fileInfo (QFileInfo of the directory)
 **/
class AbstractVideoImportModel : public QAbstractListModel
{
    Q_OBJECT
public:
    AbstractVideoImportModel(QObject *parent = nullptr);
    ~AbstractVideoImportModel() override;

    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent) const override;
    QHash<int, QByteArray> roleNames() const override;

protected:
    void init(const QFileInfo &path);

private:
    QFileInfoList m_subDirectories;
};

}
}

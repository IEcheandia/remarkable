#pragma once
#include "DirectoryLoader.h"

#include <QAbstractListModel>

namespace precitec
{

namespace storage
{
class AbstractGraphModel;
}

namespace gui
{

namespace components
{

namespace grapheditor
{

class DirectoryModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(precitec::gui::components::grapheditor::DirectoryLoader* loader READ directoryLoader WRITE setDirectoryLoader NOTIFY directoryLoaderChanged)

public:
    explicit DirectoryModel(QObject *parent = nullptr);
    ~DirectoryModel() override;

    DirectoryLoader* directoryLoader() const;
    void setDirectoryLoader(DirectoryLoader* newLoader);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE QString nameForDirectory(const QString &currentDirPath) const;
    Q_INVOKABLE QString checkGraphFromSystemFolder(const QString &currentDirName) const;
    Q_INVOKABLE bool fileNameExists(const QString &fileName, DirectoryType currentDirType) const;
    DirectoryType getDirectoryType(const QModelIndex &index);

    /**
     * @returns the QModelIndex for the user directory with the same type as the provided @p systemIndex for a system directory.
     **/
    QModelIndex indexForUser(const QModelIndex& systemIndex) const;

    /**
     * @returns the QModelIndex for the user directory with the specified @p type.
     **/
    QModelIndex indexForUser(DirectoryType type) const;

Q_SIGNALS:
    void directoryLoaderChanged();

private:
    void init();
    QModelIndex dirPathByName(const QString &dirName) const;

    std::vector<std::tuple<QString, QString, DirectoryType, DirectoryOwnership, storage::AbstractGraphModel*>> m_directories;
    DirectoryLoader* m_directoryLoader = nullptr;
    QMetaObject::Connection m_destroyedConnection;
};

}
}
}
}
Q_DECLARE_METATYPE(precitec::gui::components::grapheditor::DirectoryModel*)



#pragma once

#include <QObject>
#include <QDir>
#include <QFile>

namespace precitec
{

namespace gui
{
class WeldmasterPaths;
namespace components
{

namespace grapheditor
{

enum class DirectoryType
{
    Graph,
    SubGraph,
    Macro
};

enum class DirectoryOwnership
{
    System,
    User
};

struct DirectoryContainer
{
    QString fileName;
    QString filePath;
    QString fileDescription;
    std::vector<std::tuple<QString, QString, DirectoryType, DirectoryOwnership>> directories;
};

class DirectoryLoader : public QObject
{
    Q_OBJECT

public:
    explicit DirectoryLoader(QObject *parent = nullptr);
    ~DirectoryLoader() override;

    Q_INVOKABLE void loadAdditionalDirectories();
    void saveAdditionalDirectories();

    //DirectoryModel
    const std::vector<std::tuple<QString, QString, DirectoryType, DirectoryOwnership>>& getDirectories();

Q_SIGNALS:
    void directoriesChanged();
    void defaultPathsChanged();

private:
    bool checkConfigDirectory();
    bool checkDirectoryExists(const QString &path);
    void insertDefaultDirectories();

    QString m_baseDirectory{QString::fromUtf8(qgetenv("WM_BASE_DIR"))};
    QString m_configDirectory{m_baseDirectory + QStringLiteral("/config/")};
    QString m_fileName{"graphEditorDirectories.json"};

    DirectoryContainer m_directoryContainer;
};

}
}
}
}
Q_DECLARE_METATYPE(precitec::gui::components::grapheditor::DirectoryLoader*)
Q_DECLARE_METATYPE(precitec::gui::components::grapheditor::DirectoryContainer*)
Q_DECLARE_METATYPE(precitec::gui::components::grapheditor::DirectoryOwnership)

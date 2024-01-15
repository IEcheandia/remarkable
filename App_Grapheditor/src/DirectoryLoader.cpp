#include "DirectoryLoader.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "../App_Gui/plugins/general/weldmasterPaths.h"

using namespace precitec::gui::components::grapheditor;

DirectoryLoader::DirectoryLoader(QObject* parent) : QObject(parent)
{ }

DirectoryLoader::~DirectoryLoader() = default;

void DirectoryLoader::loadAdditionalDirectories()
{
    QByteArray data;

    if (!checkConfigDirectory())
    {
        return;
    }

    m_directoryContainer.directories.clear();
    insertDefaultDirectories();

    QFile fileInConfigDir(m_configDirectory + m_fileName);
    if (!fileInConfigDir.exists())
    {
        saveAdditionalDirectories();
    }

    fileInConfigDir.open(QIODevice::ReadWrite);             //ReadWrite to
    if (fileInConfigDir.isOpen())
    {
        data = fileInConfigDir.readAll();
    }
    fileInConfigDir.close();

    QJsonParseError errorPtr;
    QJsonDocument doc = QJsonDocument::fromJson(data, &errorPtr);
    bool fileIsEmpty = false;
    if (doc.isNull())
    {
        if (data.size() == 0)
        {
            fileIsEmpty = true;
        }
    }
    QJsonObject rootObj = doc.object();
    if (fileIsEmpty)
    {
        m_directoryContainer.fileName = m_fileName;
        m_directoryContainer.filePath = m_configDirectory;
        m_directoryContainer.fileDescription = QStringLiteral("This is a file where manually entered directories for the Grapheditor can be saved. All graphs in the directories can be loaded by selecting the correct directory.");
        return;
    }
    m_directoryContainer.fileName = rootObj.value("Name").toString();
    m_directoryContainer.filePath = rootObj.value("Path").toString();
    m_directoryContainer.fileDescription = rootObj.value("Description").toString();
    QJsonArray directoryArray = rootObj.value("Directories").toArray();

    for (const auto &value : directoryArray)
    {
        auto it = std::find_if(m_directoryContainer.directories.begin(), m_directoryContainer.directories.end(), [&value](const auto &currentDir){return std::get<0>(currentDir) == value.toObject().value("Name").toString();});
        if (it == m_directoryContainer.directories.end())
        {
            m_directoryContainer.directories.push_back({value.toObject().value("Name").toString(), value.toObject().value("Path").toString(), value.toObject().value("IsGraphDir").toBool() ? DirectoryType::Graph : DirectoryType::SubGraph, DirectoryOwnership::User});
        }
    }
    emit directoriesChanged();
}

void DirectoryLoader::saveAdditionalDirectories()
{
    if (!checkConfigDirectory())
    {
        return;
    }

    QJsonObject directoryContainer;
    directoryContainer.insert(QStringLiteral("Name"), m_directoryContainer.fileName);
    directoryContainer.insert(QStringLiteral("Path"), m_directoryContainer.filePath);
    directoryContainer.insert(QStringLiteral("Description"), m_directoryContainer.fileDescription);
    QJsonArray directories;

    for (const auto &directory : m_directoryContainer.directories)
    {
        QJsonObject directoryEntry;
        directoryEntry.insert(QStringLiteral("Name"), std::get<0>(directory));
        directoryEntry.insert(QStringLiteral("Path"), std::get<1>(directory));
        directoryEntry.insert(QStringLiteral("IsGraphDir"), std::get<2>(directory) == DirectoryType::Graph);
        directories.append(directoryEntry);
    }

    directoryContainer.insert(QStringLiteral("Directories"), directories);

    QFile fileInConfigDir(m_configDirectory + m_fileName);
    fileInConfigDir.open(QIODevice::WriteOnly);
    if (fileInConfigDir.isOpen())
    {
        fileInConfigDir.resize(0);
        QJsonDocument doc;
        doc.setObject(directoryContainer);
        fileInConfigDir.write(doc.toJson());
    }
    fileInConfigDir.close();
}

const std::vector<std::tuple<QString, QString, DirectoryType, DirectoryOwnership>>& DirectoryLoader::getDirectories()
{
    return m_directoryContainer.directories;
}

bool DirectoryLoader::checkConfigDirectory()
{
    QDir configDirectory(m_configDirectory);

    if (!configDirectory.exists())
    {
        return false;
    }

    return true;
}

bool DirectoryLoader::checkDirectoryExists(const QString &path)
{
    QDir dirDestination(path);
    if (!dirDestination.exists())
    {
        return false;
    }
    return true;
}

void DirectoryLoader::insertDefaultDirectories()
{
    m_directoryContainer.fileName = m_fileName;
    m_directoryContainer.filePath = m_configDirectory;
    m_directoryContainer.fileDescription = QStringLiteral("This is a file where manually entered directories for the Grapheditor can be saved. All graphs in the directories can be loaded by selecting the correct directory.");

    QString defaultName;
    QString defaultPath;

    defaultName = QStringLiteral("System graphs");
    defaultPath = WeldmasterPaths::instance()->systemGraphDir();
    if (checkDirectoryExists(defaultPath))
    {
        m_directoryContainer.directories.push_back({defaultName, defaultPath, DirectoryType::Graph, DirectoryOwnership::System});
    }
    defaultName = QStringLiteral("Custom graphs");
    defaultPath = WeldmasterPaths::instance()->graphDir();
    if (checkDirectoryExists(defaultPath))
    {
        m_directoryContainer.directories.push_back({defaultName, defaultPath, DirectoryType::Graph, DirectoryOwnership::User});
    }else
    {
        QDir createFolder{defaultPath};
        createFolder.mkpath(".");
        m_directoryContainer.directories.push_back({defaultName, defaultPath, DirectoryType::Graph, DirectoryOwnership::User});
    }
    defaultName = QStringLiteral("Subgraphs Input");
    defaultPath = WeldmasterPaths::instance()->subGraphInputDir();
    if (checkDirectoryExists(defaultPath))
    {
        m_directoryContainer.directories.push_back({defaultName, defaultPath, DirectoryType::SubGraph, DirectoryOwnership::System});
    }
    defaultName = QStringLiteral("Subgraphs ROI");
    defaultPath = WeldmasterPaths::instance()->subGraphROIDir();
    if (checkDirectoryExists(defaultPath))
    {
        m_directoryContainer.directories.push_back({defaultName, defaultPath, DirectoryType::SubGraph, DirectoryOwnership::System});
    }
    defaultName = QStringLiteral("Subgraphs Pre-Process");
    defaultPath = WeldmasterPaths::instance()->subGraphPreDir();
    if (checkDirectoryExists(defaultPath))
    {
        m_directoryContainer.directories.push_back({defaultName, defaultPath, DirectoryType::SubGraph, DirectoryOwnership::System});
    }
    defaultName = QStringLiteral("Subgraphs Process");
    defaultPath = WeldmasterPaths::instance()->subGraphProcessDir();
    if (checkDirectoryExists(defaultPath))
    {
        m_directoryContainer.directories.push_back({defaultName, defaultPath, DirectoryType::SubGraph, DirectoryOwnership::System});
    }
    defaultName = QStringLiteral("Subgraphs Post-Process");
    defaultPath = WeldmasterPaths::instance()->subGraphPostDir();
    if (checkDirectoryExists(defaultPath))
    {
        m_directoryContainer.directories.push_back({defaultName, defaultPath, DirectoryType::SubGraph, DirectoryOwnership::System});
    }
    defaultName = QStringLiteral("Subgraphs Output");
    defaultPath = WeldmasterPaths::instance()->subGraphOutputDir();
    if (checkDirectoryExists(defaultPath))
    {
        m_directoryContainer.directories.push_back({defaultName, defaultPath, DirectoryType::SubGraph, DirectoryOwnership::System});
    }
    defaultName = QStringLiteral("Subgraphs Special S1");
    defaultPath = WeldmasterPaths::instance()->subGraphS6KSpecialS1Dir();
    if (checkDirectoryExists(defaultPath))
    {
        m_directoryContainer.directories.push_back({defaultName, defaultPath, DirectoryType::SubGraph, DirectoryOwnership::System});
    }
    defaultName = QStringLiteral("Subgraphs Special S2");
    defaultPath = WeldmasterPaths::instance()->subGraphS6KSpecialS2Dir();
    if (checkDirectoryExists(defaultPath))
    {
        m_directoryContainer.directories.push_back({defaultName, defaultPath, DirectoryType::SubGraph, DirectoryOwnership::System});
    }
    defaultName = QStringLiteral("Custom Subgraphs");
    defaultPath = WeldmasterPaths::instance()->userSubGraphDir();
    if (checkDirectoryExists(defaultPath))
    {
        m_directoryContainer.directories.push_back({defaultName, defaultPath, DirectoryType::SubGraph, DirectoryOwnership::User});
    }
    else
    {
        QDir createFolder{defaultPath};
        createFolder.mkpath(".");
        m_directoryContainer.directories.push_back({defaultName, defaultPath, DirectoryType::SubGraph, DirectoryOwnership::User});
    }
    defaultName = QStringLiteral("System Macros");
    defaultPath = WeldmasterPaths::instance()->systemMacroDir();
    if (checkDirectoryExists(defaultPath))
    {
        m_directoryContainer.directories.push_back({defaultName, defaultPath, DirectoryType::Macro, DirectoryOwnership::System});
    }
    defaultName = QStringLiteral("Custom Macros");
    defaultPath = WeldmasterPaths::instance()->userMacroDir();
    if (checkDirectoryExists(defaultPath))
    {
        m_directoryContainer.directories.push_back({defaultName, defaultPath, DirectoryType::Macro, DirectoryOwnership::User});
    }else
    {
        QDir createFolder{defaultPath};
        createFolder.mkpath(".");
        m_directoryContainer.directories.push_back({defaultName, defaultPath, DirectoryType::Macro, DirectoryOwnership::User});
    }
}

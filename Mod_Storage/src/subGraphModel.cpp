#include "subGraphModel.h"
#include "../App_Storage/src/compatibility.h"

#include <QCryptographicHash>
#include <QFileSystemWatcher>
#include <QUrl>
#include <QDirIterator>

#include "module/logType.h"
#include "module/moduleLogger.h"
namespace precitec
{
namespace storage
{

SubGraphModel::SubGraphModel(QObject *parent)
    : AbstractGraphModel(parent)
    , m_watcher(new QFileSystemWatcher{this})
{
    connect(this, &SubGraphModel::modelReset, this,
        [this] ()
        {
            m_categories.clear();
            for (const auto &storage : graphStorage())
            {
                m_categories.push_back(QFileInfo{std::get<QString>(storage)}.absoluteDir().dirName());
                parseSubGraph(std::get<fliplib::GraphContainer>(storage));
            }
            createAvailableCategories();
        }
    );
    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this,
        [this]
        {
            loadSubGraphs(m_systemDir, m_userDir);
        }
    );
    connect(this, &SubGraphModel::dataChanged,
        [this]
        {
            m_cachedGraphs.clear();
        }
    );
}

SubGraphModel::~SubGraphModel() = default;

QVariant SubGraphModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant{};
    }
    const auto &graph = this->graph(index);
    switch (role)
    {
    case Qt::DisplayRole:
        return QString::fromStdString(graph.name);
    case Qt::UserRole:
        return compatibility::toQt(graph.id);
    case Qt::UserRole+1:
    {
        QStringList ret;
        for (auto &group : graph.filterGroups)
        {
            ret << QString::fromStdString(group.name);
        }
        return ret;
    }
    case Qt::UserRole + 2:
        return m_enabled.at(index.row());
    case Qt::UserRole + 3:
        return m_checked.at(index.row());
    case Qt::UserRole + 4:
        return m_categories.at(index.row());
    case Qt::UserRole + 5:
        return graphImage(index);
    case Qt::UserRole + 6:
        return QString::fromStdString(graph.comment);
    case Qt::UserRole + 7:
        return pdfFiles().contains(QString::fromStdString(graph.name));
    case Qt::UserRole + 8:
        return QString::fromStdString(graph.group);
    }

    return QVariant{};
}

QHash<int, QByteArray> SubGraphModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("name")},
        {Qt::UserRole, QByteArrayLiteral("uuid")},
        {Qt::UserRole+1, QByteArrayLiteral("groups")},
        {Qt::UserRole+2, QByteArrayLiteral("enabled")},
        {Qt::UserRole+3, QByteArrayLiteral("checked")},
        {Qt::UserRole+4, QByteArrayLiteral("category")},
        {Qt::UserRole+5, QByteArrayLiteral("image")},
        {Qt::UserRole+6, QByteArrayLiteral("comment")},
        {Qt::UserRole+7, QByteArrayLiteral("pdfAvailable")},
        {Qt::UserRole+8, QByteArrayLiteral("groupName")},
    };
}

void SubGraphModel::loadSubGraphs(const QString &systemGraphDirectory, const QString &userGraphDirectory)
{
    m_systemDir = systemGraphDirectory;
    m_userDir = userGraphDirectory;

    std::vector<std::pair<QFileInfo, StorageType>> subDirs;

    if (QFileInfo systemGraphDirectoryInfo{systemGraphDirectory}; systemGraphDirectoryInfo.exists() && systemGraphDirectoryInfo.isDir())
    {
        subDirs.emplace_back(std::make_pair(systemGraphDirectory, StorageType::System));
        const auto systemSubDirs = QDir{systemGraphDirectory}.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
        subDirs.reserve(systemSubDirs.size() + subDirs.size() + 1);
        std::transform(systemSubDirs.begin(), systemSubDirs.end(), std::back_inserter(subDirs), [] (const auto &dir) { return std::make_pair(dir, StorageType::System); });
    }

    if (!m_watcher->directories().empty())
    {
        m_watcher->removePaths(m_watcher->directories());
    }

    QFileInfo userGraphDirectoryInfo{userGraphDirectory};
    if (userGraphDirectoryInfo.exists() && userGraphDirectoryInfo.isDir())
    {
        subDirs.emplace_back(std::make_pair(userGraphDirectoryInfo, StorageType::User));
        m_watcher->addPaths(QStringList{{userGraphDirectory}});
    }

    std::vector<std::pair<QFileInfo, StorageType>> files;
    for (const auto & subDir : subDirs)
    {
        const auto &graphFiles = QDir{std::get<0>(subDir).absoluteFilePath()}.entryInfoList(QStringList{QStringLiteral("*.xml")}, QDir::Files | QDir::Readable, QDir::Name);
        files.reserve(files.size() + graphFiles.size());
        std::transform(graphFiles.begin(), graphFiles.end(), std::back_inserter(files), [subDir] (const auto &file) { return std::make_pair(file, std::get<1>(subDir)); });
    }

    if (!checkSymmetricDifference(files))
    {
        return;
    }
    loadGraphsFromFiles(files);
}

static const std::map<Poco::UUID, std::pair<SubGraphModel::BridgedDataType, SubGraphModel::BridgeType>> s_bridges{
    {Poco::UUID("3f59b49e-7fa3-4fa8-9317-e21947b26bd8"), {SubGraphModel::BridgedDataType::ImageFrame, SubGraphModel::BridgeType::Sink}},
    {Poco::UUID("8310c333-20a2-4a91-a9f9-e72199d0604b"), {SubGraphModel::BridgedDataType::ImageFrame, SubGraphModel::BridgeType::Source}},

    {Poco::UUID("f1a572c8-8e30-4792-83bc-7508d21a533d"), {SubGraphModel::BridgedDataType::SampleFrame, SubGraphModel::BridgeType::Sink}},
    {Poco::UUID("21950067-eb43-4a7b-9707-5080b2153733"), {SubGraphModel::BridgedDataType::SampleFrame, SubGraphModel::BridgeType::Source}},

    {Poco::UUID("b50e317b-9895-4d0e-a547-8d8ce9cdaf3f"), {SubGraphModel::BridgedDataType::Blob, SubGraphModel::BridgeType::Sink}},
    {Poco::UUID("59674f26-488c-4ab5-a160-8debf4928f40"), {SubGraphModel::BridgedDataType::Blob, SubGraphModel::BridgeType::Source}},

    {Poco::UUID("e108a053-874e-4942-b415-c2004a654282"), {SubGraphModel::BridgedDataType::Line, SubGraphModel::BridgeType::Sink}},
    {Poco::UUID("5685257d-9af0-4f3d-adf4-2d17d1141312"), {SubGraphModel::BridgedDataType::Line, SubGraphModel::BridgeType::Source}},

    {Poco::UUID("488967c8-02f2-4a18-b88d-7f2c5b9749b4"), {SubGraphModel::BridgedDataType::Double, SubGraphModel::BridgeType::Sink}},
    {Poco::UUID("7770b9a6-ca04-46a7-b27c-33df27344ad4"), {SubGraphModel::BridgedDataType::Double, SubGraphModel::BridgeType::Source}},

    {Poco::UUID("316ead65-82b3-4dff-bed9-09a694c4eebb"), {SubGraphModel::BridgedDataType::PointList, SubGraphModel::BridgeType::Sink}},
    {Poco::UUID("55971812-0b2e-4202-bfbd-e7821a9044f6"), {SubGraphModel::BridgedDataType::PointList, SubGraphModel::BridgeType::Source}},

    {Poco::UUID("17b929b2-30ec-4c67-b0e3-4f319d05e50b"), {SubGraphModel::BridgedDataType::SeamFinding, SubGraphModel::BridgeType::Sink}},
    {Poco::UUID("18ece4c7-b1f5-4cf5-8b9d-4de7971a1d6d"), {SubGraphModel::BridgedDataType::SeamFinding, SubGraphModel::BridgeType::Source}},

    {Poco::UUID("523ce7df-d6a8-44da-8263-a0d500307e3c"), {SubGraphModel::BridgedDataType::PoorPenetrationCandidate, SubGraphModel::BridgeType::Sink}},
    {Poco::UUID("c260999e-c556-4c58-917d-f6dde7b79fc7"), {SubGraphModel::BridgedDataType::PoorPenetrationCandidate, SubGraphModel::BridgeType::Source}},

    {Poco::UUID("9d9a9a12-e51f-4921-a802-b0c90f516310"), {SubGraphModel::BridgedDataType::StartEndInfo, SubGraphModel::BridgeType::Sink}},
    {Poco::UUID("7c4c7bfa-39f1-498b-89bb-f3802b9f2f2a"), {SubGraphModel::BridgedDataType::StartEndInfo, SubGraphModel::BridgeType::Source}},

    {Poco::UUID("c11907fa-2f1e-4d43-b4ba-0a88fed95d17"), {SubGraphModel::BridgedDataType::SurfaceInfo, SubGraphModel::BridgeType::Sink}},
    {Poco::UUID("a8f73f37-b83d-4d6c-9cfc-4e38a53e8b7b"), {SubGraphModel::BridgedDataType::SurfaceInfo, SubGraphModel::BridgeType::Source}},

    {Poco::UUID("eff5a773-c29c-42c7-b804-dcafe5414dee"), {SubGraphModel::BridgedDataType::HoughPPCandidate, SubGraphModel::BridgeType::Sink}},
    {Poco::UUID("622b871c-c1ce-466c-89cc-376927571926"), {SubGraphModel::BridgedDataType::HoughPPCandidate, SubGraphModel::BridgeType::Source}}
};

void SubGraphModel::parseSubGraph(const fliplib::GraphContainer &graph)
{

    std::vector<Bridge> sourceBridges;
    std::vector<Bridge> sinkBridges;

    for (const auto &instanceFilter : graph.instanceFilters)
    {
        auto bridgeIt = s_bridges.find(instanceFilter.filterId);
        if (bridgeIt == s_bridges.end())
        {
            continue;
        }
        if (bridgeIt->second.second == BridgeType::Source)
        {
            sourceBridges.emplace_back(Bridge{bridgeIt->second.first, bridgeIt->second.second, instanceFilter.name, instanceFilter.id});
        } else
        {
            sinkBridges.emplace_back(Bridge{bridgeIt->second.first, bridgeIt->second.second, instanceFilter.name, instanceFilter.id});
        }
    }

    m_checked.push_back(false);
    m_enabled.push_back(sourceBridges.empty());
    m_sourceBridges.push_back(std::move(sourceBridges));
    m_sinkBridges.push_back(std::move(sinkBridges));
}

const fliplib::GraphContainer& SubGraphModel::getOrCreateCombinedGraph(const std::vector<QUuid>& subGraphIds, const Poco::UUID& graphId)
{
    auto it = std::find_if(m_cachedGraphs.begin(), m_cachedGraphs.end(), [&graphId] (const auto &graph) { return graph.id == graphId; });
    if (it == m_cachedGraphs.end())
    {
        generateGraph(subGraphIds, graphId);
        return m_cachedGraphs.back();
    }
    return *it;
}

fliplib::GraphContainer SubGraphModel::combinedGraph(const QUuid &id)
{
    auto it = std::find_if(m_generatedGraphIds.begin(), m_generatedGraphIds.end(), [id] (const auto &element) { return element.second.first == id; });
    if (it == m_generatedGraphIds.end())
    {
        return {};
    }
    return getOrCreateCombinedGraph(it->second.second, compatibility::toPoco(it->second.first));
}

void SubGraphModel::generateGraph(const std::vector<QUuid> &subGraphIds, const Poco::UUID &graphId)
{
    fliplib::GraphContainer graph{};
    graph.id = graphId;
    graph.name = std::string("Generated Graph ") + graphId.toString();
    graph.comment = "Combined Graph :";
    graph.parameterSet = compatibility::toPoco(QUuid::createUuid());

    std::vector<std::pair<Bridge, bool>> sourceBridges; //<sourceBridge, connected>
    std::vector<Bridge> sinkBridges;

    // copy the graph together
    int positionYOffset = 0;
    for (const auto &subGraphId : subGraphIds)
    {
        int maxPositionY = 0;

        const auto graphIndex = indexFor(subGraphId);
        if (!graphIndex.isValid())
        {
            wmLog(eWarning, "Requested subgraph %s does not exist.\n", subGraphId.toByteArray().data());
            continue;
        }
        const auto &subGraphSourceBridges = m_sourceBridges.at(graphIndex.row());
        sourceBridges.reserve(sourceBridges.size() + subGraphSourceBridges.size());
        for (auto & subGraphSourceBridge : subGraphSourceBridges)
        {
            sourceBridges.emplace_back(subGraphSourceBridge, false);
        }

        const auto &subGraphSinkBridges = m_sinkBridges.at(graphIndex.row());
        sinkBridges.insert(sinkBridges.end(), subGraphSinkBridges.begin(), subGraphSinkBridges.end());

        const auto &subGraph = std::get<0>(graphStorage().at(graphIndex.row()));
        graph.comment += (subGraph.name + "\n");

        int idCounter = graph.filterGroups.size();
        std::map<int, int> idMapper;
        for (const auto &group : subGraph.filterGroups)
        {
            idMapper.emplace(std::make_pair(group.number, idCounter++));
        }
        auto getGroupNumber = [&idMapper] (int group)
        {
            auto it = idMapper.find(group);
            if (it == idMapper.end())
            {
                return -1;
            }
            return it->second;
        };

        for (const auto &filter : subGraph.instanceFilters)
        {
            graph.instanceFilters.push_back(filter);
            graph.instanceFilters.back().group = getGroupNumber(filter.group);
            auto & rCurrentPosition = graph.instanceFilters.back().position;
            maxPositionY = std::max(rCurrentPosition.y + rCurrentPosition.height, maxPositionY);
            rCurrentPosition.y += positionYOffset;
        }

        for (const auto &group : subGraph.filterGroups)
        {
            graph.filterGroups.emplace_back(fliplib::FilterGroup{getGroupNumber(group.number), group.parent == -1 ? -1 : getGroupNumber(group.parent), group.name, subGraph.id});
        }

        graph.pipes.insert(graph.pipes.end(), subGraph.pipes.begin(), subGraph.pipes.end());

        for (const auto &filterDescription : subGraph.filterDescriptions)
        {
            //the compact grapheditor seems to require a description for each filter, even if duplicated
            graph.filterDescriptions.push_back(filterDescription);
        }

        graph.sensors.insert(graph.sensors.end(), subGraph.sensors.begin(), subGraph.sensors.end());
        graph.errors.insert(graph.errors.end(), subGraph.errors.begin(), subGraph.errors.end());
        graph.results.insert(graph.results.end(), subGraph.results.begin(), subGraph.results.end());
        for (const auto &port : subGraph.ports)
        {
            graph.ports.push_back(port);
            graph.ports.back().group = getGroupNumber(port.group);
            auto & rCurrentPosition = graph.ports.back().position;
            maxPositionY = std::max(rCurrentPosition.y + rCurrentPosition.height, maxPositionY);
            rCurrentPosition.y += positionYOffset;
        }

        positionYOffset += (maxPositionY + 50);
    }

    // generate pipes between bridges
    for (auto sinkBridgeIt = sinkBridges.begin(); sinkBridgeIt != sinkBridges.end(); sinkBridgeIt++)
    {
        // find the pipe for this bridge
        auto outPipeIt = std::find_if(graph.pipes.begin(), graph.pipes.end(), [sinkBridgeIt] (const auto &pipe) { return pipe.receiver == sinkBridgeIt->instanceFilter; });
        if (outPipeIt == graph.pipes.end())
        {
            continue;
        }
        // find other side of bridge
        for (auto sourceBridgeIt = sourceBridges.begin(); sourceBridgeIt != sourceBridges.end(); sourceBridgeIt++)
        {
            auto & curSourceBridge = sourceBridgeIt->first;
            auto & curSourceBridgeConnected = sourceBridgeIt->second;
            if (curSourceBridge.dataType != sinkBridgeIt->dataType)
            {
                continue;
            }
            if (sinkBridgeIt->name != curSourceBridge.name)
            {
                continue;
            }
            if (curSourceBridgeConnected)
            {
                wmLog(eWarning, "The sink filter %s is present multiple times \n", curSourceBridge.name.c_str());
            }
            // find all pipes from in-bridge to other filter
            for (auto inPipeIt = graph.pipes.begin(); inPipeIt != graph.pipes.end(); inPipeIt++)
            {
                if (inPipeIt->sender != curSourceBridge.instanceFilter)
                {
                    continue;
                }
                // replace the sender to connect to the actual pipe
                inPipeIt->sender = outPipeIt->sender;
                inPipeIt->senderConnectorName = outPipeIt->senderConnectorName;
                inPipeIt->senderConnectorId = outPipeIt->senderConnectorId;
                curSourceBridgeConnected = true;
            }
        }
    }

    //check if all the source filters have been connected to the corresponding sink filter
    for (auto sourceBridgeIt = sourceBridges.begin(); sourceBridgeIt != sourceBridges.end(); sourceBridgeIt++)
    {
        if (!sourceBridgeIt->second)
        {
            wmLog(eWarning, "The requested sink filter %s is not present \n", sourceBridgeIt->first.name.c_str());
        }
    }

    // remove bridge filters from filter descriptions
    for (auto it = graph.filterDescriptions.begin(); it != graph.filterDescriptions.end(); /* no increment */)
    {
        if (s_bridges.find((*it).id) != s_bridges.end())
        {
            it = graph.filterDescriptions.erase(it);
        } else
        {
            it++;
        }
    }

    // remove bridge instance filters
    for (auto it = graph.instanceFilters.begin(); it != graph.instanceFilters.end(); /* no increment */)
    {
        if (std::any_of(sourceBridges.begin(), sourceBridges.end(), [it] (const auto &bridge) { return bridge.first.instanceFilter == it->id; }) ||
            std::any_of(sinkBridges.begin(), sinkBridges.end(), [it] (const auto &bridge) { return bridge.instanceFilter == it->id; }))
        {
            // remove all pipes with this bridge filter
            for (auto pipeIt = graph.pipes.begin(); pipeIt != graph.pipes.end(); /* no increment */)
            {
                if (pipeIt->sender == it->id || pipeIt->receiver == it->id)
                {
                    pipeIt = graph.pipes.erase(pipeIt);
                } else
                {
                    pipeIt++;
                }
            }
            it = graph.instanceFilters.erase(it);
        } else
        {
            it++;
        }
    }

    m_cachedGraphs.push_back(std::move(graph));
}

static std::vector<SubGraphModel::Bridge> s_emptyVector{};

const std::vector<SubGraphModel::Bridge> &SubGraphModel::sourceBridges(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return s_emptyVector;
    }
    return m_sourceBridges.at(index.row());
}

const std::vector<SubGraphModel::Bridge> &SubGraphModel::sinkBridges(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return s_emptyVector;
    }
    return m_sinkBridges.at(index.row());
}

QString SubGraphModel::category(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return {};
    }
    return m_categories.at(index.row());
}

void SubGraphModel::check(const std::vector<QUuid> &subGraphs)
{
    // first set checked to false for all graphs
    for (auto it = m_checked.begin(); it != m_checked.end(); it++)
    {
        *it = false;
    }

    // check the selected graphs
    for (const auto &uuid : subGraphs)
    {
        const auto &index = indexFor(uuid);
        if (!index.isValid())
        {
            continue;
        }
        m_checked[index.row()] = true;
    }

    updateEnabled();

    // verify that we don't have any checked graphs which are not enabled
    if (!validateChecked())
    {
        std::vector<QUuid> uuids;
        for (std::size_t i = 0; i < m_checked.size(); i++)
        {
            if (!m_checked.at(i))
            {
                continue;
            }
            uuids.push_back(index(i, 0).data(Qt::UserRole).toUuid());
        }
        check(uuids);
        return;
    }
    emit dataChanged(index(0, 0), index(rowCount() - 1, 0), {Qt::UserRole + 2, Qt::UserRole + 3});
}

void SubGraphModel::check(const QModelIndex &index, bool set)
{
    if (!index.isValid())
    {
        return;
    }
    if (!m_enabled.at(index.row()))
    {
        return;
    }
    if (m_checked.at(index.row()) == set)
    {
        return;
    }
    m_checked[index.row()] = set;

    do
    {
        updateEnabled();
    } while (!validateChecked());

    emit dataChanged(this->index(0, 0), this->index(rowCount() - 1, 0), {Qt::UserRole + 2, Qt::UserRole + 3});
}

bool SubGraphModel::isGraphEnabled(const QModelIndex &index)
{
    if (!index.isValid())
    {
        return false;
    }
    return m_enabled.at(index.row());
}

void SubGraphModel::updateEnabled()
{
    for (std::size_t i = 0; i < m_enabled.size(); i++)
    {
        bool match = true;
        for (const auto &sourceBridge : m_sourceBridges.at(i))
        {
            if (!matchingBridge(sourceBridge))
            {
                match = false;
                break;
            }
        }
        if (!match)
        {
            m_enabled[i] = match;
            continue;
        }
        // verify sinks
        for (const auto &sinkBridge : m_sinkBridges.at(i))
        {
            if (identicalSink(sinkBridge))
            {
                match = false;
                break;
            }
        }
        m_enabled[i] = match;
    }
}

bool SubGraphModel::validateChecked()
{
    bool valid = true;
    for (std::size_t i = 0; i < m_checked.size(); i++)
    {
        if (!m_checked.at(i))
        {
            continue;
        }
        if (m_checked.at(i) && !m_enabled.at(i))
        {
            valid = false;
            m_checked[i] = false;
        }
    }
    return valid;
}

bool SubGraphModel::matchingBridge(const Bridge &sourceBridge) const
{
    for (std::size_t i = 0; i < m_checked.size(); i++)
    {
        if (!m_checked.at(i))
        {
            continue;
        }
        const auto &sinkBridges = m_sinkBridges.at(i);
        if (std::any_of(sinkBridges.begin(), sinkBridges.end(), [&sourceBridge] (const auto &sinkBridge) { return sourceBridge.dataType == sinkBridge.dataType && sourceBridge.name == sinkBridge.name;}))
        {
            return true;
        }
    }
    return false;
}

bool SubGraphModel::identicalSink(const Bridge &sinkBridge) const
{
    for (std::size_t i = 0; i < m_checked.size(); i++)
    {
        if (!m_checked.at(i))
        {
            continue;
        }
        const auto &sinkBridges = m_sinkBridges.at(i);
        if (std::any_of(sinkBridges.begin(), sinkBridges.end(), [&sinkBridge] (const auto &candidate) { return sinkBridge.dataType == candidate.dataType && sinkBridge.name == candidate.name && sinkBridge.instanceFilter != candidate.instanceFilter;}))
        {
            return true;
        }
    }
    return false;
}

QUuid SubGraphModel::generateGraphId(const std::vector<QUuid> &subGraphs)
{
    QCryptographicHash md5{QCryptographicHash::Md5};
    for (const auto &uuid : subGraphs)
    {
        md5.addData(uuid.toByteArray());
    }
    const auto hash = md5.result();
    auto it = m_generatedGraphIds.find(hash);
    if (it != m_generatedGraphIds.end())
    {
        return it->second.first;
    }
    it = m_generatedGraphIds.emplace(hash, std::make_pair(QUuid::createUuid(), subGraphs)).first;
    return it->second.first;
}

std::vector<QUuid> SubGraphModel::checkedGraphs() const
{
    std::vector<QUuid> ret;
    for (std::size_t i = 0; i < m_checked.size(); i++)
    {
        if (!m_checked.at(i))
        {
            continue;
        }
        ret.push_back(compatibility::toQt(std::get<0>(graphStorage().at(i)).id));
    }
    return ret;
}

static const std::vector<std::pair<QString, std::string>> s_categoryTranslations {
    {QStringLiteral("1 Input"), QT_TR_NOOP("Input")},
    {QStringLiteral("2 ROI & preSearch"), QT_TR_NOOP("ROI & Pre-search")},
    {QStringLiteral("3 Pre-Process"), QT_TR_NOOP("Pre-processing")},
    {QStringLiteral("4 Process"), QT_TR_NOOP("Processing")},
    {QStringLiteral("5 Post-Process"), QT_TR_NOOP("Post-processing")},
    {QStringLiteral("6 Output"), QT_TR_NOOP("Output")}
};

const QStringList &SubGraphModel::availableCategories() const
{
    return m_availableCategories;
}

void SubGraphModel::createAvailableCategories()
{
    m_availableCategories.clear();

    QStringList &categories = m_availableCategories;
    for (auto category : m_categories)
    {
        if (categories.contains(category))
        {
            continue;
        }
        categories << category;
    }

    // sort based on s_categoryTranslation index
    std::sort(categories.begin(), categories.end(),
        [] (const QString &a, const QString &b)
        {
            auto itA = std::find_if(s_categoryTranslations.begin(), s_categoryTranslations.end(), [a] (const auto pair) { return a.compare(pair.first, Qt::CaseInsensitive) == 0;});
            auto itB = std::find_if(s_categoryTranslations.begin(), s_categoryTranslations.end(), [b] (const auto pair) { return b.compare(pair.first, Qt::CaseInsensitive) == 0;});
            if (itA == itB)
            {
                return a < b;
            }
            return std::distance(s_categoryTranslations.begin(), itA) < std::distance(s_categoryTranslations.begin(), itB);
        }
    );
}

QString SubGraphModel::categoryToName(const QString &category) const
{
    auto it = std::find_if(s_categoryTranslations.begin(), s_categoryTranslations.end(), [category] (const auto pair) { return category.compare(pair.first, Qt::CaseInsensitive) == 0;});
    if (it != s_categoryTranslations.end())
    {
        return tr(it->second.c_str());
    }
    return category;
}

bool SubGraphModel::isSinkBridgeUsed(const Bridge &sinkBridge) const
{
    for (std::size_t i = 0; i < m_checked.size(); i++)
    {
        if (!m_checked.at(i))
        {
            continue;
        }
        const auto &sourceBridge = m_sourceBridges.at(i);
        if (std::any_of(sourceBridge.begin(), sourceBridge.end(), [&sinkBridge] (const auto &candidate) { return sinkBridge.dataType == candidate.dataType && sinkBridge.name == candidate.name && sinkBridge.instanceFilter != candidate.instanceFilter;}))
        {
            return true;
        }
    }
    return false;
}

void SubGraphModel::switchToAlternative(const QModelIndex &source, const QModelIndex &target)
{
    if (!source.isValid() || !target.isValid())
    {
        return;
    }
    if (!m_checked.at(source.row()) || m_checked.at(target.row()))
    {
        return;
    }
    m_checked[source.row()] = false;
    m_checked[target.row()] = true;

    do
    {
        updateEnabled();
    } while (!validateChecked());

    emit dataChanged(this->index(0, 0), this->index(rowCount() - 1, 0), {Qt::UserRole + 2, Qt::UserRole + 3});
}

}
}

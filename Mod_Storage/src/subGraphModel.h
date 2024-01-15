#pragma once

#include "abstractGraphModel.h"

namespace precitec
{
namespace storage
{

/**
 * The SubGraphModel provides all informations about available sub graphs.
 * This is very similar to the GraphModel, the main difference is that a sub graph
 * contains bridges. A bridge is specific to a data type and has an out and an in component.
 * Multiple sub graphs can be combined to a graph by connecting the bridges.
 *
 * The SubGraphModel does not support runtime changes to the graphs. If a new sub graph
 * is added/deleted/modified in the graph storage the SubGraphModel needs to be recreated.
 **/
class SubGraphModel : public AbstractGraphModel
{
    Q_OBJECT
    /**
     * The available categories
     **/
    Q_PROPERTY(QStringList availableCategories READ availableCategories CONSTANT)
public:
    explicit SubGraphModel(QObject *parent = nullptr);
    ~SubGraphModel();

    /**
     * The data type the bridge is for.
     * This matches the connector types of ports.
     **/
    enum class BridgedDataType
    {
        ImageFrame,
        SampleFrame,
        Blob,
        Line,
        Double,
        PointList,
        SeamFinding,
        PoorPenetrationCandidate,
        StartEndInfo,
        SurfaceInfo,
        HoughPPCandidate
    };

    /**
     * The type of the bridge. Whether it's a Source (providing a value) or a Sink bridge (taking a value).
     **/
    enum class BridgeType
    {
        Source,
        Sink
    };

    /**
     * Description of one Bridge
     **/
    struct Bridge
    {
        BridgedDataType dataType;
        BridgeType type;
        std::string name;
        Poco::UUID instanceFilter;
    };


    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    /**
     * Loads all graphs provided in @p systemGraphDirectory asynchronously.
     **/
    Q_INVOKABLE void loadSubGraphs(const QString &systemGraphDirectory, const QString &userGraphDirectory = {});

    /**
     * @returns a Graph combined from @p subGraphIds with @p graphId as the graph's id.
     * If the graph does not yet exist it is generated and added to the cache
     **/
    const fliplib::GraphContainer& getOrCreateCombinedGraph(const std::vector<QUuid>& subGraphIds, const Poco::UUID& graphId);

    /**
     * @returns a Graph combined from sub graphs from which @p id was generated.
     * If there is no @p id in the generated id cache an empty graph is returned.
     **/
    fliplib::GraphContainer combinedGraph(const QUuid &id);

    /**
     * @returns the source bridges of the sub graph at @p index.
     **/
    const std::vector<Bridge> &sourceBridges(const QModelIndex &index) const;

    /**
     * @returns the sink bridges of the sub graph at @p index.
     **/
    const std::vector<Bridge> &sinkBridges(const QModelIndex &index) const;

    /**
     * The name of the category of the graph at @p index.
     **/
    QString category(const QModelIndex &index) const;

    /**
     * Sets check to @c true for all the graph uuids in @p subGraphs. All other subgraphs are set to @c false.
     * Updating the checked state also updates the enabled state.
     * If @p subGraphs contains a uuid of a graph which cannot be checked due to missing source bridges it won't be set to @c true.
     **/
    void check(const std::vector<QUuid> &subGraphs);

    /**
     * Sets check to @p set for the graph identified by @p index. If the graph at @p index is not enabled, the state is not updated.
     * Checking a graph can influence the checked and enabled state of other graphs. All are adjusted accordingly.
     **/
    Q_INVOKABLE void check(const QModelIndex &index, bool set);

    Q_INVOKABLE bool isGraphEnabled(const QModelIndex &index);

    /**
     * @returns a unique id for the graph consisting of @p subGraphs.
     **/
    QUuid generateGraphId(const std::vector<QUuid> &subGraphs);

    /**
     * @returns the currently checked graphs.
     **/
    std::vector<QUuid> checkedGraphs() const;

    const QStringList &availableCategories() const;

    /**
     * @returns a translated name for the @p category. If the @p category is not known, the passed in value is returned.
     **/
    Q_INVOKABLE QString categoryToName(const QString &category) const;

    /**
     * @returns @c true if there is a checked graph providing the bridge for @p sourceBridge.
     **/
    bool matchingBridge(const Bridge &sourceBridge) const;

    /**
     * @returns @c true if the @p sinkBridge is used by a source bridge of another filter
     **/
    bool isSinkBridgeUsed(const Bridge &sinkBridge) const;

    /**
     * Toggles the subgraphs at @p source and @p target so that @p source is unckecked and
     * @p target is checked. If @p source was unchecked or @p target was checked nothing is done.
     **/
    Q_INVOKABLE void switchToAlternative(const QModelIndex &source, const QModelIndex &target);

private:
    /**
     * Generates a Graph from the @p subGraphIds and adds it to the cache
     **/
    void generateGraph(const std::vector<QUuid> &subGraphIds, const Poco::UUID &graphId);
    /**
     * Extracts the bridges from the @p graph.
     * This method needs to be called from rowInserted signal.
     **/
    void parseSubGraph(const fliplib::GraphContainer &graph);

    /**
     * @returns @c true if there is a checked graph providing the same sink bridge as @p sink.
     **/
    bool identicalSink(const Bridge &sinkBridge) const;

    /**
     * Updates the enabled state based on the checked state of all graphs.
     **/
    void updateEnabled();

    /**
     * Validates the checked state based on enabled state.
     * @returns @c true if all checked states are correct, @c false if a checked state got updated
     **/
    bool validateChecked();

    void createAvailableCategories();

    std::vector<QString> m_categories;
    std::vector<std::vector<Bridge>> m_sinkBridges;
    std::vector<std::vector<Bridge>> m_sourceBridges;
    std::vector<fliplib::GraphContainer> m_cachedGraphs;
    std::vector<bool> m_enabled;
    std::vector<bool> m_checked;

    std::map<QByteArray, std::pair<QUuid, std::vector<QUuid>>> m_generatedGraphIds;
    QStringList m_availableCategories;
    QFileSystemWatcher *m_watcher;
    QString m_systemDir;
    QString m_userDir;
};

}
}

#pragma once
#include "abstractNodeController.h"

#include <QObject>
#include <QPointF>

class GroupControllerTest;

namespace fliplib
{
class FilterGroup;
}

namespace qan
{
class GroupItem;
class Node;
}

namespace precitec
{
namespace gui
{
namespace components
{
namespace grapheditor
{

class FilterGroup;
class PlausibilityController;

/**
 * Class for group related functionality.
 **/
class GroupController : public AbstractNodeController
{
    Q_OBJECT
    Q_PROPERTY(precitec::gui::components::grapheditor::PlausibilityController *plausibilityController READ plausibilityController WRITE setPlausibilityController NOTIFY plausibilityControllerChanged)
    Q_PROPERTY(bool canBeExportedToMacro READ canBeExportedToMacro NOTIFY canBeExportedToMacroChanged)
public:
    GroupController(QObject *parent = nullptr);
    ~GroupController();
    void insertAllFilterGroups();

    PlausibilityController *plausibilityController() const
    {
        return m_plausibilityController;
    }
    void setPlausibilityController(PlausibilityController *plausibilityController);

    void addToGroup(qan::Node *node, FilterGroup *group);
    void ungroup(qan::Node *node);
    Q_INVOKABLE void updateGroupLabel(precitec::gui::components::grapheditor::FilterGroup *group, const QString &label);
    Q_INVOKABLE void updateGroupPosition(precitec::gui::components::grapheditor::FilterGroup *group, const QPointF &point);
    Q_INVOKABLE void updateGroupSize(precitec::gui::components::grapheditor::FilterGroup *group, const QSizeF &size);
    Q_INVOKABLE void updateGroupProperty(QObject* node);
    Q_INVOKABLE void updateGroupContent(QObject* node);

    void updateContentPosition(FilterGroup *group);

    int getLowestFilterGroupID(unsigned int maximum, unsigned int actualValue) const;
    QString getGroupDefaultName(int groupID);
    FilterGroup *insertNewGroup(const QPointF &point);
    FilterGroup *insertNewGroup(const fliplib::FilterGroup &newGroup, const QPointF &point = {0, 0});

    int idFromName(const QString &name) const;

    FilterGroup *find(int id) const;

    void connectNodesToFilterGroups(const fliplib::GraphContainer &copyGraph);
    void connectNodesToFilterGroups();

    bool canBeExportedToMacro() const
    {
        return m_canBeExportedToMacro;
    }

    Q_INVOKABLE void exportSelectedToMacro(const QString &directory, const QString &name, const QString &comment);

Q_SIGNALS:
    void canBeExportedToMacroChanged();
    void plausibilityControllerChanged();

private:
    void insertOneFilterGroup(int actualGroup);
    void updateGroup(qan::Node *node, int id);
    FilterGroup *createGroupInGraph(int groupID, const QString &label, const QPointF &position);
    void initGroupItem(qan::GroupItem *groupItem, const QPointF &position);

    int getGroupID(int actualGroup) const;
    void initSelectedGroupsModel();
    void checkCanBeExportedToMacro();
    void setCanBeExportedToMacro(bool set);

    bool m_canBeExportedToMacro = false;
    PlausibilityController *m_plausibilityController = nullptr;
    QMetaObject::Connection m_plausibilityDestroyed;

    friend GroupControllerTest;
};

}
}
}
}

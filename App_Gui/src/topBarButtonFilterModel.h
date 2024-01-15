#pragma once

#include <QSortFilterProxyModel>

namespace precitec
{

namespace gui
{

class TopBarButtonFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(bool showGraphEditor READ showGraphEditor WRITE setShowGraphEditor NOTIFY showGraphEditorChanged)
    Q_PROPERTY(bool headMonitorAvailable READ isHeadMonitorAvailable WRITE setHeadMonitorAvailable NOTIFY headMonitorAvailableChanged)

public:
    explicit TopBarButtonFilterModel(QObject *parent = nullptr);
    ~TopBarButtonFilterModel() override;

    bool showGraphEditor() const
    {
        return m_showGraphEditor;
    }
    void setShowGraphEditor(bool value);

    bool isHeadMonitorAvailable() const
    {
        return m_headMonitorAvailable;
    }
    void setHeadMonitorAvailable(bool set);

Q_SIGNALS:
    void showGraphEditorChanged();
    void headMonitorAvailableChanged();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    bool m_showGraphEditor{false};
    bool m_headMonitorAvailable{false};
};

}
}
Q_DECLARE_METATYPE(precitec::gui::TopBarButtonFilterModel*)



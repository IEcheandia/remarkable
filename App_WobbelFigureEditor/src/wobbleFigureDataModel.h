#pragma once

#include <QAbstractTableModel>
#include <QList>
#include "LaserPoint.h"
#include "WobbleFigure.h"
#include "WobbleFigureEditor.h"
#include <QuickQanava.h>
#include "fileType.h"

namespace precitec
{
namespace scantracker
{
namespace components
{
namespace wobbleFigureEditor
{

class WobbleFigureDataModel : public QAbstractTableModel
{
  Q_OBJECT
  QML_ELEMENT
  Q_PROPERTY(precitec::scantracker::components::wobbleFigureEditor::WobbleFigure* figure READ figure WRITE setFigure NOTIFY figureChanged)
  Q_PROPERTY(precitec::scantracker::components::wobbleFigureEditor::WobbleFigureEditor* figureEditor READ figureEditor WRITE setFigureEditor NOTIFY figureEditorChanged)
  Q_PROPERTY(qan::GraphView* graphView READ graphView WRITE setGraphView NOTIFY graphViewChanged)

  Q_ENUMS(Roles)
public:
    enum Roles {
        IdRole = Qt::DisplayRole,
        xRole = Qt::UserRole,
        yRole,
        laserPowerCheckRole,
        laserPowerRole,
        ringPowerCheckRole,
        ringPowerRole,
        velocityCheckRole,
        velocityRole
    };
public:
  explicit WobbleFigureDataModel(QObject *parent = nullptr);
  ~WobbleFigureDataModel() override;
  QHash<int, QByteArray> roleNames() const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;

public:
  /**
  Used to restrict subsequent calls of @ref searchForNewLaserPoints() to report only points that have enabled QML-items and have a center in the circle.

  @param center Center of the cricle (ignored if radius <= 0)
  @param radius Radius of the circle, use a non-positive value to disable the filter.
  */
  Q_INVOKABLE void setFilterCircle(QPointF center, qreal radius);
  Q_INVOKABLE void searchForNewLaserPoints();
  Q_INVOKABLE int columnWidth(int column);
  Q_INVOKABLE int contentWidth();
  Q_INVOKABLE bool getPowerFiledEnabled(int row, int column) const;
  Q_INVOKABLE void setColumnWidth();
  Q_INVOKABLE void setSelection(int row, int viewWidth, int viewHeight, int tableViewWidth);
  Q_INVOKABLE void figureModelChanged();
  Q_INVOKABLE void setFileType(precitec::scantracker::components::wobbleFigureEditor::FileType fileType);
  Q_INVOKABLE bool isColumnVisible(int columnNumber);
  Q_INVOKABLE int activeColumnsCount();
  Q_INVOKABLE void resetAllData();
  Q_INVOKABLE std::vector<int> getIds() const;

public:
  WobbleFigure* figure() const;
  void setFigure(WobbleFigure* wobbleFigure);
  WobbleFigureEditor* figureEditor() const;
  void setFigureEditor(WobbleFigureEditor* wobbleFigureEditor);
  qan::GraphView* graphView() const;
  void setGraphView(qan::GraphView* graphView);

Q_SIGNALS:
    void figureChanged();
    void figureEditorChanged();
    void graphViewChanged();
    void updated(QObject* object);
    void dataUpdated();

private:
  bool getPowerChecked(double value) const;
  QString getCurrentOrPreviewsPowerValue(int role, int currentRow) const;
  double getBoolPowerValue(int role, bool value);

  void setRoleNames();

  // TODO: outsource in a utils class or something else
  QString convertfloatTo2PointedQString(float value) const;
  QString convertfloatTo0PointedQString(float value) const;
  float convertQStringToFloat(QString value) const;

private:
  QList<LaserPoint*> mData;
  QHash<int, QByteArray> mRoles;
  QList<int> mColumnWidth;
  int mContentWidth;
  int mActiveColumnCount;
  WobbleFigure* m_figure = nullptr;
  WobbleFigureEditor* m_figureEditor = nullptr;
  QMetaObject::Connection m_figureEditorDestroyedConnection;
  qan::GraphView* m_graphView = nullptr;
  QPointF m_graphViewCenterPosition;
  precitec::scantracker::components::wobbleFigureEditor::FileType m_fileType;
  qreal mFilterRadius = 0;
  std::optional<QPointF> mFilterCenter;
};

} // namepsace wobbleFigureEditor
} // namepsace components
} // namepsace scantracker
} // namepsace precitec

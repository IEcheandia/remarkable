#include "wobbleFigureDataModel.h"
#include "figureEditorSettings.h"

using precitec::scanmaster::components::wobbleFigureEditor::FigureEditorSettings;

namespace precitec
{
namespace scantracker
{
namespace components
{
namespace wobbleFigureEditor
{

WobbleFigureDataModel::WobbleFigureDataModel(QObject *parent) : QAbstractTableModel(parent)
{
    setRoleNames();
    setColumnWidth();
}

WobbleFigureDataModel::~WobbleFigureDataModel() = default;

QHash<int, QByteArray> WobbleFigureDataModel::roleNames() const
{
    return mRoles;
}

int WobbleFigureDataModel::rowCount(const QModelIndex& parent) const
{
    if(parent.isValid())
        return 0;
    return mData.size();
}

int WobbleFigureDataModel::columnCount(const QModelIndex& parent) const
{
    if(parent.isValid())
        return 0;
    return roleNames().size();
}

QVariant WobbleFigureDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }
    // TODO: maybe turn it back to switch-case
    if (role == IdRole)
    {
        return QString().setNum(mData[index.row()]->ID());
    }
    else if (role == xRole)
    {
        if (!m_figureEditor)
        {
            Q_ASSERT(false);
            return {};
        }

        return convertfloatTo2PointedQString(mData[index.row()]->center().x() / m_figureEditor->figureScale());
    }
    else if (role == yRole)
    {
        if (!m_figureEditor)
        {
            Q_ASSERT(false);
            return {};
        }

        return convertfloatTo2PointedQString(-1 * mData[index.row()]->center().y() / m_figureEditor->figureScale());
    }
    else if (role == laserPowerCheckRole)
    {
        return getPowerChecked(mData[index.row()]->laserPower());
    }
    else if (role == laserPowerRole)
    {
        // Workaround - cause in qml the element can't access to the column before
        QVariantList list;
        list << getCurrentOrPreviewsPowerValue(laserPowerRole, index.row());
        list << !getPowerChecked(mData[index.row()]->laserPower());
        return list;
    }
    else if (role == ringPowerCheckRole)
    {
        return getPowerChecked(mData[index.row()]->ringPower());
    }
    else if (role == ringPowerRole)
    {
        // Workaround - cause in qml the element can't access to the column before
        QVariantList list;
        list << getCurrentOrPreviewsPowerValue(ringPowerRole, index.row());
        list << !getPowerChecked(mData[index.row()]->ringPower());
        return list;
    }
    else if (role == velocityCheckRole)
    {
        return getPowerChecked(mData[index.row()]->velocity());
    }
    else if (role == velocityRole)
    {
        // Workaround - cause in qml the element can't access to the column before
        QVariantList list;
        list << getCurrentOrPreviewsPowerValue(velocityRole, index.row());
        list << !getPowerChecked(mData[index.row()]->velocity());
        return list;
    }
    return QVariant();
}

bool WobbleFigureDataModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role >= Qt::UserRole)
    {
        mData[index.row()]->setEditable(true);
        QPointF newPosition;
        switch (index.column()) {
        case 1:
            newPosition = QPointF(convertQStringToFloat(value.toString()) * m_figureEditor->figureScale(), mData[index.row()]->center().y());
            mData[index.row()]->setCenter(newPosition);
            m_figureEditor->updatePosition(mData[index.row()]);
            emit dataChanged(index, index);
            return true;
            break;
        case 2:
            newPosition = QPointF(mData[index.row()]->center().x(), -1 * convertQStringToFloat(value.toString()) * m_figureEditor->figureScale());
            mData[index.row()]->setCenter(newPosition);
            m_figureEditor->updatePosition(mData[index.row()]);
            emit dataChanged(index, index);
            return true;
            break;
        case 3:
            mData[index.row()]->setLaserPower(getBoolPowerValue(laserPowerRole, value.toBool()));
            break;
        case 4:
            mData[index.row()]->setLaserPower(value.toDouble());
            break;
        case 5:
            mData[index.row()]->setRingPower(getBoolPowerValue(ringPowerRole, value.toBool()));
            break;
        case 6:
            mData[index.row()]->setRingPower(value.toDouble());
            break;
        case 7:
            mData[index.row()]->setVelocity(getBoolPowerValue(velocityRole, value.toBool()));
            break;
        case 8:
            mData[index.row()]->setVelocity(value.toDouble());
            break;
        }
        m_figureEditor->updateProperties(mData[index.row()]);
        emit dataChanged(this->index(0, 0, QModelIndex()), this->index(rowCount()-1, columnCount()-1, QModelIndex()));
        return true;
    }
    return false;
}

QVariant WobbleFigureDataModel::headerData(int section, Qt::Orientation orientation, int) const
{
    if (orientation == Qt::Horizontal)
    {
        switch (section) {
        case 0:
            return QString("ID");
        case 1:
            return QString("X-Coord [mm]");
        case 2:
            return QString("Y-Coord [mm]");
        case 3:
            return QString(u8"\u21B6");
        case 4:
            return QString("LaserPower [%]");
        case 5:
            return QString(u8"\u21B6");
        case 6:
            return QString("RingPower [%]");
        case 7:
            return QString(u8"\u21B6");
        case 8:
            return QString("Velocity [mm/s]");
        }
    }
    return QVariant();
}

Qt::ItemFlags WobbleFigureDataModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

void WobbleFigureDataModel::setFilterCircle(QPointF center, qreal radius)
{
    if (radius <= 0)
    {
        mFilterCenter.reset();
        mFilterRadius = 0;
    }
    else
    {
        mFilterCenter = center;
        mFilterRadius = radius;
    }
}

void WobbleFigureDataModel::searchForNewLaserPoints()
{
    resetAllData();
    if(nullptr != m_figure)
    {
        std::vector<LaserPoint*> newLaserPoints = m_figure->getLaserPoints();
        if(newLaserPoints.size() == 0)
        {
            return;
        }
        mData.reserve(newLaserPoints.size());

        qreal const sqR = mFilterRadius * mFilterRadius;
        std::copy_if(newLaserPoints.begin(), newLaserPoints.end(), std::back_inserter(mData), [&](LaserPoint const* p)
                     {
            if (!mFilterCenter)
                return true;

            if (!p->getItem()->isEnabled())
                return false;

            QPointF v = p->center() - *mFilterCenter;
            return QPointF::dotProduct(v, v) < sqR; });

        setColumnWidth();
        emit dataChanged(index(0, 0, QModelIndex()), index(rowCount()-1, columnCount()-1, QModelIndex()));
        emit dataUpdated();
    }
}

int WobbleFigureDataModel::columnWidth(int column)
{
    if(column > mColumnWidth.size()-1)
    {
        return 0;
    }
    return mColumnWidth.at(column);
}

int WobbleFigureDataModel::contentWidth()
{
    return mContentWidth;
}

bool WobbleFigureDataModel::getPowerFiledEnabled(int row, int column) const
{
    if(column == 4)
    {
        return getPowerChecked(mData[row]->laserPower());
    }
    else if(column == 6)
    {
        return getPowerChecked(mData[row]->ringPower());
    }
    else if(column == 8)
    {
        return getPowerChecked(mData[row]->velocity());
    }
    else{
        return false;
    }
}

WobbleFigure * WobbleFigureDataModel::figure() const
{
    return m_figure;
}

void WobbleFigureDataModel::setFigure(WobbleFigure* wobbleFigure)
{
    if (m_figure == wobbleFigure)
    {
        return;
    }
    m_figure = wobbleFigure;
    emit figureChanged();
}

WobbleFigureEditor *WobbleFigureDataModel::figureEditor() const
{
    return m_figureEditor;
}

void WobbleFigureDataModel::setFigureEditor(WobbleFigureEditor *wobbleFigureEditor)
{
    if(m_figureEditor == wobbleFigureEditor)
    {
        return;
    }

    if (m_figureEditor)
    {
        //disconnect(m_figureEditor, &WobbleFigureEditor::dataChanged, this, &WobbleFigureDataModel::searchForNewLaserPoints);
    }
    disconnect(m_figureEditorDestroyedConnection);
    m_figureEditor = wobbleFigureEditor;
    if (m_figureEditor)
    {
        m_figureEditorDestroyedConnection = connect(m_figureEditor, &QObject::destroyed, this, std::bind(&WobbleFigureDataModel::setFigureEditor, this, nullptr));
        //connect(m_figureEditor, &WobbleFigureEditor::dataChanged, this, &WobbleFigureDataModel::searchForNewLaserPoints);
    }
    else
    {
        m_figureEditorDestroyedConnection = {};
    }
    emit figureEditorChanged();
}

qan::GraphView *WobbleFigureDataModel::graphView() const
{
    return m_graphView;
}

void WobbleFigureDataModel::setGraphView(qan::GraphView *graphView)
{
    if (m_graphView == graphView)
    {
        return;
    }
    m_graphView = graphView;
    m_graphViewCenterPosition = m_graphView->getContainerItem()->position();
    emit graphViewChanged();
}

bool WobbleFigureDataModel::getPowerChecked(double value) const
{
    if(value < 0)
    {
        return true;
    }
    return false;
}

QString WobbleFigureDataModel::getCurrentOrPreviewsPowerValue(int role, int currentRow) const
{
    QString invalidNumberString("-1");
    if(currentRow < 0 || currentRow > rowCount())
    {
        return invalidNumberString;
    }
    for(int i = currentRow; i > -1; i--)
    {
        double value = 0;
        if(role == laserPowerRole)
        {
            value = mData[i]->laserPower();
        }
        else if(role == ringPowerRole)
        {
            value = mData[i]->ringPower();
        }
        else if(role == velocityRole)
        {
            value = mData[i]->velocity();
        }
        if(value > -1)
        {
            return convertfloatTo0PointedQString(value);
        }
    }
    return invalidNumberString;
}

double WobbleFigureDataModel::getBoolPowerValue(int role, bool value)
{
    if(value)
    {
        return -1;
    }
    if(role == velocityRole)
    {
        return 1;
    }
    return 0;
}

void WobbleFigureDataModel::setRoleNames()
{
    mRoles[IdRole] = "Id";
    mRoles[xRole] = "xCoord";
    mRoles[yRole] = "yCoord";
    mRoles[laserPowerCheckRole] = "laserPowerCheckable";
    mRoles[laserPowerRole] = "laserPower";
    mRoles[ringPowerCheckRole] = "ringPowerCheckable";
    mRoles[ringPowerRole] = "ringPower";
    mRoles[velocityCheckRole] = "velocityCheckable";
    mRoles[velocityRole] = "velocity";
}

void WobbleFigureDataModel::setColumnWidth()
{
    mColumnWidth.clear();
    mColumnWidth.push_back(55);   // id
    mColumnWidth.push_back(120);  // xCoord
    mColumnWidth.push_back(120);  // yCoord
    mColumnWidth.push_back(45);   // laserPowerCheckable
    mColumnWidth.push_back(120);  // laserPower

    if(FigureEditorSettings::instance()->dualChannelLaser())
    {
        mColumnWidth.push_back(45);     // ringPowerCheckable
        mColumnWidth.push_back(120);    // ringPower
    }
    else
    {
        mColumnWidth.push_back(0);  // ringPowerCheckable
        mColumnWidth.push_back(0);  // ringPower
    }
    if(FileType::Wobble != m_fileType)
    {
        mColumnWidth.push_back(45);     // velocityCheckable
        mColumnWidth.push_back(120);    // velocity
    }
    else
    {
        mColumnWidth.push_back(0);  // velocityCheckable
        mColumnWidth.push_back(0);  // velocity
    }

    mContentWidth = 0;
    mActiveColumnCount = 0;
    for(int i = 0; i < mColumnWidth.size(); i++)
    {
        if(mColumnWidth.at(i) > 0)
        {
            mActiveColumnCount++;
        }
        mContentWidth += mColumnWidth.at(i);
    }
}

void WobbleFigureDataModel::setSelection(int row, int viewWidth, int viewHeight, int tableViewWidth)
{
    if(nullptr != m_figure)
    {
        LaserPoint& lp = *m_figure->searchLaserPoint(row);
        m_figure->selectNode(lp);

        if(nullptr != m_graphView)
        {
            qan::Graph::SelectedNodes& nodes = m_figure->getSelectedNodes();
            for(auto const& node : nodes)
            {
                QQuickItem* containerItem = m_graphView->getContainerItem();
                if(nullptr == containerItem)
                {
                    return;
                }
                qan::NodeItem* item = node->getItem();

                containerItem->setScale(1.0);
                m_graphView->setZoom(1.0);
                const qreal zoom = containerItem->scale();

                // 1.
                const QPointF itemPos = containerItem->mapToItem(item, QPointF{-item->width() / 2., -item->height() / 2.});
                QPointF containerPos{0., 0.};
                containerPos.rx() = itemPos.x() * zoom;
                containerPos.ry() = itemPos.y() * zoom;
                containerItem->setPosition(containerPos);

                // 2.
                const QPointF viewCenter = QPointF{(viewWidth-tableViewWidth) / 2., viewHeight / 2.};
                const QPointF viewCenterContainerCs = containerItem->mapToItem(containerItem, viewCenter);
                const QPointF viewOriginContainerCs = containerItem->mapToItem(containerItem, QPointF{0, 0});
                const QPointF translationContainerCs = viewCenterContainerCs - viewOriginContainerCs;
                containerItem->setPosition(containerItem->position() + (translationContainerCs * zoom));
                return;
            }
        }
    }
}

void WobbleFigureDataModel::figureModelChanged()
{
    if(nullptr != m_graphView)
    {
        m_graphViewCenterPosition = m_graphView->getContainerItem()->position();
    }
}

void WobbleFigureDataModel::setFileType(FileType fileType)
{
    m_fileType = fileType;
    //searchForNewLaserPoints();
}

bool WobbleFigureDataModel::isColumnVisible(int columnNumber)
{
    if (!FigureEditorSettings::instance()->dualChannelLaser()) {
        if (5 == columnNumber || 6 == columnNumber) {
            return false;
        }
    }
    if (FileType::Wobble == m_fileType) {
        if (7 == columnNumber || 8 == columnNumber) {
            return false;
        }
    }
    return true;
}

int WobbleFigureDataModel::activeColumnsCount()
{
    return mActiveColumnCount;
}

void WobbleFigureDataModel::resetAllData()
{
    beginResetModel();
    mData.clear();
    mColumnWidth.clear();
    mContentWidth = 0;
    setColumnWidth();
    endResetModel();
    emit dataUpdated();
}

std::vector<int> WobbleFigureDataModel::getIds() const
{
    std::vector<int> ret;
    ret.reserve(mData.size());
    for (LaserPoint* lp : mData)
    {
        ret.push_back(lp->ID());
    }

    return ret;
}

QString WobbleFigureDataModel::convertfloatTo2PointedQString(float value) const
{
    return QLocale(QLocale()).toString(value, 'f', 2);
}

QString WobbleFigureDataModel::convertfloatTo0PointedQString(float value) const
{
    QString valueString(QLocale(QLocale()).toString(value, 'f', 0));
    valueString.remove(".");
    valueString.remove(",");
    return valueString;
}

float WobbleFigureDataModel::convertQStringToFloat(QString value) const
{
    if(value.isEmpty())
    {
        return 0;
    }
    bool conversionSuccess;
    float convertedValue = QLocale().toFloat(value, &conversionSuccess);
    if(!conversionSuccess)
    {
        QString errorCode ("conversion to float fails with value: " + value);
        throw std::logic_error(errorCode.toStdString());
    }
    return convertedValue;
}

} // namepsace wobbleFigureEditor
} // namepsace components
} // namepsace scantracker
} // namepsace precitec

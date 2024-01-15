#include "fileSortModel.h"

namespace precitec
{

namespace scantracker
{

namespace components
{

namespace wobbleFigureEditor
{

FileSortModel::FileSortModel(QObject* parent) : QSortFilterProxyModel(parent)
{
    sort(0);
}

FileSortModel::~FileSortModel() = default;

void FileSortModel::setFileType(precitec::scantracker::components::wobbleFigureEditor::FileType newType)
{
    if (m_fileType == newType)
    {
        return;
    }
    m_fileType = newType;
    emit fileTypeChanged();
}

void FileSortModel::setScanMasterMode(bool isScanMaster)
{
    if (m_scanMasterMode == isScanMaster)
    {
        return;
    }
    m_scanMasterMode = isScanMaster;
    emit scanMasterModeChanged();
}

bool FileSortModel::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
{
    auto leftData = static_cast<FileType> (sourceModel()->data(source_left, Qt::UserRole + 1).toInt());
    auto rightData = static_cast<FileType> (sourceModel()->data(source_right, Qt::UserRole + 1).toInt());

    if (leftData != rightData)                      //Filter file types: Seam, Wobbel, Overlay, Basic
    {
        return leftData < rightData;
    }

    switch (leftData)                               //Filter condition is id within one type
    {
        case FileType::Seam:
        {
            auto leftDataNumber = sourceModel()->data(source_left, Qt::UserRole + 4).toString().remove("weldingSeam").remove(".json").toInt();
            auto rightDataNumber = sourceModel()->data(source_right, Qt::UserRole + 4).toString().remove("weldingSeam").remove(".json").toInt();
            return leftDataNumber < rightDataNumber;
        }
        case FileType::Wobble:
        {
            auto leftDataNumber = sourceModel()->data(source_left, Qt::UserRole + 4).toString().remove("figureWobble").remove(".json").toInt();
            auto rightDataNumber = sourceModel()->data(source_right, Qt::UserRole + 4).toString().remove("figureWobble").remove(".json").toInt();
            return leftDataNumber < rightDataNumber;
        }
        case FileType::Overlay:
        {
            auto leftDataNumber = sourceModel()->data(source_left, Qt::UserRole + 4).toString().remove("overlayFunction").remove(".json").toInt();
            auto rightDataNumber = sourceModel()->data(source_right, Qt::UserRole + 4).toString().remove("overlayFunction").remove(".json").toInt();
            return leftDataNumber < rightDataNumber;
        }
        case FileType::Basic:
        {
            auto leftID = sourceModel()->data(source_left, Qt::UserRole + 3).toString().toInt();
            auto rightID = sourceModel()->data(source_right, Qt::UserRole + 3).toString().toInt();
            return leftID < rightID;
        }
        default:
        { // sort by name
            QString left = sourceModel()->data(source_left, Qt::UserRole + 4).toString();
            QString right = sourceModel()->data(source_right, Qt::UserRole + 4).toString();

            return left < right;
        }
    }
}

bool FileSortModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const auto sourceIndex = sourceModel()->index(source_row, 0, source_parent);
    if (!sourceIndex.isValid())
    {
        return false;
    }

    const auto& rowFileType = static_cast<FileType> (sourceIndex.data(Qt::UserRole + 1).toInt());

    if (m_scanMasterMode && rowFileType == FileType::Basic)
    {
        return false;
    }

    if (!m_scanMasterMode && (rowFileType == FileType::Seam || rowFileType == FileType::Overlay))
    {
        return false;
    }

    return rowFileType == m_fileType || m_fileType == FileType::None;
}

}
}
}
}

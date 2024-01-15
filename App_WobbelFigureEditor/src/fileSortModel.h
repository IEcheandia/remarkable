#pragma once

#include <QSortFilterProxyModel>
#include "fileType.h"

namespace precitec
{

namespace scantracker
{

namespace components
{

namespace wobbleFigureEditor
{

/**
 * The filter sort model is used to sort all files from the file model or to show only one file type.
 * The models sorts the files depending on the type to "Seam", "Wobbel, "Overlay", "Basic".
 * Within the single types the files are sorted by the id upwards.
 **/
class FileSortModel : public QSortFilterProxyModel
{
    Q_OBJECT
    /**
     *  File type which will be still shown.
     **/
    Q_PROPERTY(precitec::scantracker::components::wobbleFigureEditor::FileType fileType READ fileType WRITE setFileType NOTIFY fileTypeChanged)
    /**
     *  ScanMaster mode
     *  This property is used to check if basic wobble figures are displayed in the load dialog.
     *  In scanmaster mode basic wobble figures are hidden.
     **/
    Q_PROPERTY(bool scanMasterMode READ scanMasterMode WRITE setScanMasterMode NOTIFY scanMasterModeChanged)

public:
    explicit FileSortModel( QObject* parent = nullptr);
    ~FileSortModel() override;

    FileType fileType() const
    {
        return m_fileType;
    }
    void setFileType(FileType newType);

    bool scanMasterMode() const
    {
        return m_scanMasterMode;
    }
    void setScanMasterMode(bool isScanMaster);

Q_SIGNALS:
    void fileTypeChanged();
    void scanMasterModeChanged();

protected:
    bool lessThan(const QModelIndex & source_left, const QModelIndex & source_right) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    FileType m_fileType{FileType::None};
    bool m_scanMasterMode = true;

};

}
}
}
}

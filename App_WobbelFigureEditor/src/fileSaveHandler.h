#pragma once

#include <QObject>
#include "fileType.h"

class FileSaveHandlerTest;
namespace precitec
{
namespace scantracker
{
namespace components
{
namespace wobbleFigureEditor
{
class FileModel;

using precitec::scantracker::components::wobbleFigureEditor::FileType;

/**
 *  File save handler is used to fille the save and save as dialog correctly. It also checks if the current number for saving the file is already used.
 **/
class FileSaveHandler : public QObject
{
    Q_OBJECT
    /**
     *  Contains the current type of the figure.
     *  It's used to determine the correct prefix and to show the correctly file type on the dialog.
     **/
    Q_PROPERTY(precitec::scantracker::components::wobbleFigureEditor::FileType type READ type WRITE setType NOTIFY typeChanged)
    /**
     *  Contains the current number which is added to the filename.
     *  It's used to add a separate file for every figure which is saved.
     *  Should be always the ID of the figure, but this is managed by the wobble figure editor.
     **/
    Q_PROPERTY(int number READ number WRITE setNumber NOTIFY numberChanged)
    /**
     *  Contains if the current entered or selected number is already used by another figure.
     *  It's used to show a warning on the dialog that the file is overriden if using this number.
     **/
    Q_PROPERTY(bool numberAlreadyUsed READ numberAlreadyUsed WRITE setNumberAlreadyUsed NOTIFY numberAlreadyUsedChanged)
    /**
     *  Contains an indicator if the previous or next free number is searched for using the save as functionality.
     **/
    Q_PROPERTY(bool searchLowerNumber READ searchLowerNumber WRITE setSearchLowerNumber NOTIFY searchLowerNumberChanged)
    /**
     *  Contains the current prefix of the filename. The prefix depends on the file type.
     *  It's used to show the correct filename in the dialog.
     **/
    Q_PROPERTY(QString filePrefix READ filePrefix NOTIFY filePrefixChanged)
    /**
     *  Contains the current label for the file type.
     *  It's used to show the correct file type in the dialog.
     **/
    Q_PROPERTY(QString fileTypeLabel READ fileTypeLabel NOTIFY fileTypeLabelChanged)
    /**
     *   File model stores all files and the filenames.
     *   It's used to check if the current number is already used by this class.
     **/
    Q_PROPERTY(precitec::scantracker::components::wobbleFigureEditor::FileModel *fileModel READ fileModel WRITE setFileModel NOTIFY fileModelChanged)

public:
    explicit FileSaveHandler(QObject* parent = nullptr);
    ~FileSaveHandler() override;

    FileType type() const
    {
        return m_type;
    }
    void setType(FileType newType);

    int number() const
    {
        return m_number;
    }
    void setNumber(int newNumber);

    bool numberAlreadyUsed() const
    {
        return m_numberAlreadyUsed;
    }
    void setNumberAlreadyUsed(bool newState);

    bool searchLowerNumber() const
    {
        return m_searchLowerNumber;
    }
    void setSearchLowerNumber(bool newState);

    QString filePrefix() const
    {
        return m_namePrefix;
    }

    QString fileTypeLabel() const
    {
        return m_fileTypeLabel;
    }

    FileModel* fileModel() const
    {
        return m_fileModel;
    }
    void setFileModel(FileModel *model);

    Q_INVOKABLE void searchAvailableNumber();
    Q_INVOKABLE void checkIfNumberIsAlreadyUsed();

Q_SIGNALS:
    void typeChanged();
    void numberChanged();
    void numberAlreadyUsedChanged();
    void searchLowerNumberChanged();
    void fileModelChanged();
    void filePrefixChanged();
    void fileTypeLabelChanged();

private:
    void setTypeAndPrefix();
    int getNumberFromFileName(const QString &fileName);

    FileType m_type = FileType::None;
    int m_number = 0;
    bool m_numberAlreadyUsed = false;
    bool m_searchLowerNumber = false;

    QString m_namePrefix;
    FileType m_fileType = FileType::None;
    QString m_fileTypeLabel;

    FileModel *m_fileModel = nullptr;
    QMetaObject::Connection m_fileModelDestroyedConnection;

    friend FileSaveHandlerTest;
};

}
}
}
}

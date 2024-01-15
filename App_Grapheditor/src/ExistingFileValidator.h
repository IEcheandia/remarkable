#pragma once
#include "DirectoryModel.h"
#include <QValidator>

#include <QAbstractListModel>

namespace precitec
{

namespace storage
{
class AbstractGraphModel;
}

namespace gui
{

namespace components
{

namespace grapheditor
{

class ExistingFileValidator : public QValidator
{
    Q_OBJECT

public:
    ExistingFileValidator(QObject *parent = nullptr);
    ~ExistingFileValidator() override;

    Q_INVOKABLE void setDirectoryModel(DirectoryModel* directoryModel);
    Q_INVOKABLE void setDirectoryIndex(const QModelIndex &index);
    QValidator::State validate(QString &input, int &pos) const override;

    Q_INVOKABLE void setCurrentDirectory(const QString &dir);


signals:
        void directoryModelChanged();

private:
    DirectoryModel *m_directoryModel = nullptr;
    QModelIndex m_directoryModelIndex;
    QString m_currentDirectory;
};

}
}
}
}
Q_DECLARE_METATYPE(precitec::gui::components::grapheditor::ExistingFileValidator*)


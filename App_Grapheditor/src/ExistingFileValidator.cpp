#include "ExistingFileValidator.h"

using namespace precitec::gui::components::grapheditor;

    ExistingFileValidator::ExistingFileValidator ( QObject *parent)
    : QValidator ( parent )
{
}

ExistingFileValidator::~ExistingFileValidator() = default;

void ExistingFileValidator::setDirectoryModel(DirectoryModel* directoryModel)
{
    m_directoryModel = directoryModel;
    emit directoryModelChanged();
}

void ExistingFileValidator::setDirectoryIndex(const QModelIndex& index)
{
    m_directoryModelIndex = index;
}

void ExistingFileValidator::setCurrentDirectory(const QString &dir)
{
    m_currentDirectory = dir;
}

QValidator::State ExistingFileValidator::validate ( QString &input, int &pos ) const
{
    if(!m_directoryModel)
    {
        return QValidator::Invalid;
    }

    if(m_directoryModel->fileNameExists(input,m_directoryModel->getDirectoryType(m_directoryModelIndex)))
    {
        return QValidator::Intermediate;
    }else
    {
        return  QValidator::Acceptable;
    }
}

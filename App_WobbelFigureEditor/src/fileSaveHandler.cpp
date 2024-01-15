#include "fileSaveHandler.h"
#include "FileModel.h"
#include <QString>

using namespace precitec::scantracker::components::wobbleFigureEditor;
using precitec::scantracker::components::wobbleFigureEditor::FileType;

FileSaveHandler::FileSaveHandler(QObject* parent) : QObject(parent)
{
    connect(this, &FileSaveHandler::typeChanged, this, &FileSaveHandler::searchAvailableNumber);
    connect(this, &FileSaveHandler::numberChanged, this, &FileSaveHandler::checkIfNumberIsAlreadyUsed);
}

FileSaveHandler::~FileSaveHandler() = default;

void FileSaveHandler::setType(FileType newType)
{
    if (m_type == newType)
    {
        return;
    }

    m_type = newType;
    emit typeChanged();
}

void FileSaveHandler::setNumber(int newNumber)
{
    if (m_number == newNumber)
    {
        return;
    }

    m_number = newNumber;
    emit numberChanged();
}

void FileSaveHandler::setNumberAlreadyUsed(bool newState)
{
    if (m_numberAlreadyUsed == newState)
    {
        return;
    }

    m_numberAlreadyUsed = newState;
    emit numberAlreadyUsedChanged();
}

void FileSaveHandler::setSearchLowerNumber(bool newState)
{
    if (m_searchLowerNumber == newState)
    {
        return;
    }

    m_searchLowerNumber = newState;
    emit searchLowerNumberChanged();
}

void FileSaveHandler::setFileModel(FileModel* model)
{
    if (m_fileModel == model)
    {
        return;
    }

    disconnect(m_fileModelDestroyedConnection);
    m_fileModel = model;

    if (m_fileModel)
    {
        m_fileModelDestroyedConnection = connect(m_fileModel, &QObject::destroyed, this, std::bind(&FileSaveHandler::setFileModel, this, nullptr));
    }
    else
    {
        m_fileModelDestroyedConnection = {};
    }
    emit fileModelChanged();
}

void FileSaveHandler::searchAvailableNumber()
{
    if (!m_fileModel || m_fileModel->files().empty())
    {
        return;
    }

    setTypeAndPrefix();
    unsigned int currentNumber = m_number;
    const auto &files = m_fileModel->files();

    while (currentNumber >= 0 && currentNumber < 1001)
    {
        auto numberIt = std::find_if(files.begin(), files.end(), [currentNumber, this](const auto &currentElement)
        {
            if (currentElement.type == m_fileType && !currentElement.name.isEmpty())
            {
                auto name = currentElement.name;
                return static_cast<unsigned int> (getNumberFromFileName(name)) == currentNumber;
            }
            return false;
        });
        if (numberIt == files.end())
        {
            if ((unsigned int) m_number != currentNumber)
            {
                setNumber(currentNumber);
                return;
            }
        }

        if (m_searchLowerNumber)
        {
            currentNumber--;
        }
        else
        {
            currentNumber++;
        }
    }
    checkIfNumberIsAlreadyUsed();
}

void FileSaveHandler::checkIfNumberIsAlreadyUsed()
{
    if (!m_fileModel)
    {
        return;
    }

    setTypeAndPrefix();

    const auto& files = m_fileModel->files();

    auto number = std::find_if(files.begin(), files.end(), [this] (const auto &currentElement)
    {
        if (currentElement.type == m_fileType)
        {
            auto name = currentElement.name;
            return getNumberFromFileName(name) == m_number;
        }
        return false;
    });

    setNumberAlreadyUsed(number != files.end());
}

void FileSaveHandler::setTypeAndPrefix()
{
    switch (m_type)
    {
        case FileType::Seam:
            m_fileType = FileType::Seam;
            m_namePrefix = QStringLiteral("weldingSeam");
            m_fileTypeLabel = QStringLiteral("Seam figure");
            break;
        case FileType::Wobble:
            m_fileType = FileType::Wobble;
            m_namePrefix = QStringLiteral("figureWobble");
            m_fileTypeLabel = QStringLiteral("Wobble figure");
            break;
        case FileType::Overlay:
            m_fileType = FileType::Overlay;
            m_namePrefix = QStringLiteral("overlayFunction");
            m_fileTypeLabel = QStringLiteral("Overlay figure");
            break;
        case FileType::Basic:
            m_fileType = FileType::Wobble;
            m_namePrefix = QStringLiteral("figureWobble");
            m_fileTypeLabel = QStringLiteral("Wobble figure");
            break;
        default:
            return;
    }
    emit filePrefixChanged();
    emit fileTypeLabelChanged();
}

int FileSaveHandler::getNumberFromFileName(const QString& fileName)
{
    auto name = fileName;
    return name.remove(m_namePrefix).remove(".json").toInt();
}


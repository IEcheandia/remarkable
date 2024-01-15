#include "FileModel.h"
#include <QDir>
#include "jsonSupport.h"
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QTimer>
#include <QFileSystemWatcher>

using json = nlohmann::ordered_json;

namespace precitec
{
namespace scantracker
{
namespace components
{
namespace wobbleFigureEditor
{

FileModel::FileModel(QObject* parent, QString const* removableDevicesPath)
    : QAbstractListModel(parent)
{
    if (removableDevicesPath)
    {
        auto base = *removableDevicesPath;
        Q_ASSERT(QDir(base).exists());
        m_seamAndOverlayFilePath = base + QStringLiteral("/weldmaster/import/weld_figure");
        m_wobbleFilePath = base + QStringLiteral("/weldmaster/import/laser_controls");
        m_dxfFilePath = base + QStringLiteral("/weldmaster/import/dxf");
    }
    else
    {
        m_seamAndOverlayFilePath = QString::fromUtf8(qgetenv("WM_BASE_DIR")) + QStringLiteral("/config/weld_figure");
        m_wobbleFilePath = QString::fromUtf8(qgetenv("WM_BASE_DIR")) + QStringLiteral("/config/laser_controls");
        m_basicFilePath = QString::fromUtf8(qgetenv("WM_BASE_DIR")) + QStringLiteral("/system_graphs/basic_figure");
    }

    auto watcher{new QFileSystemWatcher{this}};
    connect(watcher, &QFileSystemWatcher::directoryChanged, this, &FileModel::loadFiles);

    watcher->addPath(m_seamAndOverlayFilePath);
    watcher->addPath(m_wobbleFilePath);
    if (m_basicFilePath)
        watcher->addPath(*m_basicFilePath);
    if (m_dxfFilePath)
        watcher->addPath(*m_dxfFilePath);
}

FileModel::~FileModel() = default;

void FileModel::setLoading(bool isLoading)
{
    if (m_loading == isLoading)
    {
        return;
    }
    m_loading = isLoading;
    emit loadingChanged();
}

void FileModel::setFileType(FileType newFileType)
{
    if (m_fileType == newFileType)
    {
        return;
    }
    m_fileType = newFileType;
    emit fileTypeChanged();
}

QHash<int, QByteArray> FileModel::roleNames() const
{
    //DisplayRole and UserRole + 1 used in fileSortModel in lessThan()
    //UserRole + 1 used in fileSortModel in filterAcceptsRow()
    return {
        {Qt::DisplayRole, QByteArrayLiteral("name")},
        {Qt::UserRole, QByteArrayLiteral("path")},
        {Qt::UserRole + 1, QByteArrayLiteral("type")},
        {Qt::UserRole + 2, QByteArrayLiteral("typeName")},
        {Qt::UserRole + 3, QByteArrayLiteral("id")},
        {Qt::UserRole + 4, QByteArrayLiteral("fileName")},
        {Qt::UserRole + 5, QByteArrayLiteral("visibleName")},
    };
}

QVariant FileModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    const auto &file = m_files.at(index.row());

    switch (role)
    {
        case Qt::DisplayRole:
            return file.visibleName.isEmpty() ? file.name : file.visibleName + QStringLiteral(" (") + QString::fromStdString(file.id) + QStringLiteral(")");
        case Qt::UserRole:
            return file.path;
        case Qt::UserRole + 1:
            return QVariant::fromValue(file.type);
        case Qt::UserRole + 2:
            return typeToString(file.type);
        case Qt::UserRole + 3:
            return QString::fromStdString(file.id);
        case Qt::UserRole + 4:
            return file.name;
        case Qt::UserRole + 5:
            return file.visibleName;
    }

    return {};
}

int FileModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_files.size();
}

QString FileModel::filePathFromName(const QString& fileName) const
{
    auto foundFile = std::find_if(m_files.begin(), m_files.end(), [fileName](const auto &actualFile){return actualFile.name == fileName;});
    if (foundFile != m_files.end())
    {
        return foundFile->path;
    }
    return {};
}

void FileModel::loadFiles()
{
    if (loading())
    {
        return;
    }

    setLoading(true);

    loadFilesImpl();
}

std::vector<FileModel::FileInfoContainer> FileModel::files()
{
    return m_files;
}

FileModel::LoadingFeedback FileModel::loadJsonFromFile(const QString& filename)
{
    const auto &loadResult = loadJsonFromFile(filename, m_fileType);
    if (const auto feedback = std::get<LoadingFeedback>(loadResult); feedback != LoadingFeedback::NoErrors || !std::get<std::optional<FigureVariant>>(loadResult))
    {
        return feedback;
    }
    const auto &variant = std::get<std::optional<FigureVariant>>(loadResult).value();
    switch (variant.index())
    {
        case std::size_t(FileType::Seam):
            m_seamFigure = std::get<std::size_t(FileType::Seam)>(variant);
            break;
        case std::size_t(FileType::Wobble):
            m_wobbleFigure = std::get<std::size_t(FileType::Wobble)>(variant);
            break;
        case std::size_t(FileType::Overlay):
            m_overlayFigure = std::get<std::size_t(FileType::Overlay)>(variant);
            break;
        case std::size_t(FileType::Basic):
            m_wobbleFigure = std::get<std::size_t(FileType::Wobble)>(variant);
            break;
        default:
            return LoadingFeedback::NameOrTypeError;
    }

    reset();

    m_filename = filename;
    emit filenameChanged();

    emit figureLoaded(m_fileType);
    return LoadingFeedback::NoErrors;
}

std::pair<FileModel::LoadingFeedback, std::optional<FileModel::FigureVariant>> FileModel::loadJsonFromFile(const QString& filename, FileType type) const
{
    if (filename.isEmpty())
    {
        return {LoadingFeedback::NameOrTypeError, {}};
    }
    return loadJsonFromFile(QFileInfo{filePathFromName(filename)}, type);
}

std::pair<FileModel::LoadingFeedback, std::optional<FileModel::FigureVariant>> FileModel::loadJsonFromFile(const QFileInfo& file, FileType type) const
{
    if (type == FileType::None)
    {
        return {LoadingFeedback::NameOrTypeError, {}};
    }
    if (!file.exists())
    {
        return {LoadingFeedback::FileNotFound, {}};
    }

    json jsonObject;

    if (!RTC6::readFromFile(jsonObject, file.absoluteFilePath().toStdString()))
    {
        return {LoadingFeedback::EmptyFile, {}};
    }

    try
    {
        switch (type)
        {
            case FileType::Seam:
            {
                RTC6::seamFigure::SeamFigure seam = jsonObject;
                return {LoadingFeedback::NoErrors, FigureVariant{seam}};
            }
            case FileType::Wobble:
            {
                RTC6::wobbleFigure::Figure wobble = jsonObject;
                return {LoadingFeedback::NoErrors, FigureVariant{wobble}};
            }
            case FileType::Overlay:
            {
                RTC6::function::OverlayFunction overlay = jsonObject;
                return {LoadingFeedback::NoErrors, FigureVariant{overlay}};
            }
            case FileType::Basic:
            {
                RTC6::wobbleFigure::Figure wobble = jsonObject;
                return {LoadingFeedback::NoErrors, FigureVariant{wobble}};
            }
            default:
                break;
        }
    }
    catch (...)
    {
        return {LoadingFeedback::CorruptedFile, {}};
    }
    return {LoadingFeedback::NameOrTypeError, {}};
}

void FileModel::reset()
{
    m_filename.clear();
    emit filenameChanged();
}

template<class T>
bool FileModel::saveFigureToFile(const T& figure, const QString& fullPath)
{
    json manipulatedJsonObject;
    try
    {
        manipulatedJsonObject = figure;
    }
    catch(...)
    {
        return false;
    }

    return RTC6::printToFile(manipulatedJsonObject, fullPath.toStdString());
}

template bool FileModel::saveFigureToFile<RTC6::seamFigure::SeamFigure>(const RTC6::seamFigure::SeamFigure& figure, const QString& fullPath);
template bool FileModel::saveFigureToFile<RTC6::wobbleFigure::Figure>(const RTC6::wobbleFigure::Figure& figure, const QString& fullPath);
template bool FileModel::saveFigureToFile<RTC6::function::OverlayFunction>(const RTC6::function::OverlayFunction& figure, const QString& fullPath);

template<typename T>
bool FileModel::saveFigureImpl(const T& figure, const QString& filename)
{
    if (saveFigureToFile(figure, filePathForSaveFigure(filename)))
    {
        // Immediately load enumerate the files to make sure the saved file is available for loading.
        loadFilesImpl(true);
        if (!filename.isEmpty())
        {
            m_filename = filename;
            emit filenameChanged();
        }

        //No file system watcher is used to save the current file. The timer is used to wait until the file system finished its operations.
        QTimer::singleShot(std::chrono::milliseconds{100}, this, &FileModel::loadFiles);
        return true;
    }

    return false;
}

bool FileModel::saveFigure(const RTC6::seamFigure::SeamFigure& figure, const QString& filename)
{
    return saveFigureImpl(figure, filename);
}

bool FileModel::saveFigure(const RTC6::wobbleFigure::Figure& figure, const QString& filename)
{
    return saveFigureImpl(figure, filename);
}

bool FileModel::saveFigure(const RTC6::function::OverlayFunction& figure, const QString& filename)
{
    return saveFigureImpl(figure, filename);
}

bool FileModel::fileExists()
{
    return QFile(filePathForSaveFigure({})).exists();
}

bool FileModel::deleteFigure()
{
    return QFile(filePathForSaveFigure({})).remove();
}

RTC6::function::OverlayFunction FileModel::loadOverlayFunction(const QString& filename) const
{
    return loadFigureImpl<RTC6::function::OverlayFunction>(filename, FileType::Overlay);
}

RTC6::seamFigure::SeamFigure FileModel::loadSeamFigure(const QString& filename) const
{
    return loadFigureImpl<RTC6::seamFigure::SeamFigure>(filename, FileType::Seam);
}

RTC6::wobbleFigure::Figure FileModel::loadWobbleFigure(const QString& filename) const
{
    return loadFigureImpl<RTC6::wobbleFigure::Figure>(filename, FileType::Wobble);
}

RTC6::wobbleFigure::Figure FileModel::loadBasicFigure(const QString& filename) const
{
    return loadFigureImpl<RTC6::wobbleFigure::Figure>(filename, FileType::Basic);
}

QString FileModel::filePathForSaveFigure(const QString& filename) const
{
    return filePathForSaveFigure(filename, m_fileType);
}

QString FileModel::filePathForSaveFigure(const QString& filename, FileType type) const
{
    switch (type)
    {
    case FileType::Seam:
        return filename.isEmpty() ? filePathFromName(m_filename) : m_seamAndOverlayFilePath + "/" + filename + ".json";
    case FileType::Wobble:
        return filename.isEmpty() ? filePathFromName(m_filename) : m_wobbleFilePath + "/" + filename + ".json";
    case FileType::Overlay:
        return filename.isEmpty() ? filePathFromName(m_filename) : m_seamAndOverlayFilePath + "/" + filename + ".json";
    case FileType::Basic:
        return filename.isEmpty() ? filePathFromName(m_filename) : m_wobbleFilePath + "/" + filename + ".json";
    default:
        Q_UNREACHABLE();
        return {};
    }
}

QString FileModel::exportInfixForFigureType(FileType fileType) const
{
    switch (fileType)
    {
    case FileType::Seam:
    case FileType::Overlay:
        return QStringLiteral("/weldmaster/figures/weld_figure");
    case FileType::Wobble:
        return QStringLiteral("/weldmaster/figures/laser_controls");
    case FileType::Basic:
        Q_ASSERT(!"basic type should not be exported");
        return QStringLiteral("/weldmaster/figures/basic_figure");
    default:
        Q_UNREACHABLE();
        return {};
    }
}

QString FileModel::typeToString(FileType type) const
{
    switch (type)
    {
        case FileType::Seam:
            return QStringLiteral("Seam figure");
        case FileType::Wobble:
            return QStringLiteral("Wobble figure");
        case FileType::Overlay:
            return QStringLiteral("Overlay function");
        case FileType::Basic:
            return QStringLiteral("Basic figure");
        case FileType::Dxf:
            return QStringLiteral("DXF file");
        default:
            return {};
    }
}

void FileModel::loadFilesImpl(bool wait)
{
    m_pendingWatcher = nullptr;

    const QDir seamAndOverlayDir{m_seamAndOverlayFilePath};
    const QDir wobbleFileDir{m_wobbleFilePath};
    std::optional<QDir> basicFileDir;
    std::optional<QDir> dxfFileDir;

    if (m_basicFilePath)
    {
        basicFileDir.emplace(*m_basicFilePath);
    }

    if (m_dxfFilePath)
    {
        dxfFileDir.emplace(*m_dxfFilePath);
    }

    auto loadAllFiles = [this](const QDir seamAndOverlayPath, const QDir wobbleFilePath, const std::optional<QDir> basicFileDir, const std::optional<QDir> dxfFileDir) -> std::vector<FileInfoContainer>
    {
        std::vector<FileInfoContainer> allFiles;
        FileInfoContainer fileInfo;

        if (seamAndOverlayPath.exists())
        {
            const auto files = seamAndOverlayPath.entryInfoList(QStringList{{QStringLiteral("weldingSeam*.json")}, {QStringLiteral("overlayFunction*.json")}}, QDir::Files);
            for (const auto &file : files)
            {
                if (file.fileName().contains(QStringLiteral("weldingSeam")))
                {
                    const auto &loadResult = loadJsonFromFile(file, FileType::Seam);
                    if (std::get<LoadingFeedback>(loadResult) == LoadingFeedback::NoErrors && std::get<std::optional<FigureVariant>>(loadResult))
                    {
                        const auto &variant = std::get<std::optional<FigureVariant>>(loadResult).value();
                        const auto &figure = std::get<RTC6::seamFigure::SeamFigure>(variant);
                        allFiles.emplace_back(FileInfoContainer{file.fileName(), file.filePath(), FileType::Seam, figure.ID, QString::fromStdString(figure.name)});
                    }
                }
                else
                {
                    const auto &loadResult = loadJsonFromFile(file, FileType::Overlay);
                    if (std::get<LoadingFeedback>(loadResult) == LoadingFeedback::NoErrors && std::get<std::optional<FigureVariant>>(loadResult))
                    {
                        const auto &variant = std::get<std::optional<FigureVariant>>(loadResult).value();
                        const auto &figure = std::get<RTC6::function::OverlayFunction>(variant);
                        allFiles.emplace_back(FileInfoContainer{file.fileName(), file.filePath(), FileType::Overlay, figure.ID, QString::fromStdString(figure.name)});
                    }
                }
            }
        }

        if (wobbleFilePath.exists())
        {
            const auto files = wobbleFilePath.entryInfoList(QStringList{{QStringLiteral("figureWobble*.json")}}, QDir::Files);
            for (const auto &file : files)
            {
                const auto &loadResult = loadJsonFromFile(file, FileType::Wobble);
                if (std::get<LoadingFeedback>(loadResult) == LoadingFeedback::NoErrors && std::get<std::optional<FigureVariant>>(loadResult))
                {
                    const auto &variant = std::get<std::optional<FigureVariant>>(loadResult).value();
                    const auto &figure = std::get<RTC6::wobbleFigure::Figure>(variant);
                    allFiles.emplace_back(FileInfoContainer{file.fileName(), file.filePath(), FileType::Wobble, figure.ID, QString::fromStdString(figure.name)});
                }
            }
        }

        if (basicFileDir && basicFileDir->exists())
        {
            const auto files = basicFileDir->entryInfoList(QStringList{{QStringLiteral("basic*.json")}}, QDir::Files);
            for (const auto& file : files)
            {
                const auto& loadResult = loadJsonFromFile(file, FileType::Basic);
                if (std::get<LoadingFeedback>(loadResult) == LoadingFeedback::NoErrors && std::get<std::optional<FigureVariant>>(loadResult))
                {
                    const auto &variant = std::get<std::optional<FigureVariant>>(loadResult).value();
                    const auto &figure = std::get<RTC6::wobbleFigure::Figure>(variant);
                    allFiles.emplace_back(FileInfoContainer{file.fileName(), file.filePath(), FileType::Basic, figure.ID, QString::fromStdString(figure.name)});
                }
            }
        }

        if (dxfFileDir && dxfFileDir->exists())
        {
            const auto files = dxfFileDir->entryInfoList(QStringList{{QStringLiteral("*.dxf"), QStringLiteral("*.DXF")}}, QDir::Files);
            for (const auto& file : files)
            {
                allFiles.emplace_back(FileInfoContainer{file.fileName(), file.filePath(), FileType::Dxf, {}, {}});
            }
        }

        return allFiles;
    };

    auto applyResult = [this](std::vector<FileInfoContainer> const& result)
    {
        m_pendingWatcher = nullptr;
        beginResetModel();
        m_files.clear();
        m_files = result;
        endResetModel();
        setLoading(false);
        emit emptyChanged();
    };

    if (wait)
    {
        applyResult(loadAllFiles(seamAndOverlayDir, wobbleFileDir, basicFileDir, dxfFileDir));
    }
    else
    {
        auto watcher = m_pendingWatcher = new QFutureWatcher<std::vector<FileInfoContainer>>(this);
        connect(watcher, &QFutureWatcher<std::vector<FileInfoContainer>>::finished, this,
                [applyResult, watcher, this]
                {
                    watcher->deleteLater();

                    // Discard results if a call to loadFilesImpl() has been made since the future was created.
                    // Otherwise we would potentially overwrite more recent results with the results of this future.
                    if (watcher != this->m_pendingWatcher)
                    {
                        return;
                    }

                    applyResult(watcher->result());
                });
        watcher->setFuture(QtConcurrent::run(loadAllFiles, seamAndOverlayDir, wobbleFileDir, basicFileDir, dxfFileDir));
    }
}

template <typename T>
T FileModel::loadFigureImpl(const QString &filename, FileType type) const
{
    const auto &loadResult = loadJsonFromFile(filename, type);
    if (const auto feedback = std::get<LoadingFeedback>(loadResult); feedback != LoadingFeedback::NoErrors || !std::get<std::optional<FigureVariant>>(loadResult))
    {
        return {};
    }
    const auto &variant = std::get<std::optional<FigureVariant>>(loadResult).value();
    return std::get<T>(variant);
}

QModelIndex FileModel::indexForSeamFigure(int id) const
{
    return indexFor(id, FileType::Seam);
}

QModelIndex FileModel::indexForWobbleFigure(int id) const
{
    return indexFor(id, FileType::Wobble);
}

QModelIndex FileModel::indexFor(int id, FileType fileType) const
{
    auto it = std::find_if(m_files.begin(), m_files.end(),
        [=] (const auto &file)
        {
            if (file.type != fileType)
            {
                return false;
            }
            try
            {
                int fileId = std::stoi(file.id);
                return fileId == id;
            } catch (...)
            {
                return false;
            }
        }
    );
    if (it == m_files.end())
    {
        return {};
    }
    return index(std::distance(m_files.begin(), it), 0);
}

ImportFileModelFactory* ImportFileModelFactory::instance()
{
    static ImportFileModelFactory inst;
    return &inst;
}

FileModel* ImportFileModelFactory::createImportFileModel(QString removableDevicesPath)
{
    return new FileModel(nullptr, &removableDevicesPath);
}

}
}
}
}

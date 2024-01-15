#pragma once

#include <QAbstractListModel>
#include <QFileInfo>
#include <QFutureWatcher>
#include <vector>
#include <optional>

#include "fileType.h"
#include "editorDataTypes.h"

class FileModelTest;
namespace precitec
{
namespace scantracker
{
namespace components
{
namespace wobbleFigureEditor
{

/**
 * Model which loads all seam, wobble and overlay files.
 **/
class FileModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool loading READ loading WRITE setLoading NOTIFY loadingChanged)

    Q_PROPERTY(precitec::scantracker::components::wobbleFigureEditor::FileType fileType READ fileType WRITE setFileType NOTIFY fileTypeChanged);
    Q_PROPERTY(QString filename READ filename NOTIFY filenameChanged)
    Q_PROPERTY(bool empty READ empty NOTIFY emptyChanged)

public:
    explicit FileModel(QObject* parent = nullptr, QString const* removableDevicesPath = nullptr);
    ~FileModel() override;

    enum class LoadingFeedback
    {
        NameOrTypeError = 0,
        FileNotFound,
        EmptyFile,
        CorruptedFile,
        NoErrors
    };
    Q_ENUM(LoadingFeedback)

    struct FileInfoContainer
    {
        QString name;
        QString path;
        FileType type;
        std::string id;
        QString visibleName;
    };

    bool loading() const
    {
        return m_loading;
    }
    void setLoading(bool isLoading);

    FileType fileType() const
    {
        return m_fileType;
    }
    void setFileType(FileType newFileType);

    QString filename() const
    {
        return m_filename;
    }

    bool empty() const
    {
        return m_files.empty();
    }

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    Q_INVOKABLE void loadFiles();

    std::vector<FileInfoContainer> files();

    Q_INVOKABLE LoadingFeedback loadJsonFromFile(const QString& filename);
    Q_INVOKABLE void reset();

    RTC6::seamFigure::SeamFigure seamFigure() const
    {
        return m_seamFigure;
    }
    void setSeamFigure(RTC6::seamFigure::SeamFigure figure)
    {
        m_seamFigure = figure;
        m_fileType = FileType::Seam;
        emit figureLoaded(FileType::Seam);
    }

    RTC6::wobbleFigure::Figure wobbleFigure() const
    {
        return m_wobbleFigure;
    }

    RTC6::function::OverlayFunction overlayFigure() const
    {
        return m_overlayFigure;
    }

    bool saveFigure(const RTC6::seamFigure::SeamFigure& figure, const QString &filename = {});
    bool saveFigure(const RTC6::wobbleFigure::Figure& figure, const QString &filename = {});
    bool saveFigure(const RTC6::function::OverlayFunction& figure, const QString &filename = {});

    bool fileExists();
    bool deleteFigure();

    RTC6::function::OverlayFunction loadOverlayFunction(const QString& filename) const;
    RTC6::seamFigure::SeamFigure loadSeamFigure(const QString& filename) const;
    RTC6::wobbleFigure::Figure loadWobbleFigure(const QString& filename) const;
    RTC6::wobbleFigure::Figure loadBasicFigure(const QString& filename) const;

    /**
     * @returns index to the figure with the given @p id and type @c FileType::Seam
     **/
    Q_INVOKABLE QModelIndex indexForSeamFigure(int id) const;

    /**
     * @returns index to the figure with the given @p id and type @c FileType::Wobble
     **/
    Q_INVOKABLE QModelIndex indexForWobbleFigure(int id) const;

    template<typename T>
    static bool saveFigureToFile(const T& figure, const QString& fullPath);

    /**
     * @returns <i>infix</i> that is needed to build the path for exporting/importing a figure of the given type: removableDevices::Service.path() + <i><b>infix</b></i> + "/" + <i>filenameAndExtension</i>
     * @note There is a slash at the start but not at the end of the infix.
     */
    Q_INVOKABLE QString exportInfixForFigureType(precitec::scantracker::components::wobbleFigureEditor::FileType fileType) const;

Q_SIGNALS:
    void loadingChanged();
    void fileTypeChanged();
    void filenameChanged();
    void figureLoaded(precitec::scantracker::components::wobbleFigureEditor::FileType type);
    void emptyChanged();

private:
    QModelIndex indexFor(int id, FileType fileType) const;
    QString filePathFromName(const QString& fileName) const;
    QString filePathForSaveFigure(const QString& filename) const;
    QString filePathForSaveFigure(const QString& filename, FileType fileType) const;
    template<typename T>
    bool saveFigureImpl(const T& figure, const QString& filename);
    QString typeToString(FileType type) const;
    void loadFilesImpl(bool wait = false);
    template <typename T>
    T loadFigureImpl(const QString &filename, FileType type) const;

    using FigureVariant = std::variant<RTC6::seamFigure::SeamFigure, RTC6::wobbleFigure::Figure, RTC6::function::OverlayFunction>;

    std::pair<LoadingFeedback, std::optional<FigureVariant>> loadJsonFromFile(const QString& filename, FileType type) const;
    std::pair<LoadingFeedback, std::optional<FigureVariant>> loadJsonFromFile(const QFileInfo& file, FileType type) const;

    std::vector<FileInfoContainer> m_files;

    QString m_seamAndOverlayFilePath;
    QString m_wobbleFilePath;
    std::optional<QString> m_basicFilePath;
    std::optional<QString> m_dxfFilePath;
    bool m_loading{false};

    FileType m_fileType{FileType::None};
    QString m_filename;

    RTC6::seamFigure::SeamFigure m_seamFigure;
    RTC6::wobbleFigure::Figure m_wobbleFigure;
    RTC6::function::OverlayFunction m_overlayFigure;

    QFutureWatcher<std::vector<FileInfoContainer>>* m_pendingWatcher = nullptr;

    friend FileModelTest;
};

class ImportFileModelFactory : public QObject
{
    Q_OBJECT
public:
    static ImportFileModelFactory* instance();
    Q_INVOKABLE precitec::scantracker::components::wobbleFigureEditor::FileModel* createImportFileModel(QString removableDevicesPath);
};
}
}
}
}

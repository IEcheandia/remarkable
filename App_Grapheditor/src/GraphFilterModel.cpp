#include "GraphFilterModel.h"

#include "fliplib/Fliplib.h"
#include "fliplib/BaseFilterInterface.h"
#include "fliplib/BaseFilter.h"

#include <QDir>
#include <QFile>
#include <QDebug>
#include <QDirIterator>

#include <Poco/ClassLoader.h>

typedef int (*version)();

using namespace precitec::gui::components::grapheditor;

GraphFilterModel::GraphFilterModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

GraphFilterModel::~GraphFilterModel() = default;

QString GraphFilterModel::filterImagePath() const
{
    return m_filterImagePath;
}

void GraphFilterModel::setFilterImagePath(const QString& path)
{
    if (m_filterImagePath != path)
    {
        m_filterImagePath = path;
        emit filterImagePathChanged();
    }
}

QVariant GraphFilterModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    const auto &filterInfo = m_filterInformationModel.at(index.row());

    switch (role)
    {
    case Qt::DisplayRole:
        return filterInfo.filterLibName;
    case Qt::UserRole:
        return filterInfo.libPath;
    case Qt::UserRole+1:
        return filterInfo.filterName;
    case Qt::UserRole+2:
        return filterInfo.filterID;
    case Qt::UserRole+3:
        return filterInfo.filterType;
    case Qt::UserRole+4:
        return filterInfo.filterImagePath;
    case Qt::UserRole+5:
        return !filterInfo.pdfFileName.isEmpty();
    case Qt::UserRole+6:
        return filterInfo.pdfFileName;
    }

    return {};
}

QHash<int, QByteArray> GraphFilterModel::roleNames() const
{
    return
    {
        {Qt::DisplayRole, QByteArrayLiteral("filterLibName")},
        {Qt::UserRole, QByteArrayLiteral("libPath")},
        {Qt::UserRole+1, QByteArrayLiteral("filterName")},
        {Qt::UserRole+2, QByteArrayLiteral("filterID")},
        {Qt::UserRole+3, QByteArrayLiteral("filterType")},
        {Qt::UserRole+4, QByteArrayLiteral("filterImagePath")},
        {Qt::UserRole+5, QByteArrayLiteral("pdfAvailable")},
        {Qt::UserRole+6, QByteArrayLiteral("pdfFileName")},
    };
}

int GraphFilterModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_filterInformationModel.size();
}

void GraphFilterModel::init(const QString& path)
{
    QDir dir(path);
    if (!dir.exists())
    {
        qDebug() << "Cannot find the example directory";
    }
    if (dir.count() <= 2)
    {
        qDebug() << "Directory contains no entries";
    }
    QFileInfoList files;
    files << dir.entryInfoList(QStringList() << QStringLiteral("libFilter_*.so"), QDir::Files);

    Poco::ClassLoader<fliplib::BaseFilterInterface> pluginLoader;
    for (const auto &file : files)
    {
        bool libVersionOk = false;
        auto sharedLibPath = file.absoluteFilePath().toStdString();
        Poco::SharedLibrary sharedLib;
        try
        {
            sharedLib.load(sharedLibPath);

            if (sharedLib.hasSymbol("version"))
            {
                auto symbolPtr = sharedLib.getSymbol("version");

                version funcValue = (version)symbolPtr;
                const auto versionNumber = funcValue();
                if (versionNumber == FLIPLIB_VERSION)
                {
                    libVersionOk = true;
                }
            }

            sharedLib.unload();
        }
        catch (Poco::LibraryLoadException& e)
        {
            qWarning() << "LibraryLoadException " << e.displayText().c_str();
            throw(e);
        }

        if (libVersionOk)
        {
            try
            {
                pluginLoader.loadLibrary(file.absoluteFilePath().toStdString());
                const auto &manifest = pluginLoader.manifestFor(file.absoluteFilePath().toStdString());
                beginResetModel();
                for (auto it = manifest.begin(); it != manifest.end(); ++it)
                {
                    FilterInfoContainer actualFilterInfo;
                    actualFilterInfo.libPath = file.absoluteFilePath();
                    actualFilterInfo.filterLibName = file.baseName();
                    actualFilterInfo.filterLibName.remove("libFilter_");
                    actualFilterInfo.filterLibName = QString{"Filter_"} + actualFilterInfo.filterLibName;
                    auto filterInterface = pluginLoader.create(it->name());
                    auto filter = static_cast<fliplib::BaseFilter*>(filterInterface);
                    actualFilterInfo.filterID = precitec::storage::compatibility::toQt(filter->filterID());
                    actualFilterInfo.filterImagePath = QUrl::fromLocalFile(m_filterImagePath + actualFilterInfo.filterID.toString(QUuid::WithoutBraces) + QStringLiteral(".png"));
                    actualFilterInfo.filterName = QString::fromStdString(filter->name());
                    actualFilterInfo.filterType = filter->getFilterType();
                    actualFilterInfo.inPipes = filter->inPipeConnectors();
                    actualFilterInfo.outPipes = filter->outPipeConnectors();
                    for (unsigned int i = 0; i < filter->variantIDSize(); i++)
                    {
                        actualFilterInfo.variantID.push_back(precitec::storage::compatibility::toQt(filter->variantID(i)));
                    }
                    actualFilterInfo.pdfFileName = pdfFileNameForFilterName(actualFilterInfo.filterName);
                    QDirIterator pdfIt(m_pdfFilesDir, QStringList() << actualFilterInfo.pdfFileName + QStringLiteral(".pdf"), QDir::Files, QDirIterator::Subdirectories);
                    if (!pdfIt.hasNext())
                    {
                        actualFilterInfo.pdfFileName = QString{};
                    }
                    m_filterInformationModel.push_back(actualFilterInfo);
                    pluginLoader.destroy(it->name(), filterInterface);
                }
                endResetModel();
                pluginLoader.unloadLibrary(file.absoluteFilePath().toStdString());
            }
            catch (Poco::LibraryLoadException &e)
            {
                qWarning() << "Exception in class loader " << e.displayText().c_str() << " " << e.message().c_str();
                throw(e);
            }
        }
    }
    emit ready();
}

QString GraphFilterModel::pdfFileNameForFilterName(const QString& filterName) const
{
    static const std::map<QString, QString> s_map{
        {QStringLiteral("Convolution3X3"), QStringLiteral("Filter_ConvolutionThreeXThree")},
        {QStringLiteral("SurfaceCalculator2Rows"), QStringLiteral("Filter_SurfaceCalculatorTwoRows")},
        {QStringLiteral("GetRunData3D"), QStringLiteral("Filter_GetRunDataThreeD")},
        {QStringLiteral("LineRandkerbe3Inputs"), QStringLiteral("Filter_RandkerbeThreeInputs")},
        {QStringLiteral("DataSubsampling2"), QStringLiteral("Filter_DataSubsamplingTwo")},
        {QStringLiteral("Intersect2Lines"), QStringLiteral("Filter_IntersectTwoLines")},
        {QStringLiteral("MaxJump2"), QStringLiteral("Filter_MaxJumpTwo")},
        {QStringLiteral("TemporalLowPass2"), QStringLiteral("Filter_TemporalLowPassTwo")},
    };
    if (auto it = s_map.find(filterName); it != s_map.end())
    {
        return it->second;
    }
    return QStringLiteral("Filter_") + filterName;
}

void GraphFilterModel::printInformationToTxt(const QString& txtName) const
{
    QString dirPath = QDir::homePath();
    dirPath = dirPath + "/" + txtName;
    QFile fileForInfos(dirPath);
    if (!fileForInfos.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "File cannot be opened!";
        return;
    }

    QTextStream out(&fileForInfos);
    out << "Number of filter:" << m_filterInformationModel.size() << "\n\n";

    int i = 1;
    for (auto const& filterInfo : m_filterInformationModel)
    {
        out << "Number: " << i << "\n";
        out << "filterLibName:  " << filterInfo.filterLibName << "\n";
        out << "Path:        " << filterInfo.libPath << "\n";
        out << "Name:        " << filterInfo.filterName << "\n";
        out << "FilterID:    " << filterInfo.filterID.toString() << "\n";
        out << "FilterType: " << filterInfo.filterType << "\n\n";
        i++;
    }
}

std::vector<GraphFilterModel::FilterInfoContainer> GraphFilterModel::getFilterInformationModel() const
{
    return m_filterInformationModel;
}

void GraphFilterModel::setPdfFilesDir(const QString& pdfFilesDir)
{
    if (m_pdfFilesDir.compare(pdfFilesDir) == 0)
    {
        return;
    }
    m_pdfFilesDir = pdfFilesDir;
    emit pdfFilesDirChanged();
}

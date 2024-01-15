#pragma once

#include "../App_Storage/src/compatibility.h"                                                               // Needed because of QUuid in strct FilterInfoContainer

#include "fliplib/PipeConnector.h"

#include <vector>

#include <QString>
#include <QUrl>
#include <QAbstractListModel>

namespace precitec
{

namespace gui
{

namespace components
{

namespace grapheditor
{

class GraphFilterModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString pdfFilesDir READ pdfFilesDir WRITE setPdfFilesDir NOTIFY pdfFilesDirChanged)

public:
    explicit GraphFilterModel(QObject *parent = nullptr);
    ~GraphFilterModel() override;

    Q_PROPERTY(QString filterImagePath READ filterImagePath WRITE setFilterImagePath NOTIFY filterImagePathChanged)

    QString filterImagePath() const;
    void setFilterImagePath(const QString &path);

    struct FilterInfoContainer
    {
        QString filterLibName;
        QString libPath;
        QString filterName;
        QString pdfFileName;
        QUuid filterID;
        QUrl filterImagePath;
        int filterType;
        std::vector<fliplib::PipeConnector> inPipes;                //Pipes which go in the filter (left side of the filter)
        std::vector<fliplib::PipeConnector> outPipes;               //Pipes which go out the filter (right side of the filter)
        std::vector<QUuid> variantID;
    };

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    Q_INVOKABLE void init(const QString &path);
    Q_INVOKABLE void printInformationToTxt(const QString &txtName = QStringLiteral("FilterInfos.txt")) const;
    std::vector<FilterInfoContainer> getFilterInformationModel() const;

    QString pdfFilesDir() const
    {
        return m_pdfFilesDir;
    }
    void setPdfFilesDir(const QString& pdfFilesDir);

Q_SIGNALS:
    void ready();
    void filterImagePathChanged();
    void pdfFilesDirChanged();

private:
    QString pdfFileNameForFilterName(const QString& filterName) const;
    std::vector<FilterInfoContainer> m_filterInformationModel;
    QString m_filterImagePath;
    QString m_pdfFilesDir;

};

}
}
}
}
Q_DECLARE_METATYPE(precitec::gui::components::grapheditor::GraphFilterModel*)

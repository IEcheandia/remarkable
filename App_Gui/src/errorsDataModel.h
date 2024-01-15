#pragma once

#include <QAbstractListModel>
#include <QColor>

#include <vector>

class QColor;

namespace precitec
{
namespace gui
{

struct ErrorData {
    ErrorData (QString name, QColor color, qreal position, int visualSeamNumber)
        : m_name(name)
        , m_color(color)
        , m_position(position)
        , m_visualSeamNumber(visualSeamNumber)
    {
    }
    QString m_name;
    QColor m_color;
    qreal m_position = 0.0;
    int m_visualSeamNumber = 0;
};

class AbstractPlotterDataModel;

class ErrorsDataModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(precitec::gui::AbstractPlotterDataModel* plotterModel READ plotterModel WRITE setPlotterModel NOTIFY plotterModelChanged)

public:
    explicit ErrorsDataModel(QObject* parent = nullptr);
    ~ErrorsDataModel();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    AbstractPlotterDataModel* plotterModel() const
    {
        return m_plotterModel;
    }
    void setPlotterModel(AbstractPlotterDataModel* model);

Q_SIGNALS:
    void plotterModelChanged();

private:
    void update();

    std::vector<ErrorData> m_errors;
    AbstractPlotterDataModel* m_plotterModel = nullptr;
    QMetaObject::Connection m_destroyedConnection;
};

}
}



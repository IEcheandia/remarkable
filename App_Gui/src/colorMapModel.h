#pragma once

#include <QAbstractListModel>

namespace precitec
{
namespace storage
{

class Product;

}
namespace gui
{
namespace components
{
namespace plotter
{

class MulticolorSet;
class ColorMap;

}
}

class ColorMapModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(precitec::storage::Product* currentProduct READ currentProduct WRITE setCurrentProduct NOTIFY currentProductChanged)

    Q_PROPERTY(precitec::gui::components::plotter::MulticolorSet* example READ example CONSTANT)

    Q_PROPERTY(bool enabled READ enabled NOTIFY currentProductChanged)

    Q_PROPERTY(bool binary READ binary WRITE setBinary NOTIFY binaryChanged)

    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)

public:
    explicit ColorMapModel(QObject *parent = nullptr);
    ~ColorMapModel() override;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    precitec::storage::Product* currentProduct() const
    {
        return m_currentProduct;
    }
    void setCurrentProduct(precitec::storage::Product* product);

    precitec::gui::components::plotter::MulticolorSet* example() const
    {
        return m_example;
    }

    bool enabled() const
    {
        return m_currentProduct;
    }

    bool binary() const
    {
        return m_binary;
    }
    void setBinary(bool binary);

    Q_INVOKABLE void addColor(float value, const QColor &color);
    Q_INVOKABLE void updateValue(quint32 idx, float value);
    Q_INVOKABLE void updateColor(quint32 idx, const QColor &color);
    Q_INVOKABLE void removeColor(quint32 idx);
    Q_INVOKABLE void reset();

Q_SIGNALS:
    void currentProductChanged();
    void binaryChanged();
    void rowCountChanged();
    void markAsChanged();

private:
    void prepareExample();
    bool m_binary = false;
    precitec::storage::Product* m_currentProduct = nullptr;
    precitec::gui::components::plotter::ColorMap* m_colorMap = nullptr;
    QMetaObject::Connection m_productDestroyed;
    precitec::gui::components::plotter::MulticolorSet* m_example;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::ColorMapModel*)


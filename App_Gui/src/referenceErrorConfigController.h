#pragma once

#include <QObject>

namespace precitec
{
namespace storage
{

class Product;
class SeamError;

}
namespace gui
{
namespace components
{
namespace plotter
{

class DataSet;

}
}

class ReferenceErrorConfigController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(precitec::storage::Product* currentProduct READ currentProduct WRITE setCurrentProduct NOTIFY currentProductChanged)

    Q_PROPERTY(precitec::storage::SeamError *seamError READ seamError WRITE setSeamError NOTIFY seamErrorChanged)

    Q_PROPERTY(precitec::gui::components::plotter::DataSet* visualReference READ visualReference NOTIFY visualReferenceChanged)

    Q_PROPERTY(precitec::gui::components::plotter::DataSet* lowerShadow READ lowerShadow CONSTANT)

    Q_PROPERTY(precitec::gui::components::plotter::DataSet* upperShadow READ upperShadow CONSTANT)

    Q_PROPERTY(precitec::gui::components::plotter::DataSet* lowerReference READ lowerReference CONSTANT)

    Q_PROPERTY(precitec::gui::components::plotter::DataSet* upperReference READ upperReference CONSTANT)

    Q_PROPERTY(precitec::gui::components::plotter::DataSet* middleReference READ middleReference CONSTANT)

    Q_PROPERTY(int offset READ offset WRITE setOffset NOTIFY offsetChanged)

public:
    explicit ReferenceErrorConfigController(QObject *parent = nullptr);
    ~ReferenceErrorConfigController() override;

    precitec::storage::Product* currentProduct() const
    {
        return m_currentProduct;
    }
    void setCurrentProduct(precitec::storage::Product* product);

    precitec::storage::SeamError *seamError() const
    {
        return m_seamError;
    }
    void setSeamError(precitec::storage::SeamError *seamError);

    precitec::gui::components::plotter::DataSet* visualReference() const
    {
        return m_visualReference;
    }
    Q_INVOKABLE void setVisualReference(precitec::gui::components::plotter::DataSet *ds);

    precitec::gui::components::plotter::DataSet* lowerShadow() const
    {
        return m_lowerShadow;
    }

    precitec::gui::components::plotter::DataSet* upperShadow() const
    {
        return m_upperShadow;
    }

    precitec::gui::components::plotter::DataSet* lowerReference() const
    {
        return m_lowerReference;
    }

    precitec::gui::components::plotter::DataSet* middleReference() const
    {
        return m_middleReference;
    }

    precitec::gui::components::plotter::DataSet* upperReference() const
    {
        return m_upperReference;
    }

    int offset() const
    {
        return m_offset;
    }
    void setOffset(int offset);

Q_SIGNALS:
    void currentProductChanged();
    void seamErrorChanged();
    void visualReferenceChanged();
    void referenceChanged();
    void offsetChanged();

private:
    void updateLowerBoundary();
    void updateUpperBoundary();
    void loadReferenceCurve();
    void updateReferenceCurve();

    int m_offset = 0;

    precitec::gui::components::plotter::DataSet* m_visualReference;
    precitec::gui::components::plotter::DataSet* m_lowerShadow;
    precitec::gui::components::plotter::DataSet* m_upperShadow;
    precitec::gui::components::plotter::DataSet* m_lowerReference;
    precitec::gui::components::plotter::DataSet* m_middleReference;
    precitec::gui::components::plotter::DataSet* m_upperReference;

    precitec::storage::Product* m_currentProduct = nullptr;
    QMetaObject::Connection m_productDestroyConnection;

    precitec::storage::SeamError* m_seamError = nullptr;
    QMetaObject::Connection m_destroyedConnection;
    QMetaObject::Connection m_resultValueChangedConnection;
};

}
}
Q_DECLARE_METATYPE(precitec::gui::ReferenceErrorConfigController*)




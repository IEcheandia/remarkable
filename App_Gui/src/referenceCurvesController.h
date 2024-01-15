#pragma once

#include <QObject>

namespace precitec
{
namespace storage
{

class Product;
class ReferenceCurve;

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

class ReferenceCurvesController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(precitec::storage::Product* currentProduct READ currentProduct WRITE setCurrentProduct NOTIFY currentProductChanged)

    Q_PROPERTY(precitec::storage::ReferenceCurve* referenceCurve READ referenceCurve WRITE setReferenceCurve NOTIFY referenceCurveChanged)

    Q_PROPERTY(precitec::gui::components::plotter::DataSet* lower READ lower CONSTANT)

    Q_PROPERTY(precitec::gui::components::plotter::DataSet* middle READ middle CONSTANT)

    Q_PROPERTY(precitec::gui::components::plotter::DataSet* upper READ upper CONSTANT)

public:
    explicit ReferenceCurvesController(QObject* parent = nullptr);
    ~ReferenceCurvesController() override;

    precitec::storage::Product* currentProduct() const
    {
        return m_currentProduct;
    }
    void setCurrentProduct(precitec::storage::Product* product);

    precitec::storage::ReferenceCurve* referenceCurve() const
    {
        return m_referenceCurve;
    }
    void setReferenceCurve(precitec::storage::ReferenceCurve* referenceCurve);

    precitec::gui::components::plotter::DataSet* lower() const
    {
        return m_lower;
    }

    precitec::gui::components::plotter::DataSet* middle() const
    {
        return m_middle;
    }

    precitec::gui::components::plotter::DataSet* upper() const
    {
        return m_upper;
    }

Q_SIGNALS:
    void currentProductChanged();
    void referenceCurveChanged();

private:
    void setSamples();

    precitec::storage::Product* m_currentProduct = nullptr;
    QMetaObject::Connection m_productDestroyConnection;

    precitec::storage::ReferenceCurve* m_referenceCurve = nullptr;
    QMetaObject::Connection m_curveDestroyConnection;

    precitec::gui::components::plotter::DataSet* m_lower;
    precitec::gui::components::plotter::DataSet* m_middle;
    precitec::gui::components::plotter::DataSet* m_upper;
};

}
}
Q_DECLARE_METATYPE(precitec::gui::ReferenceCurvesController*)

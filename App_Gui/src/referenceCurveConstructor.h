#pragma once

#include "instanceResultModel.h"
#include "referenceCurve.h"

class ReferenceCurveConstructorTest;

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

class DataSet;

}
}


/**
 * @brief Constructs a @link{ReferenceCurve}, based on the underlying instance data and the set jitter and algorithm type
 *
 * The base @link{InstanceResultModel} class provides information on the result instances of a certain product and result type.
 * This class uses this information to  construct a @link{ReferenceCurve}, based on the set @link{jitter} and the selected algorithm @link{referenceType}.
 * Each instance result is loaded, its samples processed and is freed from memory as soon as it is no longer required.
 **/

class ReferenceCurveConstructor : public InstanceResultModel
{
    Q_OBJECT

    /**
     * Product, for which the reference curve is constructed.
     * Holds the reference curve sample data
     **/
    Q_PROPERTY(precitec::storage::Product* currentProduct READ currentProduct WRITE setCurrentProduct NOTIFY currentProductChanged)

    /**
     * Reference curve instance, for which the samples are to be computed.
     **/
    Q_PROPERTY(precitec::storage::ReferenceCurve* referenceCurve READ referenceCurve WRITE setReferenceCurve NOTIFY referenceCurveChanged)

    /**
     * Construction algorithm type
     * Available types are @link{ReferenceType::Average}, @link{ReferenceType::Median} and @link{ReferenceType::MinMax}
     **/
    Q_PROPERTY(precitec::storage::ReferenceCurve::ReferenceType referenceType READ referenceType WRITE setReferenceType NOTIFY referenceTypeChanged)

    /**
     * Jitter value.
     * Defines a horizontal range, in which the algorithm compares the different samples to determine the
     * @link{upper} and @link{lower} boundaries of the @link{ReferenceCurve}.
     **/
    Q_PROPERTY(float jitter READ jitter WRITE setJitter NOTIFY jitterChanged)

    /**
     * Lower boundary of the @link{ReferenceCurve}
     **/
    Q_PROPERTY(precitec::gui::components::plotter::DataSet* lower READ lower CONSTANT)

    /**
     * The middle values of the @link{ReferenceCurve}
     **/
    Q_PROPERTY(precitec::gui::components::plotter::DataSet* middle READ middle CONSTANT)

    /**
     * Upper boundary of the @link{ReferenceCurve}
     **/
    Q_PROPERTY(precitec::gui::components::plotter::DataSet* upper READ upper CONSTANT)

    /**
     * Is the curve currently being constructed
     **/
    Q_PROPERTY(bool updating READ updating NOTIFY updatingChanged)

    /**
     * Has the curve been changed through the editor
     **/
    Q_PROPERTY(bool hasChanges READ hasChanges NOTIFY hasChanged)

public:
    explicit ReferenceCurveConstructor(QObject* parent = nullptr);
    ~ReferenceCurveConstructor() override;

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

    precitec::storage::ReferenceCurve::ReferenceType referenceType() const
    {
        return m_referenceType;
    }
    void setReferenceType(precitec::storage::ReferenceCurve::ReferenceType referenceType);

    float jitter() const
    {
        return m_jitter;
    }
    void setJitter(float jitter);

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

    bool updating() const
    {
        return m_updating;
    }

    bool hasChanges() const
    {
        return m_hasChanges;
    }
    void markAsChanged();
    void resetChanges();

    Q_INVOKABLE void save();

Q_SIGNALS:
    void currentProductChanged();
    void referenceCurveChanged();
    void referenceTypeChanged();
    void jitterChanged();
    void updatingChanged();
    void hasChanged();
    void progressChanged(float progress);

private:
    void refreshUpdating();
    void computeCurves();
    void changeCurves();
    void updateReferences();

    struct Instance {
        QString path;
        quint32 seamNumber = -1;
    };

    // construction functions to be executed in a separate thread
    void middleCurve(std::vector<Instance> instances, precitec::storage::ReferenceCurve::ReferenceType referenceType);
    void computeAverage(const std::vector<float>& x_values, std::vector<std::vector<float>>& y_values, std::vector<QVector2D>& output);
    void computeMedian(const std::vector<float>& x_values, std::vector<std::vector<float>>& y_values, std::vector<QVector2D>& output);
    void upperBound();
    void lowerBound();
    void minMaxCurve(std::vector<Instance> instances);

    bool m_updating = false;
    bool m_updatePending = false;
    bool m_upperUpdating = false;
    bool m_middleUpdating = false;
    bool m_lowerUpdating = false;
    bool m_hasChanges = false;

    float m_jitter = 0.0f;
    precitec::storage::ReferenceCurve::ReferenceType m_referenceType = precitec::storage::ReferenceCurve::ReferenceType::Average;

    precitec::storage::Product* m_currentProduct = nullptr;
    QMetaObject::Connection m_productDestroyConnection;

    precitec::storage::ReferenceCurve* m_referenceCurve = nullptr;
    QMetaObject::Connection m_curveDestroyConnection;

    precitec::gui::components::plotter::DataSet* m_lower;
    precitec::gui::components::plotter::DataSet* m_middle;
    precitec::gui::components::plotter::DataSet* m_upper;

    friend ReferenceCurveConstructorTest;
};

}
}
Q_DECLARE_METATYPE(precitec::gui::ReferenceCurveConstructor*)




#pragma once

#include <QObject>

class StaticErrorConfigControllerTest;

namespace precitec
{
namespace storage
{

class SeamError;

}
namespace gui
{
namespace components
{
namespace plotter
{

class DataSet;
class InfiniteSet;

}
}

class StaticErrorConfigController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(precitec::storage::SeamError *seamError READ seamError WRITE setSeamError NOTIFY seamErrorChanged)

    Q_PROPERTY(precitec::gui::components::plotter::DataSet* visualReference READ visualReference NOTIFY visualReferenceChanged)

    Q_PROPERTY(precitec::gui::components::plotter::InfiniteSet* lowerBoundary READ lowerBoundary CONSTANT)

    Q_PROPERTY(precitec::gui::components::plotter::InfiniteSet* upperBoundary READ upperBoundary CONSTANT)

    Q_PROPERTY(precitec::gui::components::plotter::InfiniteSet* shiftedLowerBoundary READ shiftedLowerBoundary CONSTANT)

    Q_PROPERTY(precitec::gui::components::plotter::InfiniteSet* shiftedUpperBoundary READ shiftedUpperBoundary CONSTANT)

public:
    explicit StaticErrorConfigController(QObject *parent = nullptr);
    ~StaticErrorConfigController() override;

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

    precitec::gui::components::plotter::InfiniteSet* lowerBoundary() const
    {
        return m_lowerBoundary;
    }

    precitec::gui::components::plotter::InfiniteSet* upperBoundary() const
    {
        return m_upperBoundary;
    }

    precitec::gui::components::plotter::InfiniteSet* shiftedLowerBoundary() const
    {
        return m_shiftedLowerBoundary;
    }

    precitec::gui::components::plotter::InfiniteSet* shiftedUpperBoundary() const
    {
        return m_shiftedUpperBoundary;
    }

    Q_INVOKABLE void setMinFromReference();
    Q_INVOKABLE void setMaxFromReference();

Q_SIGNALS:
    void seamErrorChanged();
    void visualReferenceChanged();

private:
    void updateLowerBoundary();
    void updateUpperBoundary();

    precitec::gui::components::plotter::DataSet *m_visualReference = nullptr;
    precitec::gui::components::plotter::InfiniteSet* m_lowerBoundary = nullptr;
    precitec::gui::components::plotter::InfiniteSet* m_upperBoundary = nullptr;
    precitec::gui::components::plotter::InfiniteSet* m_shiftedLowerBoundary = nullptr;
    precitec::gui::components::plotter::InfiniteSet* m_shiftedUpperBoundary = nullptr;
    precitec::storage::SeamError* m_seamError = nullptr;
    QMetaObject::Connection m_destroyedConnection;
    QMetaObject::Connection m_resultValueChangedConnection;

    friend StaticErrorConfigControllerTest;
};

}
}
Q_DECLARE_METATYPE(precitec::gui::StaticErrorConfigController*)



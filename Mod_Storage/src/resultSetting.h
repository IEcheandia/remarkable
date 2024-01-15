#pragma once

#include <QObject>
#include <QUuid>

namespace precitec
{
namespace storage
{

class ResultSettingModel;

/**
 * Represent the configuration class for a result
 * Used mainly for the plotter values
 */
class ResultSetting : public QObject
{
    Q_OBJECT
    /**
     * The unique identifier for this Result
     **/
    Q_PROPERTY(QUuid uuid READ uuid CONSTANT)
    /**
     * The identifier of this Result
     **/
    Q_PROPERTY(int enumType READ enumType CONSTANT)
    /**
     * The descriptive name of this Result.
     **/
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    /**
     * The Plotter number of this Result
     **/
    Q_PROPERTY(int plotterNumber READ plotterNumber WRITE setPlotterNumber NOTIFY plotterNumberChanged)
    /**
     * Show in Plotter
     **/
    Q_PROPERTY(int plottable READ plottable WRITE setPlottable NOTIFY plottableChanged)
    /**
     * The min value (plotter boundry) of this Result
     **/
    Q_PROPERTY(double min READ min WRITE setMin NOTIFY minChanged)
    /**
     * The max value (plotter boundry) of this Result
     **/
    Q_PROPERTY(double max READ max WRITE setMax NOTIFY maxChanged)
    /**
     * The color (plotter) of this Result
     **/
    Q_PROPERTY(QString lineColor READ lineColor WRITE setLineColor NOTIFY lineColorChanged)
    /**
     * Show this Result on main
     **/
    Q_PROPERTY(int visibleItem READ visibleItem WRITE setVisibleItem NOTIFY visibleItemChanged)
    /**
     * The visualization to be used to plot the data
     * Default is @c Visualization::Plot2D
     **/
    Q_PROPERTY(precitec::storage::ResultSetting::Visualization visualization READ visualization WRITE setVisualization NOTIFY visualizationChanged)
    /**
     * The disabled value (eg enable/disable the deleteButton) of this Result
     **/
    Q_PROPERTY(int disabled READ disabled WRITE setDisabled NOTIFY disabledChanged)

public:
    /**
     * Enum describing the options for the Visualization
     **/
    enum class Visualization
    {
        /**
         * All values are binary, so either 0 or 1
         **/
        Binary,
        /**
         * The values can be plotted by an x/y axis plot
         **/
        Plot2D,
        /**
         * The values represent a 3D height field
         **/
        Plot3D
    };
    Q_ENUM(Visualization)

    enum class Type {
        Uuid,
        EnumType,
        Name,
        PlotterNumber,
        Plottable,
        Min,
        Max,
        LineColor,
        Visible,
        Visualization,
        Disabled
    };
    Q_ENUM(Type)

    /**
     * Constructs the Parameter with the given @p uuid and the given @p parent
     **/
    ResultSetting(QObject *parent = nullptr) : QObject(parent) {}
    explicit ResultSetting(const QUuid &uuid, const int enumType, QObject *parent = nullptr);
    ~ResultSetting() override;

    QUuid uuid() const
    {
        return m_uuid;
    }
    int enumType() const
    {
        return m_enumType;
    }
    QString name() const
    {
        return m_name;
    }
    int plotterNumber() const
    {
        return m_plotterNumber;
    }
    int plottable() const
    {
        return m_plottable;
    }
    double min() const
    {
        return m_min;
    }
    double max() const
    {
        return m_max;
    }
    QString lineColor() const
    {
        return m_lineColor;
    }

    int visibleItem() const
    {
        return m_visibleItem;
    }

    Visualization visualization() const
    {
        return m_visualization;
    }
    int disabled() const
    {
        return m_disabled;
    }

    void setName(const QString &name);
    void setPlotterNumber(int plotterNumber);
    void setPlottable(int plottable);
    void setMin(double min);
    void setMax(double max);
    void setLineColor(const QString &lineColor);
    void setVisibleItem(int visible);
    void setVisualization(Visualization visualization);
    void setDisabled(int disabled);

    void updateValue(const QVariant &data, ResultSetting::Type target);

    QJsonObject toJson() const;

    static ResultSetting *fromJson(const QJsonObject &object, QObject *parent);

    enum class StorageType
    {
        System,
        UserOverlay
    };
    void setStorageType(StorageType type)
    {
        m_storageType = type;
    }
    StorageType storageType() const
    {
        return m_storageType;
    }

Q_SIGNALS:
    void typeIdChanged();
    void nameChanged();
    void plotterNumberChanged();
    void plottableChanged();
    void minChanged();
    void maxChanged();
    void lineColorChanged();
    void visibleItemChanged();
    void visualizationChanged();
    void disabledChanged();

private:
    QUuid m_uuid;
    int m_enumType;
    QString m_name;
    int m_plotterNumber;
    int m_plottable;
    double m_min;
    double m_max;
    int m_visibleItem;
    int m_disabled;
    QString m_lineColor;
    Visualization m_visualization = Visualization::Plot2D;
    StorageType m_storageType = StorageType::System;
};

}
}
Q_DECLARE_METATYPE(precitec::storage::ResultSetting*)

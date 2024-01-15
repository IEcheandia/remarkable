#pragma once

#include <QAbstractListModel>
#include "hardwareParameters.h"

#include <functional>

namespace precitec
{

namespace storage
{
class Attribute;
class AttributeModel;
class Parameter;
class ParameterSet;
class Seam;
}

namespace gui
{

/**
 * The HardwareParameterModel provides a few dedicated hardware parameters and their
 * values in the hardware ParameterSet of a Seam.
 *
 * The model provides the following roles:
 * @li display: user visible name of the hardware parameter
 * @li key: the HardwareParameterModel::Key identifying the parameter
 * @li enabled: whether the hardware Parameter is available in the hardware ParameterSet
 * @li attribute: the Attribute for this hardware Parameter
 * @li parameter: the Parameter in the hardware ParameterSet, is @c null if enabled is @c false
 * @li milliFromMicro: the value is in um and should be presented in mm
 **/
class AbstractHardwareParameterModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * The AttribueModel providing the Attribute description required when creating a missing hardware Parameter.
     **/
    Q_PROPERTY(precitec::storage::AttributeModel *attributeModel READ attributeModel WRITE setAttributeModel NOTIFY attributeModelChanged)
public:
    explicit AbstractHardwareParameterModel(QObject *parent = nullptr);
    ~AbstractHardwareParameterModel();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex{}) const override;
    QHash<int, QByteArray> roleNames() const override;

    precitec::storage::AttributeModel *attributeModel() const
    {
        return m_attributeModel;
    }
    virtual void setAttributeModel(precitec::storage::AttributeModel *model);

    /**
     * Gets the ParameterSet for the Seam. If there is none it will be created.
     * Assumes there is a seam.
     **/
    virtual precitec::storage::ParameterSet *getParameterSet() const = 0;

    /**
     * Enables the hardware Parameter identified by @p key.
     * If @p set is @c true, a Parameter is created in the ParameterSet, if @p set is @c false an existing Parameter
     * gets removed and destroyed.
     **/
    Q_INVOKABLE void setEnable(precitec::gui::HardwareParameters::Key key, bool set);

    /**
     * Updates the parameter with @p key to the @p value.
     **/
    Q_INVOKABLE void updateHardwareParameter(precitec::gui::HardwareParameters::Key key, const QVariant &value);

    void updateLedSendData(precitec::storage::ParameterSet* parameterSet);

    void updateGenerateScanTracker2DFigure(precitec::storage::ParameterSet* parameterSet);

    QModelIndex indexForKey(precitec::gui::HardwareParameters::Key key) const;

    /**
     * @returns The translated name for @p attribute if known by this model, otherwise the name of the attribute.
     **/
    static QString nameForAttribute(precitec::storage::Attribute *attribute);

Q_SIGNALS:
    void attributeModelChanged();
    /**
     * Emitted whenever a Parameter changed a value or got enabled/disabled
     **/
    void parameterChanged();
    void markAsChanged();

protected:
    precitec::storage::Parameter *findParameter(HardwareParameters::Key key) const;
    precitec::storage::Parameter *findParameter(precitec::storage::ParameterSet *parameterSet, HardwareParameters::Key key) const;
    precitec::storage::Attribute *findAttribute(HardwareParameters::Key key) const;

    virtual bool isValid() const = 0;
    /**
     * Like getParameterSet, but does not create
     **/
    virtual precitec::storage::ParameterSet *getParameterSetDirect() const = 0;

private:
    void updateLedSendData();
    void updateGenerateScanTracker2DFigure();
    void updateSendParameter(precitec::storage::ParameterSet* parameterSet, const QLatin1String& keyName, const QUuid& attributeUuid, std::function<bool(HardwareParameters*, const QUuid&)> isKey);
    precitec::storage::AttributeModel *m_attributeModel = nullptr;
    QMetaObject::Connection m_attributeModelReset;
    QMetaObject::Connection m_attributeModelDestroyed;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::AbstractHardwareParameterModel*)

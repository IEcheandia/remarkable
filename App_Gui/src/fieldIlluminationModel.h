#pragma once
#include "abstractDeviceKeyModel.h"
#include "message/device.interface.h"

namespace precitec
{
namespace gui
{

class FieldIlluminationModel : public AbstractDeviceKeyModel
{
    Q_OBJECT
public:
    explicit FieldIlluminationModel(QObject *parent = nullptr);
    ~FieldIlluminationModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    /**
     * Enables the panel of the field illumination at @p index.
     * @param enable Whether the line laser should be enabled or disabled
     **/
    Q_INVOKABLE void enable(int index, bool enable);
    /**
     * Sets the @p intensity of the panel at @p index.
     **/
    Q_INVOKABLE void setIntensity(int index, int intensity);

    /**
     * Sets the @p pulse width of the panel at @p index.
     **/
    Q_INVOKABLE void setPulseWidth(int index, int intensity);

private:
    void init();
    void keyValueChanged(const QUuid &id, const precitec::interface::SmpKeyValue &kv);
    struct Data {
        precitec::interface::SmpKeyValue toggle;
        precitec::interface::SmpKeyValue intensity;
        precitec::interface::SmpKeyValue pulseWidth;
        QString name;
    };
    std::vector<Data> m_data;
    QMetaObject::Connection m_notificationConnection; 
    
};    
    
}
}

Q_DECLARE_METATYPE(precitec::gui::FieldIlluminationModel*)


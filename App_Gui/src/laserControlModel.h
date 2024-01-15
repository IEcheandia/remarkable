#pragma once

#include "abstractLaserControlModel.h"

class QUuid;
class LaserControlModelTest;

namespace precitec
{

namespace gui
{

class ProductController;

class LaserControlModel : public AbstractLaserControlModel
{
    Q_OBJECT

    /**
     * Weather a preset is being edited at the moment
     **/
    Q_PROPERTY(bool isEditing READ isEditing NOTIFY editingChanged)

    /**
     * The Product Controller required to update the Seam LC Hardware Parameters and save the corresponding Products
     **/
    Q_PROPERTY(precitec::gui::ProductController *productController READ productController WRITE setProductController NOTIFY productControllerChanged)

public:
    explicit LaserControlModel(QObject *parent = nullptr);
    ~LaserControlModel() override;

    bool isEditing() const
    {
        return m_isEditing;
    }
    void setEditing(bool editing);

    ProductController *productController() const
    {
        return m_productController;
    }
    void setProductController(ProductController *controller);

    Q_INVOKABLE precitec::storage::LaserControlPreset* createNewPreset();
    Q_INVOKABLE void deletePreset(precitec::storage::LaserControlPreset* preset);
    Q_INVOKABLE void updateHardwareParameters(const QUuid &presetId);

Q_SIGNALS:
    void editingChanged();
    void productControllerChanged();

protected:
    void load() override;

private:
    void enablePresets();
    void disablePresets();
    bool findPreset(const QUuid &uuid);
    int findPresetIndex(const QUuid &uuid);

    ProductController *m_productController = nullptr;
    QMetaObject::Connection m_productControllerDestroyed;

    bool m_isEditing = false;

    friend LaserControlModelTest;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::LaserControlModel*)

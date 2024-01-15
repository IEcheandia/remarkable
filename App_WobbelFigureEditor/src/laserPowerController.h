#pragma once

#include <QObject>

namespace precitec
{

namespace scantracker
{

namespace components
{

namespace wobbleFigureEditor
{
class WobbleFigureEditor;
class PowerRampModel;
class LaserPoint;

class LaserPowerController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(precitec::scantracker::components::wobbleFigureEditor::WobbleFigureEditor* figureEditor READ figureEditor WRITE setFigureEditor NOTIFY figureEditorChanged)
    Q_PROPERTY(precitec::scantracker::components::wobbleFigureEditor::PowerRampModel* powerRampModel READ powerRampModel WRITE setPowerRampModel NOTIFY powerRampModelChanged)

public:
    explicit LaserPowerController(QObject* parent = nullptr);
    ~LaserPowerController() override;

    WobbleFigureEditor* figureEditor() const
    {
        return m_figureEditor;
    }
    void setFigureEditor(WobbleFigureEditor* newEditor);

    PowerRampModel* powerRampModel() const
    {
        return m_powerRampModel;
    }
    void setPowerRampModel(PowerRampModel* newRampModel);

    Q_INVOKABLE void applyPowerRampChanges(bool modifyDefault, bool asOffset);

Q_SIGNALS:
    void figureEditorChanged();
    void powerRampModelChanged();

private:
    WobbleFigureEditor* m_figureEditor = nullptr;
    QMetaObject::Connection m_figureEditorDestroyedConnection;
    PowerRampModel* m_powerRampModel = nullptr;
    QMetaObject::Connection m_powerRampModelDestroyedConnection;
};

}
}
}
}

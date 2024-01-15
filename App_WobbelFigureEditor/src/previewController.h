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
class FileModel;
}
}
}

namespace scanmaster
{
namespace components
{
namespace wobbleFigureEditor
{

class LaserPointController;

/**
 * A simple controller to preview a given figure.
 **/
class PreviewController : public QObject
{
    Q_OBJECT
    /**
     * File model
     * It is used to load the figure.
     **/
    Q_PROPERTY(precitec::scantracker::components::wobbleFigureEditor::FileModel* fileModel READ fileModel WRITE setFileModel NOTIFY fileModelChanged)

    /**
     * Laser point controller object
     * Is used to draw the figure.
     **/
    Q_PROPERTY(precitec::scanmaster::components::wobbleFigureEditor::LaserPointController* laserPointController READ laserPointController WRITE setLaserPointController NOTIFY laserPointControllerChanged)

public:
    PreviewController(QObject *parent = nullptr);
    ~PreviewController() override;

    precitec::scantracker::components::wobbleFigureEditor::FileModel* fileModel() const
    {
        return m_fileModel;
    }
    void setFileModel(precitec::scantracker::components::wobbleFigureEditor::FileModel* model);

    precitec::scanmaster::components::wobbleFigureEditor::LaserPointController* laserPointController() const
    {
        return m_laserPointController;
    }
    void setLaserPointController(precitec::scanmaster::components::wobbleFigureEditor::LaserPointController* newLaserPointController);

    /**
     * Preview the given seam figure at @p index in @link{fileModel}.
     **/
    Q_INVOKABLE void previewSeamFigure(const QModelIndex &index);

    /**
     * Preview the given wobble figure at @p index in @link{fileModel}.
     **/
    Q_INVOKABLE void previewWobbleFigure(const QModelIndex &index);

    /**
     * Preview the given basic figure (wobble figure) at @p index in @link{fileModel}.
     **/
    Q_INVOKABLE void previewBasicFigure(const QModelIndex &index);

Q_SIGNALS:
    void fileModelChanged();
    void laserPointControllerChanged();

private:
    precitec::scantracker::components::wobbleFigureEditor::FileModel* m_fileModel = nullptr;
    QMetaObject::Connection m_fileModelDestroyedConnection;
    precitec::scanmaster::components::wobbleFigureEditor::LaserPointController* m_laserPointController = nullptr;
    QMetaObject::Connection m_laserPointControllerDestroyedConnection;

    template <typename T, typename U>
    void preview(const QModelIndex &index, T load, U draw);
};

}
}
}
}

#pragma once

#include "hardwareParameterController.h"

namespace precitec
{
namespace gui
{

/**
 * Customized HardwareParameterController for setting ScanTracker2D specific hardware parameters.
 */
class ScanTracker2DHardwareParameterController : public HardwareParameterController
{
    Q_OBJECT
    /**
     * Whether a custom, basic or no figure is specified.
     **/
    Q_PROPERTY(precitec::gui::ScanTracker2DHardwareParameterController::Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    /**
     * The id of the selected wobble file or basic file. If none is set it is @c -1.
     **/
    Q_PROPERTY(int fileId READ fileId WRITE setFileId NOTIFY fileIdChanged)
public:
    explicit ScanTracker2DHardwareParameterController(QObject *parent = nullptr);
    ~ScanTracker2DHardwareParameterController() override;

    enum class Mode
    {
        NotSet,
        BasicFigure,
        CustomFigure
    };
    Q_ENUM(Mode)
    Mode mode() const
    {
        return m_mode;
    }
    void setMode(Mode mode);

    int fileId() const
    {
        return m_fileId;
    }
    void setFileId(int fileId);

Q_SIGNALS:
    void modeChanged();
    void fileIdChanged();

private:
    void initMode();
    void updateModeParameter();
    void initFileId();
    void resetFileId();
    /**
     * Without setting parameter
     **/
    void setFileIdInternal(int fileId);
    Mode m_mode{Mode::NotSet};
    int m_fileId{-1};
};

}
}

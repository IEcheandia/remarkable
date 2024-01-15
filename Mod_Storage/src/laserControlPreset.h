#pragma once

#include <QObject>
#include <QUuid>

namespace precitec
{
namespace gui
{
namespace components
{
namespace plotter
{

class DataSet;

}
}
}
namespace storage
{

class LaserControlPreset : public QObject
{
    Q_OBJECT

    /**
     * The uuid of the preset
     **/
    Q_PROPERTY(QUuid uuid READ uuid CONSTANT)

    /**
     * The name of the preset
     **/
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

    /**
     * Weather the preset is enabled. An enabled preset has color, while a disabled is gray
     **/
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

    /**
     * Weather the preset has unsaved changes
     **/
    Q_PROPERTY(bool hasChanges READ hasChanges NOTIFY markAsChanged)

    /**
     * The State of the preset. Indicates if the preset is new or being edited
     * Describes which buttons to display in the editor
     * Default is @c State::Default
     **/
    Q_PROPERTY(State state READ state WRITE setState NOTIFY stateChanged)

    /**
     * A small curve that shows the power levels of the first channel of the Laser Controller.
     * It is computed based on the first four values of @link{power}.
     **/
    Q_PROPERTY(precitec::gui::components::plotter::DataSet* channel1PowerDataSet READ channel1PowerDataSet CONSTANT)

    /**
     * A small curve that shows the power levels of the second channel of the Laser Controller.
     * It is computed based on the last four values of @link{power}.
     **/
    Q_PROPERTY(precitec::gui::components::plotter::DataSet* channel2PowerDataSet READ channel2PowerDataSet CONSTANT)

public:
    explicit LaserControlPreset(QObject* parent = nullptr);
    ~LaserControlPreset() override;

    enum class State {
        Default,
        Edit,
        New
    };
    Q_ENUM(State)

    enum class Key {
        LC_Parameter_No3,
        LC_Parameter_No4,
        LC_Parameter_No5,
        LC_Parameter_No6,
        LC_Parameter_No11,
        LC_Parameter_No12,
        LC_Parameter_No13,
        LC_Parameter_No14,
        LC_Parameter_No16,
        LC_Parameter_No17,
        LC_Parameter_No18,
        LC_Parameter_No19,
        LC_Parameter_No24,
        LC_Parameter_No25,
        LC_Parameter_No26,
        LC_Parameter_No27
    };
    Q_ENUM(Key)

    QUuid uuid() const
    {
        return m_uuid;
    }

    QString name() const
    {
        return m_name;
    }
    void setName(const QString &name);

    int power(uint index) const
    {
        return m_power.at(index);
    }
    void setPower(uint index, int power);
    void setPower(const std::vector<int>& power);

    int offset(uint index) const
    {
        return m_offset.at(index);
    }
    void setOffset(uint index, int offset);
    void setOffset(const std::vector<int>& offset);

    int getValue(Key key) const;
    void setValue(Key key, int value);

    bool enabled() const
    {
        return m_enabled;
    }
    void setEnabled(int enabled);

    bool hasChanges() const
    {
        return m_hasChanges;
    }
    void setChange(bool set);

    State state() const
    {
        return m_state;
    }
    void setState(State state);

    QString filePath() const
    {
        return m_filePath;
    }
    void setFilePath(const QString &path);

    precitec::gui::components::plotter::DataSet* channel1PowerDataSet() const
    {
        return m_channel1Power;
    }

    precitec::gui::components::plotter::DataSet* channel2PowerDataSet() const
    {
        return m_channel2Power;
    }

    static LaserControlPreset *load(const QString &filePath, QObject* parent);
    static bool deleteFile(const QString &filePath);
    void save(const QString &filePath);
    Q_INVOKABLE void save();
    Q_INVOKABLE void restore();

    static const uint numberOfSamples()
    {
        return m_numberOfSamples;
    }

Q_SIGNALS:
    void nameChanged();
    void powerChanged(uint index);
    void offsetChanged(uint index);
    void enabledChanged();
    void markAsChanged();
    void stateChanged();
    void editStarted();
    void editStopped();
    void channel1SamplesChanged();
    void channel2SamplesChanged();

private:
    void init();
    void computeChannel1Samples();
    void computeChannel2Samples();

    static constexpr int m_numberOfSamples = 8;

    QUuid m_uuid;
    QString m_name = QString();
    QString m_filePath = QString();

    bool m_enabled = true;
    bool m_hasChanges = false;

    std::vector<int> m_power;
    std::vector<int> m_offset;
    State m_state = State::Default;

    precitec::gui::components::plotter::DataSet* m_channel1Power;
    precitec::gui::components::plotter::DataSet* m_channel2Power;
};

}
}
Q_DECLARE_METATYPE(precitec::storage::LaserControlPreset*)


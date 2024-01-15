#pragma once
#include "abstractParameterSetDeltaModel.h"

namespace precitec
{

namespace storage
{
class Parameter;
}

namespace gui
{

class ParameterSetToGraphDeltaModel : public AbstractParameterSetDeltaModel
{
    Q_OBJECT
    Q_PROPERTY(QString longestValueSeam READ longestValueSeam NOTIFY longestValueSeamChanged)
    Q_PROPERTY(QString longestValueGraph READ longestValueGraph NOTIFY longestValueGraphChanged)
public:
    explicit ParameterSetToGraphDeltaModel(QObject *parent = nullptr);
    ~ParameterSetToGraphDeltaModel() override;

    QString longestValueSeam() const
    {
        return m_longestValueSeam;
    }
    QString longestValueGraph() const
    {
        return m_longestValueGraph;
    }

    /**
     * The original filter Parameter at @p index.
     **/
    Q_INVOKABLE precitec::storage::Parameter *getFilterParameter(const QModelIndex &index) const;

    /**
     * Updates the filter Parameter at @p index and the original filter Parameter in the seam to @p value.
     **/
    Q_INVOKABLE void updateFilterParameter(const QModelIndex &index, const QVariant &value);

Q_SIGNALS:
    void longestValueSeamChanged();
    void longestValueGraphChanged();

protected:
    void init() override;
private:
    QString m_longestValueSeam;
    QString m_longestValueGraph;
};

}
}

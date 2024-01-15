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

class ParameterSetsDeltaModel : public AbstractParameterSetDeltaModel
{
    Q_OBJECT
public:
    explicit ParameterSetsDeltaModel(QObject *parent = nullptr);
    ~ParameterSetsDeltaModel() override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    /**
     * The original filter Parameter at @p index.
     **/
    Q_INVOKABLE precitec::storage::Parameter *getFilterParameter(const QModelIndex &index) const;

    /**
     * Updates the filter Parameter at @p index and the original filter Parameter in the seam to @p value.
     **/
    Q_INVOKABLE void updateFilterParameter(const QModelIndex &index, const QVariant &value);

protected:
    void init() override;

private:
    std::vector<storage::Seam*> m_measureTaks;
    std::vector<QString> m_maxString;
};

}
}

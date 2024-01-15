#pragma once
#include "seam.h"

namespace precitec
{
namespace storage
{
enum class CopyMode;

/**
 * A LinkedSeam is a specialized Seam which shares (almost) all settings from the Seam.
 * It is not used for processing. During processing the Seam the LinkedSeam links to
 * is invoked several times. Internally this situation gets represented by the LinkedSeam.
 *
 * In difference to the Seam the LinkedSeam has a unique @link{label} and a pointer to the
 * Seam it @link{link}s to. A Seam can be linked to multiple LinkedSeams but the label must
 * be unique for all LinkedSeams.
 *
 * The lifetime of a LinkedSeam is connected to it's Seam. If a Seam gets destroyed all LinkedSeams
 * get destroyed as well.
 **/
class LinkedSeam : public Seam
{
    Q_OBJECT
    Q_PROPERTY(QString label READ label CONSTANT)
    Q_PROPERTY(precitec::storage::Seam *linkTo READ linkTo CONSTANT)
public:
    ~LinkedSeam() override;

    QString label() const
    {
        return m_label;
    }

    precitec::storage::Seam *linkTo() const
    {
        return m_link;
    }

    LinkedSeam *clone(CopyMode mode, Seam *link) const;
    QJsonObject toJson() const override;

    static LinkedSeam *create(QUuid newUuid, Seam *link, const QString &label);

    static LinkedSeam *fromJson(const QJsonObject &object, Seam *parent);

private:
    LinkedSeam(QUuid uuid, Seam *link);
    QString m_label;
    precitec::storage::Seam *m_link;
};

}
}

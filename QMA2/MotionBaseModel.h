#ifndef MOTIONBASEMODEL_H
#define MOTIONBASEMODEL_H

#include <QtGui/QAbstractItemView>

namespace vpvl {
class Bone;
class Face;
class PMDModel;
class VMDMotion;
class VPDPose;
}

class MotionBaseModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit MotionBaseModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

protected:
    vpvl::PMDModel *m_model;
    QList<QString> m_keys;
    QHash< QPair<int, int>, QVariant > m_values;
};

#endif // MOTIONBASEMODEL_H

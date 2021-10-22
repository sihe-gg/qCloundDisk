#include "logininfo.h"

LoginInfo::LoginInfo(QObject *parent)
    : QAbstractItemModel(parent)
{
}

QVariant LoginInfo::headerData(int section, Qt::Orientation orientation, int role) const
{
    // FIXME: Implement me!
}

QModelIndex LoginInfo::index(int row, int column, const QModelIndex &parent) const
{
    // FIXME: Implement me!
}

QModelIndex LoginInfo::parent(const QModelIndex &index) const
{
    // FIXME: Implement me!
}

int LoginInfo::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 0;

    // FIXME: Implement me!
}

int LoginInfo::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 0;

    // FIXME: Implement me!
}

QVariant LoginInfo::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    // FIXME: Implement me!
    return QVariant();
}

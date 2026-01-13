#include "registdialog.h"

RegistDialog::RegistDialog(QObject *parent)
    : QAbstractItemModel(parent)
{}

QVariant RegistDialog::headerData(int section, Qt::Orientation orientation, int role) const
{
    // FIXME: Implement me!
}

QModelIndex RegistDialog::index(int row, int column, const QModelIndex &parent) const
{
    // FIXME: Implement me!
}

QModelIndex RegistDialog::parent(const QModelIndex &index) const
{
    // FIXME: Implement me!
}

int RegistDialog::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 0;

    // FIXME: Implement me!
}

int RegistDialog::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 0;

    // FIXME: Implement me!
}

QVariant RegistDialog::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    // FIXME: Implement me!
    return QVariant();
}

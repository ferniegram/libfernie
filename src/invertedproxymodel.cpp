#include "invertedproxymodel.h"

#define DEBUG_MODULE InvertedProxyModel
#include "debuglog.h"

InvertedProxyModel::InvertedProxyModel(QObject *parent) : QSortFilterProxyModel(parent) {
    setDynamicSortFilter(true);
}

void InvertedProxyModel::setSource(QObject *model) {
    setSourceModel(qobject_cast<QAbstractItemModel*>(model));
}

void InvertedProxyModel::setSourceModel(QAbstractItemModel *model) {
    if (sourceModel() != model) {
        QSortFilterProxyModel::setSourceModel(model);
        emit sourceChanged();
    }
}

QModelIndex InvertedProxyModel::mapFromSource(const QModelIndex &sourceIndex) const {
    if (!sourceIndex.isValid()) return QModelIndex();
    int row = sourceModel()->rowCount(sourceIndex.parent()) - sourceIndex.row() - 1;
    return createIndex(row, sourceIndex.column(), sourceIndex.internalPointer());
}

QModelIndex InvertedProxyModel::mapToSource(const QModelIndex &proxyIndex) const {
    if (!proxyIndex.isValid()) return QModelIndex();
    int row = sourceModel()->rowCount(proxyIndex.parent()) - proxyIndex.row() - 1;
    return sourceModel()->index(row, proxyIndex.column(), proxyIndex.parent());
}

#ifndef INVERTEDPROXYMODEL_H
#define INVERTEDPROXYMODEL_H

#include <QSortFilterProxyModel>

class InvertedProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

    Q_PROPERTY(QObject* sourceModel READ sourceModel WRITE setSource NOTIFY sourceChanged)

public:
    explicit InvertedProxyModel(QObject *parent = nullptr);

    void setSource(QObject* model);
    void setSourceModel(QAbstractItemModel *model) Q_DECL_OVERRIDE;

    virtual QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;
    virtual QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;

signals:
    void sourceChanged();
};

#endif // INVERTEDPROXYMODEL_H

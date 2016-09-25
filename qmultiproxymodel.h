#ifndef QMULTIPROXYMODEL_H
#define QMULTIPROXYMODEL_H

#include <QAbstractProxyModel>

class QMultiProxyModelPrivate;

class QMultiProxyModel : public QAbstractProxyModel
{
    Q_OBJECT
public:
    explicit QMultiProxyModel(QObject *parent = 0);
    virtual ~QMultiProxyModel();

    QList<QAbstractItemModel *> sourceModels();
    bool addSourceModel(QAbstractItemModel *model);
    bool removeSourceModel(QAbstractItemModel *model);
    void clearSourceModelsList();
    bool containsSourceModel(QAbstractItemModel *model);

    virtual QVariant data(const QModelIndex &proxyIndex, int role) const;
    virtual QModelIndex mapToSource(const QModelIndex &proxyIndex) const;
    virtual QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;

    virtual QItemSelection mapSelectionToSource(const QItemSelection &selection) const;
    virtual QItemSelection mapSelectionFromSource(const QItemSelection &selection) const;

    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& child) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QModelIndex buddy(const QModelIndex &index) const;

#if QT_VERSION >= 0x050000
    QHash<int, QByteArray> roleNames() const;
#endif

private:
    void setSourceModel(QAbstractItemModel *sourceModel) { Q_UNUSED(sourceModel)}

    QMultiProxyModelPrivate *const d_ptr;
    Q_DECLARE_PRIVATE(QMultiProxyModel)

    Q_PRIVATE_SLOT(d_func(), void _q_rowsAboutToBeInserted(QModelIndex,int,int))
    Q_PRIVATE_SLOT(d_func(), void _q_rowsInserted(QModelIndex,int,int))
    Q_PRIVATE_SLOT(d_func(), void _q_rowsAboutToBeRemoved(QModelIndex,int,int))
    Q_PRIVATE_SLOT(d_func(), void _q_rowsRemoved(QModelIndex,int,int))
    Q_PRIVATE_SLOT(d_func(), void _q_rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int))
    Q_PRIVATE_SLOT(d_func(), void _q_rowsMoved( QModelIndex,int,int,QModelIndex,int))

    Q_PRIVATE_SLOT(d_func(), void _q_columnsAboutToBeInserted(QModelIndex,int,int))
    Q_PRIVATE_SLOT(d_func(), void _q_columnsInserted(QModelIndex,int,int))
    Q_PRIVATE_SLOT(d_func(), void _q_columnsAboutToBeRemoved(QModelIndex,int,int))
    Q_PRIVATE_SLOT(d_func(), void _q_columnsRemoved(QModelIndex,int,int))
    Q_PRIVATE_SLOT(d_func(), void _q_columnsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int))
    Q_PRIVATE_SLOT(d_func(), void _q_columnsMoved(QModelIndex,int,int,QModelIndex,int))

    Q_PRIVATE_SLOT(d_func(), void _q_modelAboutToBeReset())
    Q_PRIVATE_SLOT(d_func(), void _q_modelReset())
    Q_PRIVATE_SLOT(d_func(), void _q_headerDataChanged(Qt::Orientation,int,int))

#if QT_VERSION < 0x050000
    Q_PRIVATE_SLOT(d_func(), void _q_layoutAboutToBeChanged())
    Q_PRIVATE_SLOT(d_func(), void _q_layoutChanged())
    Q_PRIVATE_SLOT(d_func(), void _q_dataChanged(QModelIndex,QModelIndex))
#else
    Q_PRIVATE_SLOT(d_func(), void _q_layoutAboutToBeChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint))
    Q_PRIVATE_SLOT(d_func(), void _q_layoutChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint))
    Q_PRIVATE_SLOT(d_func(), void _q_dataChanged(QModelIndex,QModelIndex,QVector<int>))
#endif
};

#endif // QMULTIPROXYMODEL_H

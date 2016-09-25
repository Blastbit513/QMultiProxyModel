#include "qmultiproxymodel.h"
#include <QDebug>
#include <QItemSelection>

class QMultiProxyModelPrivate
{
    QMultiProxyModel *const q_ptr;
    Q_DECLARE_PUBLIC(QMultiProxyModel)
    QList<QAbstractItemModel *> m_sourceModels;

#if QT_VERSION >= 0x050000
    QHash<int, QByteArray> m_rolenames;
#endif

    QMultiProxyModelPrivate(QMultiProxyModel *qptr);
    void updateRolenames();
    int offsetForModel(const QAbstractItemModel *) const;
    const QAbstractItemModel * sourceModelByProxyRow(int row) const;

public /* slots */:
    void _q_rowsAboutToBeInserted(const QModelIndex &parent, int start, int end);
    void _q_rowsInserted(const QModelIndex &parent, int start, int end);
    void _q_rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    void _q_rowsRemoved(const QModelIndex &parent, int start, int end);
    void _q_rowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest);
    void _q_rowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest);

    void _q_columnsAboutToBeInserted(const QModelIndex &parent, int start, int end);
    void _q_columnsInserted(const QModelIndex &parent, int start, int end);
    void _q_columnsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    void _q_columnsRemoved(const QModelIndex &parent, int start, int end);
    void _q_columnsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest);
    void _q_columnsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest);

    void _q_modelAboutToBeReset();
    void _q_modelReset();
    void _q_headerDataChanged(Qt::Orientation orientation, int first, int last);

#if QT_VERSION < 0x050000
    void _q_layoutAboutToBeChanged();
    void _q_layoutChanged();
    void _q_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
#else
    void _q_layoutAboutToBeChanged(const QList<QPersistentModelIndex> &sourceParents, QAbstractItemModel::LayoutChangeHint hint);
    void _q_layoutChanged(const QList<QPersistentModelIndex> &sourceParents, QAbstractItemModel::LayoutChangeHint hint);
    void _q_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
#endif
};

QMultiProxyModelPrivate::QMultiProxyModelPrivate(QMultiProxyModel *qptr) : q_ptr(qptr)
{
}

int QMultiProxyModelPrivate::offsetForModel(const QAbstractItemModel *sourceModel) const
{
    int offset = 0;
    foreach (QAbstractItemModel *model, m_sourceModels) {
        if (sourceModel == model) {
            return offset;
        }
        offset += model->rowCount();
    }
    return -1;
}

const QAbstractItemModel *QMultiProxyModelPrivate::sourceModelByProxyRow(int row) const
{
    int offset = 0;
    foreach (const QAbstractItemModel *model, m_sourceModels) {
        if ((model->rowCount() + offset) > row) {
            return model;
        }
        offset += model->rowCount();
    }
    return 0;
}

/*!
 * \todo In Qt5 we have no possibility to notify viewers about update of roleNames if the viewer has been using the proxy model.
 * May be we can resetModel, but it's overhead.
 * Anyway it's an expansion of functionality and not necessary now.
 */
void QMultiProxyModelPrivate::updateRolenames()
{
#if QT_VERSION < 0x050000
    Q_Q(QMultiProxyModel);
    QHash<int,QByteArray> allRoleNames = q->roleNames();
    bool changed = false;
#else
    QHash<int,QByteArray> &allRoleNames = m_rolenames;
#endif

    foreach (QAbstractItemModel *model, m_sourceModels) {
        const QHash<int,QByteArray> &modelRN = model->roleNames();
        foreach (int role, modelRN.uniqueKeys()) {
            if (!allRoleNames.contains(role) || allRoleNames.value(role) != modelRN.value(role) ) {
#if QT_VERSION < 0x050000
                changed = true;
#endif
                allRoleNames.insertMulti(role, modelRN.value(role));
            }
        }
    }
#if QT_VERSION < 0x050000
    if (changed) {
        q->setRoleNames(allRoleNames);
    }
#endif
}

void QMultiProxyModelPrivate::_q_rowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
    Q_Q(QMultiProxyModel);
    QAbstractItemModel *srcModel = qobject_cast<QAbstractItemModel*>(q->sender());
    Q_ASSERT(srcModel);
    Q_ASSERT(parent.isValid() ? parent.model() == srcModel : true);

    int offset = offsetForModel(srcModel);
    q->beginInsertRows(parent, offset+start, offset+end);
}

void QMultiProxyModelPrivate::_q_rowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(start)
    Q_UNUSED(end)

    Q_Q(QMultiProxyModel);
    updateRolenames();
    QAbstractItemModel *srcModel = qobject_cast<QAbstractItemModel*>(q->sender());
    Q_ASSERT(srcModel);
    Q_ASSERT(parent.isValid() ? parent.model() == srcModel : true);

    q->endInsertRows();
}

void QMultiProxyModelPrivate::_q_rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    Q_Q(QMultiProxyModel);
    QAbstractItemModel *srcModel = qobject_cast<QAbstractItemModel*>(q->sender());
    Q_ASSERT(srcModel);
    Q_ASSERT(parent.isValid() ? parent.model() == srcModel : true);

    int offset = offsetForModel(srcModel);
    q->beginRemoveRows(q->mapFromSource(parent), offset+start, offset+end);
}

void QMultiProxyModelPrivate::_q_rowsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)

    Q_Q(QMultiProxyModel);
    q->endRemoveRows();
}

void QMultiProxyModelPrivate::_q_rowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest)
{
    Q_Q(QMultiProxyModel);
    QAbstractItemModel *srcModel = qobject_cast<QAbstractItemModel*>(q->sender());
    Q_ASSERT(srcModel);
    Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == srcModel : true);
    Q_ASSERT(destParent.isValid() ? destParent.model() == srcModel : true);

    int offset = offsetForModel(srcModel);
    q->beginMoveRows(q->mapFromSource(sourceParent), offset+sourceStart, offset+sourceEnd, q->mapFromSource(destParent), offset+dest);
}

void QMultiProxyModelPrivate::_q_rowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest)
{
    Q_UNUSED(sourceStart)
    Q_UNUSED(sourceEnd)
    Q_UNUSED(dest)

    Q_Q(QMultiProxyModel);
    QAbstractItemModel *srcModel = qobject_cast<QAbstractItemModel*>(q->sender());
    Q_ASSERT(srcModel);
    Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == srcModel : true);
    Q_ASSERT(destParent.isValid() ? destParent.model() == srcModel : true);

    q->endMoveRows();
}

void QMultiProxyModelPrivate::_q_columnsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
    Q_Q(QMultiProxyModel);
    QAbstractItemModel *srcModel = qobject_cast<QAbstractItemModel*>(q->sender());
    Q_ASSERT(srcModel);
    Q_ASSERT(parent.isValid() ? parent.model() == srcModel : true);
    q->beginInsertColumns(q->mapFromSource(parent), start, end);
}

void QMultiProxyModelPrivate::_q_columnsInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(start)
    Q_UNUSED(end)

    Q_Q(QMultiProxyModel);
    QAbstractItemModel *srcModel = qobject_cast<QAbstractItemModel*>(q->sender());
    Q_ASSERT(srcModel);
    Q_ASSERT(parent.isValid() ? parent.model() == srcModel : true);

    updateRolenames();
    q->endInsertColumns();
}

void QMultiProxyModelPrivate::_q_columnsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    Q_Q(QMultiProxyModel);
    QAbstractItemModel *srcModel = qobject_cast<QAbstractItemModel*>(q->sender());
    Q_ASSERT(srcModel);
    Q_ASSERT(parent.isValid() ? parent.model() == srcModel : true);
    q->beginRemoveColumns(q->mapFromSource(parent), start, end);
}

void QMultiProxyModelPrivate::_q_columnsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(start)
    Q_UNUSED(end)

    Q_Q(QMultiProxyModel);
    QAbstractItemModel *srcModel = qobject_cast<QAbstractItemModel*>(q->sender());
    Q_ASSERT(srcModel);
    Q_ASSERT(parent.isValid() ? parent.model() == srcModel : true);

    q->endRemoveColumns();
}

void QMultiProxyModelPrivate::_q_columnsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest)
{
    Q_Q(QMultiProxyModel);
    QAbstractItemModel *srcModel = qobject_cast<QAbstractItemModel*>(q->sender());
    Q_ASSERT(srcModel);
    Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == srcModel : true);
    Q_ASSERT(destParent.isValid() ? destParent.model() == srcModel : true);

    q->beginMoveColumns(q->mapFromSource(sourceParent), sourceStart, sourceEnd, q->mapFromSource(destParent), dest);
}

void QMultiProxyModelPrivate::_q_columnsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destParent, int dest)
{
    Q_UNUSED(sourceStart)
    Q_UNUSED(sourceEnd)
    Q_UNUSED(dest)

    Q_Q(QMultiProxyModel);
    QAbstractItemModel *srcModel = qobject_cast<QAbstractItemModel*>(q->sender());
    Q_ASSERT(srcModel);
    Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == srcModel : true);
    Q_ASSERT(destParent.isValid() ? destParent.model() == srcModel : true);

    q->endMoveColumns();
}

void QMultiProxyModelPrivate::_q_modelAboutToBeReset()
{
    Q_Q(QMultiProxyModel);
    emit q->beginResetModel();
}

void QMultiProxyModelPrivate::_q_modelReset()
{
    Q_Q(QMultiProxyModel);
    emit q->endResetModel();
}

/*!
 * \todo I have no definite idea what to do if we have several models with different headers.
 * Perhaps we need to merge headers.
 */
void QMultiProxyModelPrivate::_q_headerDataChanged(Qt::Orientation orientation, int first, int last)
{
    Q_Q(QMultiProxyModel);
    emit q->headerDataChanged(orientation, first, last);
}

#if QT_VERSION < 0x050000
void QMultiProxyModelPrivate::_q_layoutAboutToBeChanged()
{
    Q_Q(QMultiProxyModel);
    emit q->layoutAboutToBeChanged();
}
#else
void QMultiProxyModelPrivate::_q_layoutAboutToBeChanged(const QList<QPersistentModelIndex> &sourceParents, QAbstractItemModel::LayoutChangeHint hint)
{
    Q_UNUSED(sourceParents)
    Q_UNUSED(hint)

    Q_Q(QMultiProxyModel);
    emit q->layoutAboutToBeChanged();
}
#endif

#if QT_VERSION < 0x050000
void QMultiProxyModelPrivate::_q_layoutChanged()
{
    Q_Q(QMultiProxyModel);
    emit q->layoutChanged();
}
#else
void QMultiProxyModelPrivate::_q_layoutChanged(const QList<QPersistentModelIndex> &sourceParents, QAbstractItemModel::LayoutChangeHint hint)
{
    Q_UNUSED(sourceParents)
    Q_UNUSED(hint)

    Q_Q(QMultiProxyModel);
    emit q->layoutChanged();
}
#endif

#if QT_VERSION < 0x050000
void QMultiProxyModelPrivate::_q_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_Q(QMultiProxyModel);
    Q_ASSERT(topLeft.isValid() ? topLeft.model() != q : true);
    Q_ASSERT(bottomRight.isValid() ? bottomRight.model() != q : true);
    emit q->dataChanged(q->mapFromSource(topLeft), q->mapFromSource(bottomRight));
}
#else
void QMultiProxyModelPrivate::_q_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    Q_Q(QMultiProxyModel);
    Q_ASSERT(topLeft.isValid() ? topLeft.model() != q : true);
    Q_ASSERT(bottomRight.isValid() ? bottomRight.model() != q : true);
    emit q->dataChanged(q->mapFromSource(topLeft), q->mapFromSource(bottomRight), roles);
}
#endif

/*!
    \class QMultiProxyModel
    \brief The QMultiProxyModel class provides several item models as one model.
    \ingroup model-view

    \note If the source model is deleted or no source model is specified, the
    proxy model operates on a empty placeholder model.
    \sa QSortFilterProxyModel, QAbstractItemModel, {Model/View Programming}
*/

/*!
 * \brief Constructs a proxy model with the given parent.
 */
QMultiProxyModel::QMultiProxyModel(QObject *parent) :
    QAbstractProxyModel(parent),
    d_ptr(new QMultiProxyModelPrivate(this))
{

}

/*!
 * \brief Destroys the proxy model.
 */
QMultiProxyModel::~QMultiProxyModel()
{

}

/*!
 *  \brief Returns the models that contains the data that is available through the proxy model.
 *  \return List of models
 */
QList<QAbstractItemModel *> QMultiProxyModel::sourceModels()
{
    Q_D(QMultiProxyModel);
    return d->m_sourceModels;
}

/*!
 * \brief Adds the given source model into list for processing by the proxy model.
 * \note Source models will be provided in the order they were added.
 * \param model
 * \return Returns false if the given model is NULL or already contained in the model's list; otherwise returns true.
 */
bool QMultiProxyModel::addSourceModel(QAbstractItemModel *model)
{
    Q_D(QMultiProxyModel);

    if (!model || d->m_sourceModels.contains(model)) {
        return false;
    }

    beginResetModel();
    d->m_sourceModels.append(model);
    d->updateRolenames();

    connect(model, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
            SLOT(_q_rowsAboutToBeInserted(QModelIndex,int,int)));
    connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
            SLOT(_q_rowsInserted(QModelIndex,int,int)));
    connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            SLOT(_q_rowsAboutToBeRemoved(QModelIndex,int,int)));
    connect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            SLOT(_q_rowsRemoved(QModelIndex,int,int)));
    connect(model, SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
            SLOT(_q_rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
    connect(model, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
            SLOT(_q_rowsMoved(QModelIndex,int,int,QModelIndex,int)));
    connect(model, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)),
            SLOT(_q_columnsAboutToBeInserted(QModelIndex,int,int)));
    connect(model, SIGNAL(columnsInserted(QModelIndex,int,int)),
            SLOT(_q_columnsInserted(QModelIndex,int,int)));
    connect(model, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
            SLOT(_q_columnsAboutToBeRemoved(QModelIndex,int,int)));
    connect(model, SIGNAL(columnsRemoved(QModelIndex,int,int)),
            SLOT(_q_columnsRemoved(QModelIndex,int,int)));
    connect(model, SIGNAL(columnsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
            SLOT(_q_columnsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
    connect(model, SIGNAL(columnsMoved(QModelIndex,int,int,QModelIndex,int)),
            SLOT(_q_columnsMoved(QModelIndex,int,int,QModelIndex,int)));

    connect(model, SIGNAL(modelAboutToBeReset()),
            SLOT(_q_modelAboutToBeReset()));
    connect(model, SIGNAL(modelReset()),
            SLOT(_q_modelReset()));
    connect(model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
            SLOT(_q_headerDataChanged(Qt::Orientation,int,int)));

#if QT_VERSION < 0x050000
    connect(model, SIGNAL(layoutAboutToBeChanged()),
            SLOT(_q_layoutAboutToBeChanged()));
    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            SLOT(_q_dataChanged(QModelIndex,QModelIndex)));
    connect(model, SIGNAL(layoutChanged()),
            SLOT(_q_layoutChanged()));
#else
    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            SLOT(_q_dataChanged(QModelIndex,QModelIndex,QVector<int>)));
    connect(model, SIGNAL(layoutAboutToBeChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)),
            SLOT(_q_layoutAboutToBeChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)));
    connect(model, SIGNAL(layoutChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)),
            SLOT(_q_layoutChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)));
#endif

    endResetModel();

    return true;
}

/*!
 * \brief Removes the given source model from the model's list.
 * \note The proxy model will be reseted.
 * \param model
 * \return Returns true if the proxy model contains an occurrence of the given model; otherwise returns false.
 */
bool QMultiProxyModel::removeSourceModel(QAbstractItemModel *model)
{
    Q_D(QMultiProxyModel);
    if (d->m_sourceModels.contains(model)) {
        beginResetModel();

        if (model) {
            disconnect(model, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
                       this, SLOT(_q_rowsAboutToBeInserted(QModelIndex,int,int)));
            disconnect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                       this, SLOT(_q_rowsInserted(QModelIndex,int,int)));
            disconnect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                       this, SLOT(_q_rowsAboutToBeRemoved(QModelIndex,int,int)));
            disconnect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                       this, SLOT(_q_rowsRemoved(QModelIndex,int,int)));
            disconnect(model, SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
                       this, SLOT(_q_rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
            disconnect(model, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
                       this, SLOT(_q_rowsMoved(QModelIndex,int,int,QModelIndex,int)));
            disconnect(model, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)),
                       this, SLOT(_q_columnsAboutToBeInserted(QModelIndex,int,int)));
            disconnect(model, SIGNAL(columnsInserted(QModelIndex,int,int)),
                       this, SLOT(_q_columnsInserted(QModelIndex,int,int)));
            disconnect(model, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
                       this, SLOT(_q_columnsAboutToBeRemoved(QModelIndex,int,int)));
            disconnect(model, SIGNAL(columnsRemoved(QModelIndex,int,int)),
                       this, SLOT(_q_columnsRemoved(QModelIndex,int,int)));
            disconnect(model, SIGNAL(columnsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
                       this, SLOT(_q_columnsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
            disconnect(model, SIGNAL(columnsMoved(QModelIndex,int,int,QModelIndex,int)),
                       this, SLOT(_q_columnsMoved(QModelIndex,int,int,QModelIndex,int)));

            disconnect(model, SIGNAL(modelAboutToBeReset()),
                       this, SLOT(_q_modelAboutToBeReset()));
            disconnect(model, SIGNAL(modelReset()),
                       this, SLOT(_q_modelReset()));
            disconnect(model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
                       this, SLOT(_q_headerDataChanged(Qt::Orientation,int,int)));

#if QT_VERSION < 0x050000
            disconnect(model, SIGNAL(layoutAboutToBeChanged()),
                       this, SLOT(_q_layoutAboutToBeChanged()));
            disconnect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                       this, SLOT(_q_dataChanged(QModelIndex,QModelIndex)));
            disconnect(model, SIGNAL(layoutChanged()),
                       this, SLOT(_q_layoutChanged()));
#else
            disconnect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
                       this, SLOT(_q_dataChanged(QModelIndex,QModelIndex,QVector<int>)));
            disconnect(model, SIGNAL(layoutAboutToBeChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)),
                       this, SLOT(_q_layoutAboutToBeChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)));
            disconnect(model, SIGNAL(layoutChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)),
                       this, SLOT(_q_layoutChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)));
#endif
        }
        d->m_sourceModels.removeAll(model);
        endResetModel();
    } else {
        return false;
    }
    return true;
}

/*!
 * \brief Clear the model's list.
 * \sa removeSourceModel()
 * \todo Don't reset the proxy model every iteration
 */
void QMultiProxyModel::clearSourceModelsList()
{
    Q_D(QMultiProxyModel);
    foreach (QAbstractItemModel *model, d->m_sourceModels) {
        removeSourceModel(model);
    }
}

/*!
 * \param model
 * \return Returns true if the proxy model contains an occurrence of the given model; otherwise returns false.
 */
bool QMultiProxyModel::containsSourceModel(QAbstractItemModel *model)
{
    Q_D(QMultiProxyModel);
    return d->m_sourceModels.contains(model);
}

/*!
 * \brief reimplemented QAbstractProxyModel::data
 */
QVariant QMultiProxyModel::data(const QModelIndex &proxyIndex, int role) const
{
    Q_D(const QMultiProxyModel);
    const QAbstractItemModel *model = d->sourceModelByProxyRow(proxyIndex.row());
    if (!model) {
        return QVariant();
    }
    return model->data(mapToSource(proxyIndex), role);
}

/*!
 * \brief reimplemented QAbstractProxyModel::mapToSource
 */
QModelIndex QMultiProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!proxyIndex.isValid())
        return QModelIndex();
    Q_ASSERT(proxyIndex.model() == this);

    Q_D(const QMultiProxyModel);
    const QAbstractItemModel *model = d->sourceModelByProxyRow(proxyIndex.row());
    if (model) {
        int newRow = proxyIndex.row() - d->offsetForModel(model);
        return model->index(newRow, proxyIndex.column());
    }
    return QModelIndex();
}

/*!
 * \brief reimplemented QAbstractProxyModel::mapFromSource
 */
QModelIndex QMultiProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid())
        return QModelIndex();
    Q_ASSERT(sourceIndex.model() != this);

    Q_D(const QMultiProxyModel);
    int row = d->offsetForModel(sourceIndex.model()) + sourceIndex.row();
    return createIndex(row, sourceIndex.column());
}


/*! \todo Check ranges for multi selection.
 * If \a it and \a end from different models we need to return several ranges for every model.
 * Otherwise returns how it made now.
 */

/*!
 * \brief reimplemented QAbstractProxyModel::mapSelectionToSource
 */
QItemSelection QMultiProxyModel::mapSelectionToSource(const QItemSelection &selection) const
{
    QItemSelection sourceSelection;

    QItemSelection::const_iterator it = selection.constBegin();
    const QItemSelection::const_iterator end = selection.constEnd();
    for ( ; it != end; ++it) {
        const QItemSelectionRange range(mapToSource(it->topLeft()), mapToSource(it->bottomRight()));
        sourceSelection.append(range);
    }

    return sourceSelection;
}

/*!
 * \brief reimplemented QAbstractProxyModel::mapSelectionFromSource
 */
QItemSelection QMultiProxyModel::mapSelectionFromSource(const QItemSelection &selection) const
{
    QItemSelection proxySelection;

    QItemSelection::const_iterator it = selection.constBegin();
    const QItemSelection::const_iterator end = selection.constEnd();
    for ( ; it != end; ++it) {
        const QItemSelectionRange range(mapFromSource(it->topLeft()), mapFromSource(it->bottomRight()));
        proxySelection.append(range);
    }

    return proxySelection;
}

/*!
 * \brief reimplemented QAbstractProxyModel::columnCount
 */
int QMultiProxyModel::columnCount(const QModelIndex &parent) const
{
    Q_ASSERT(parent.isValid() ? parent.model() == this : true);

    Q_D(const QMultiProxyModel);
    const QAbstractItemModel *model = d->sourceModelByProxyRow(parent.row());
    if (!model) {
        return 0;
    }
    return model->columnCount(mapToSource(parent));
}

/*!
 * \brief reimplemented QAbstractProxyModel::index
 */
QModelIndex QMultiProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return createIndex(row, column);
}

/*!
 * \brief reimplemented QAbstractProxyModel::parent
 */
QModelIndex QMultiProxyModel::parent(const QModelIndex &child) const
{
    Q_ASSERT(child.isValid() ? child.model() == this : true);
    const QModelIndex sourceIndex = mapToSource(child);
    const QModelIndex sourceParent = sourceIndex.parent();
    return mapFromSource(sourceParent);
}

/*!
 * \brief reimplemented QAbstractProxyModel::rowCount
 */
int QMultiProxyModel::rowCount(const QModelIndex &parent) const
{
    Q_ASSERT(parent.isValid() ? parent.model() == this : true);

    Q_D(const QMultiProxyModel);
    int rowCountSum = 0;
    foreach (QAbstractItemModel *model, d->m_sourceModels) {
        rowCountSum += model->rowCount(parent);
    }
    return rowCountSum;
}

/*!
 * \brief reimplemented QAbstractProxyModel::flags
 */
Qt::ItemFlags QMultiProxyModel::flags(const QModelIndex &index) const{
    Q_D(const QMultiProxyModel);
    const QAbstractItemModel *model = d->sourceModelByProxyRow(index.row());
    if (model) {
        return model->flags(mapToSource(this->index(index.row(), index.column())));
    }
    return QAbstractProxyModel::flags(index);
}

/*!
 * \brief reimplemented QAbstractProxyModel::buddy
 */
QModelIndex QMultiProxyModel::buddy(const QModelIndex &index) const{
    Q_D(const QMultiProxyModel);
    QModelIndex source_index = mapToSource(index);
    QModelIndex source_buddy;
    const QAbstractItemModel *model = d->sourceModelByProxyRow(index.row());
    if (model) {
        source_buddy = model->buddy(source_index);
    }

    if (source_index == source_buddy) {
        return index;
    }
    return mapFromSource(source_buddy);
}

#if QT_VERSION >= 0x050000

/*!
 * \brief overriden QAbstractProxyModel::roleNames
 */
QHash<int, QByteArray> QMultiProxyModel::roleNames() const
{
    Q_D(const QMultiProxyModel);
    return d->m_rolenames;
}
#endif

#include "moc_qmultiproxymodel.cpp"

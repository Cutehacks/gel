#include "collection.h"
#include "jsonlistmodel.h"

namespace com { namespace cutehacks { namespace gel {

Collection::Collection(QObject *parent) : QSortFilterProxyModel(parent)
{
    connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(emitCountChanged()));
    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(emitCountChanged()));

    sort(0);
}

QJSValue Collection::comparator() const
{
    return m_comparator;
}

QJSValue Collection::filter() const
{
    return m_filter;
}

JsonListModel *Collection::model() const
{
    return qobject_cast<JsonListModel*>(sourceModel());
}

void Collection::setComparator(QJSValue comparator)
{
    if (m_comparator.strictlyEquals(comparator))
        return;

    m_comparator = comparator;
    updateModel();
    emit comparatorChanged(comparator);
}

void Collection::setFilter(QJSValue filter)
{
    if (m_filter.strictlyEquals(filter))
        return;

    m_filter = filter;
    invalidateFilter();
    emit filterChanged(filter);
}

void Collection::setModel(JsonListModel *model)
{
    JsonListModel *oldModel = qobject_cast<JsonListModel*>(sourceModel());
    if (oldModel == model)
        return;

    if (oldModel) {
        disconnect(oldModel, SIGNAL(rolesChanged()),
                   this, SLOT(rolesChanged()));
    }

    setSourceModel(model);
    updateModel();
    connect(model, SIGNAL(rolesChanged()), this, SLOT(rolesChanged()));

    emit modelChanged(model);
}

void Collection::rolesChanged()
{
    resetInternalData();
    updateModel();
}

void Collection::emitCountChanged()
{
    emit countChanged(rowCount());
}

void Collection::updateModel()
{
    if (model() && m_comparator.isString()) {
        int role = model()->getRole(m_comparator.toString());
        setSortRole(role);
    }
}


bool Collection::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (m_filter.isCallable()) {
        QJSValue result = m_filter.call(QJSValueList()
                                        << model()->at(source_row)
                                        << source_row);
        return result.toBool();
    } else {
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    }
}

bool Collection::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    if (m_comparator.isCallable()) {
        QJSValue left = model()->at(source_left.row());
        QJSValue right = model()->at(source_right.row());
        QJSValue result = m_comparator.call(QJSValueList() << left << right);
        return result.toBool();
    } else {
        return QSortFilterProxyModel::lessThan(source_left, source_right);
    }
}

QJSValue Collection::at(int row) const
{
    QModelIndex m = index(row, 0);
    QModelIndex source = mapToSource(m);
    return model()->at(source.row());
}

void Collection::reSort()
{
    if (dynamicSortFilter())
    {
        // Workaround: If dynamic_sortfilter == true, sort(0) will not (always)
        // result in d->sort() being called, but setDynamicSortFilter(true) will.
        setDynamicSortFilter(true);
    }
    else
    {
        sort(0);
    }
}

} } }

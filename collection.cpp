#include "collection.h"
#include "jsonlistmodel.h"

Collection::Collection(QObject *parent) : QSortFilterProxyModel(parent)
{
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
        disconnect(oldModel, SIGNAL(roleAdded(QString)),
                   this, SLOT(resetInternalData()));
    }

    setSourceModel(model);
    updateModel();
    connect(model, SIGNAL(roleAdded(QString)), this, SLOT(roleAdded(QString)));

    emit modelChanged(model);
}

void Collection::roleAdded(QString)
{
    resetInternalData();
    updateModel();
}

void Collection::updateModel()
{
    if (model() && m_comparator.isString()) {
        int role = model()->getRole(m_comparator.toString());
        setSortRole(role);
    }
//    sort(0);
}


bool Collection::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (m_filter.isCallable()) {
        QJSValue result = m_filter.call(QJSValueList() << model()->at(source_row));
        return result.toBool();
    } else {
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    }
}

bool Collection::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    /*if (m_comparator.isString()) {
        QString compare = m_comparator.toString();
        QJSValue left = model()->at(source_left.row());
        QJSValue right = model()->at(source_right.row());

        // TODO: Avoid variant?
        QVariant leftVal = left.property(compare).toVariant();
        QVariant rightVal = right.property(compare).toVariant();
        return leftVal < rightVal;
    } else */
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
    JsonListModel *jsonModel = qobject_cast<JsonListModel*>(sourceModel());
    return jsonModel->at(source.row());
}


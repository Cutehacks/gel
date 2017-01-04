#ifndef COLLECTION_H
#define COLLECTION_H

#include <QtCore/QSortFilterProxyModel>
#include <QtQml/QJSValue>

#include "jsonlistmodel.h"

namespace com { namespace cutehacks { namespace gel {

class Collection : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QJSValue comparator READ comparator WRITE setComparator NOTIFY comparatorChanged)
    Q_PROPERTY(QJSValue filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(bool descendingSort READ descendingSort WRITE setDescendingSort NOTIFY descendingSortChanged)
    Q_PROPERTY(bool caseSensitiveSort READ caseSensitiveSort WRITE setCaseSensitiveSort NOTIFY caseSensitiveSortChanged)
    Q_PROPERTY(bool localeAwareSort READ localeAwareSort WRITE setLocaleAwareSort NOTIFY localeAwareSortChanged)
    Q_PROPERTY(com::cutehacks::gel::JsonListModel* model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    Collection(QObject *parent = 0);

    QJSValue comparator() const;
    QJSValue filter() const;
    JsonListModel *model() const;

    Q_INVOKABLE QJSValue at(int) const;

    Q_INVOKABLE void reSort();
    Q_INVOKABLE void reFilter();

    inline bool caseSensitiveSort() const
    {
        return sortCaseSensitivity() == Qt::CaseSensitive;
    }

    inline bool localeAwareSort() const
    {
        return isSortLocaleAware();
    }

    inline bool descendingSort() const
    {
        return sortOrder() == Qt::DescendingOrder;
    }

    inline int count() const { return rowCount(); }

public slots:
    void setComparator(QJSValue comparator);
    void setFilter(QJSValue filter);
    void setModel(JsonListModel* model);
    void setCaseSensitiveSort(bool caseSensitiveSort)
    {
        Qt::CaseSensitivity cs = caseSensitiveSort
                ? Qt::CaseSensitive
                : Qt::CaseInsensitive;

        if (cs == sortCaseSensitivity())
            return;

        setSortCaseSensitivity(cs);
        emit caseSensitiveSortChanged(caseSensitiveSort);
    }

    void setLocaleAwareSort(bool localeAwareSort)
    {
        if (localeAwareSort == isSortLocaleAware())
            return;

        setSortLocaleAware(localeAwareSort);
        emit localeAwareSortChanged(localeAwareSort);
    }

    void setDescendingSort(bool descendingSort)
    {
        if (descendingSort == (sortOrder() == Qt::DescendingOrder))
            return;

        sort(0, descendingSort ? Qt::DescendingOrder : Qt::AscendingOrder);
        emit descendingSortChanged(descendingSort);
    }

private slots:
    void rolesChanged();

signals:
    void comparatorChanged(QJSValue comparator);
    void filterChanged(QJSValue filter);
    void modelChanged(JsonListModel* model);
    void caseSensitiveSortChanged(bool caseSensitiveSort);
    void localeAwareSortChanged(bool localeAwareSort);
    void descendingSortChanged(bool descendingSort);
    void countChanged(int count);

private slots:
    void emitCountChanged();

protected:
    void updateModel();
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;

private:
    mutable QJSValue m_comparator;
    mutable QJSValue m_filter;
};

} } }

#endif // COLLECTION_H

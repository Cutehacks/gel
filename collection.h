#ifndef COLLECTION_H
#define COLLECTION_H

#include <QtCore/QSortFilterProxyModel>
#include <QtQml/QJSValue>

class JsonListModel;

class Collection : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(QJSValue comparator READ comparator WRITE setComparator NOTIFY comparatorChanged)
    Q_PROPERTY(QJSValue filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(bool descendingSort READ descendingSort WRITE setDescendingSort NOTIFY descendingSortChanged)
    Q_PROPERTY(bool caseSensitiveSort READ caseSensitiveSort WRITE setCaseSensitiveSort NOTIFY caseSensitiveSortChanged)
    Q_PROPERTY(bool localeAwareSort READ localeAwareSort WRITE setLocaleAwareSort NOTIFY localeAwareSortChanged)
    Q_PROPERTY(JsonListModel* model READ model WRITE setModel NOTIFY modelChanged)

public:
    Collection(QObject *parent = 0);

    QJSValue comparator() const;
    QJSValue filter() const;
    JsonListModel *model() const;

    Q_INVOKABLE QJSValue at(int) const;

    inline bool caseSensitiveSort() const
    {
        return sortCaseSensitivity() == Qt::CaseSensitive;
    }

    inline bool localeAwareSort() const
    {
        return isSortLocaleAware();
    }

    bool descendingSort() const
    {
        return sortOrder() == Qt::DescendingOrder;
    }

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
    void roleAdded(QString);

signals:
    void comparatorChanged(QJSValue comparator);
    void filterChanged(QJSValue filter);
    void modelChanged(JsonListModel* model);
    void caseSensitiveSortChanged(bool caseSensitiveSort);
    void localeAwareSortChanged(bool localeAwareSort);
    void descendingSortChanged(bool descendingSort);

protected:
    void updateModel();
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;

private:
    mutable QJSValue m_comparator;
    mutable QJSValue m_filter;
};

#endif // COLLECTION_H

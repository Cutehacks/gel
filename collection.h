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
    Q_PROPERTY(JsonListModel* model READ model WRITE setModel NOTIFY modelChanged)

public:
    Collection(QObject *parent = 0);

    QJSValue comparator() const;
    QJSValue filter() const;
    JsonListModel *model() const;

public slots:
    void setComparator(QJSValue comparator);
    void setFilter(QJSValue filter);
    void setModel(JsonListModel* model);

private slots:
    void roleAdded(QString);

signals:
    void comparatorChanged(QJSValue comparator);
    void filterChanged(QJSValue filter);
    void modelChanged(JsonListModel* model);

protected:
    void updateModel();
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;

private:
    mutable QJSValue m_comparator;
    mutable QJSValue m_filter;
};

#endif // COLLECTION_H

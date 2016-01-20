#ifndef JSONLISTMODEL_H
#define JSONLISTMODEL_H

#include <QtCore/QHash>
#include <QtCore/QAbstractItemModel>
#include <QtQml/QJSValue>

class QReadWriteLock;

class JsonListModel : public QAbstractItemModel
{
    Q_OBJECT

    Q_PROPERTY(QString idAttribute READ idAttribute WRITE setIdAttribute NOTIFY idAttributeChanged)

public:
    JsonListModel(QObject *parent = 0);

    Q_INVOKABLE void add(const QJSValue&);
    Q_INVOKABLE void remove(const QJSValue&);
    Q_INVOKABLE void clear();
    Q_INVOKABLE QJSValue at(int) const;

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QHash<int, QByteArray> roleNames() const;
    QString idAttribute() const;
    int getRole(const QString&) const;

protected:
    int addItem(const QJSValue &item);
    void addRole(const QString&);
    QString getRole(int role) const;

    void extractRoles(const QJSValue &item, const QString&);

public slots:
    void setIdAttribute(QString idAttribute);

signals:
    void idAttributeChanged(QString idAttribute);
    void roleAdded(const QString&);

private:
    mutable QReadWriteLock *m_lock;
    QHash<QString, QJSValue> m_items;
    QList<QString> m_keys;
    QList<QString> m_roles;
    QString m_idAttribute;
};

#endif // JSONLISTMODEL_H

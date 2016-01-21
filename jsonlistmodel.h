#ifndef JSONLISTMODEL_H
#define JSONLISTMODEL_H

#include <QtCore/QHash>
#include <QtCore/QSet>
#include <QtCore/QAbstractItemModel>
#include <QtQml/QJSValue>

class QReadWriteLock;

class JsonListModel : public QAbstractItemModel
{
    Q_OBJECT

    Q_PROPERTY(QString idAttribute READ idAttribute WRITE setIdAttribute NOTIFY idAttributeChanged)
    Q_PROPERTY(bool dynamicRoles READ dynamicRoles WRITE setDynamicRoles NOTIFY dynamicRolesChanged)
    Q_PROPERTY(QJSValue attachedProperties READ attachedProperties WRITE setAttachedProperties NOTIFY attachedPropertiesChanged)

public:
    JsonListModel(QObject *parent = 0);

    Q_INVOKABLE void add(const QJSValue&);
    Q_INVOKABLE void remove(const QJSValue&);
    Q_INVOKABLE void clear();
    Q_INVOKABLE QJSValue at(int) const;
    Q_INVOKABLE QJSValue get(const QJSValue&) const;

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QHash<int, QByteArray> roleNames() const;
    QString idAttribute() const;
    int getRole(const QString&) const;
    bool dynamicRoles() const;
    void setDynamicRoles(bool dynamicRoles);
    QJSValue attachedProperties() const;

protected:
    int addItem(const QJSValue &item);
    QString getRole(int role) const;
    bool extractRoles(const QJSValue &item, const QString&);

public slots:
    void setIdAttribute(QString idAttribute);
    void setAttachedProperties(QJSValue attachedProperties);

signals:
    void idAttributeChanged(QString idAttribute);
    void rolesChanged();
    void dynamicRolesChanged();
    void attachedPropertiesChanged(QJSValue attachedProperties);

private:
    bool addRole(const QString &string);

    mutable QReadWriteLock *m_lock;
    QHash<QString, QJSValue> m_items;
    QList<QString> m_keys;
    QSet<QString> m_roleSet;
    QList<QString> m_roles;
    QString m_idAttribute;
    bool m_dynamicRoles;
    QJSValue m_attachedProperties;
};

#endif // JSONLISTMODEL_H

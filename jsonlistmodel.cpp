#include <QDebug>
#include <QtCore/QReadWriteLock>
#include <QtQml/qqml.h>
#include <QtQml/QQmlEngine>
#include "jsonlistmodel.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
#include "jsvalueiterator.h"
#else
#include <QtQml/QJSValueIterator>
typedef QJSValueIterator JSValueIterator;
#endif

static const int BASE_ROLE = Qt::UserRole + 1;

JsonListModel::JsonListModel(QObject *parent) :
    QAbstractItemModel(parent),
    m_lock(new QReadWriteLock(QReadWriteLock::Recursive)),
    m_idAttribute("id"),
    m_dynamicRoles(false)
{
}

bool JsonListModel::dynamicRoles() const
{
    return m_dynamicRoles;
}

void JsonListModel::setDynamicRoles(bool dynamicRoles)
{
    if (dynamicRoles == m_dynamicRoles)
        return;
    m_dynamicRoles = dynamicRoles;
    emit dynamicRolesChanged();
}

QJSValue JsonListModel::attachedProperties() const
{
    return m_attachedProperties;
}

bool JsonListModel::addRole(const QString &role)
{
    // QML's views seem to freak out if the role order changes
    // so use a set to track which roles are already added and a list
    // to maintain the correct order

    if (m_roleSet.constFind(role) != m_roleSet.constEnd())
        return false;

    m_roleSet.insert(role);
    m_roles << role;

    return true;
}

QJSValue JsonListModel::clone(QQmlEngine *engine, const QJSValue &src) const
{
    if (!src.isObject())
        return src;

    QJSValue dst = engine->newObject();

    JSValueIterator it(src);
    while (it.next()) {
        QJSValue v = it.value();
        if (v.isObject())
            v = clone(engine, v);
        dst.setProperty(it.name(), v);
    }
    return dst;
}

bool JsonListModel::extractRoles(const QJSValue &item,
                                 const QString &prefix = QString())
{
    bool rolesAdded = false;

    if (prefix.isNull() && (item.isString() || item.isNumber() || item.isDate())) {
        if (addRole("modelData"))
            rolesAdded = true;
    } else {
        JSValueIterator it(item);
        while (it.next()) {
            QString n = prefix + it.name();
            if (addRole(n))
                rolesAdded = true;
            QJSValue v = it.value();
            if (!v.isArray() && v.isObject() && extractRoles(it.value(), n + "."))
                rolesAdded = true;
        }
    }

    return rolesAdded;
}

int JsonListModel::addItem(const QJSValue &item)
{
    int row = -1;

    QString id;
    if (item.isString() || item.isNumber() || item.isDate()) {
        id = item.toString();
    } else if (item.isObject()) {
        if (!item.hasProperty(m_idAttribute)) {
            qWarning() << QString("Object does not have a %1 property").arg(m_idAttribute);
            return row;
        }
        id = item.property(m_idAttribute).toString();
    }

    QJSValue existingItem = m_items[id];

    if (existingItem.strictlyEquals(item)) {
        return row;
    }

    m_items[id] = item;
    if (existingItem.isUndefined()) {
        m_keys.append(id);
        row = m_keys.count() - 1;
        return row;
    }

    return m_keys.indexOf(id);
}

void JsonListModel::add(const QJSValue &item)
{
    m_lock->lockForWrite();
    int originalSize = m_keys.count();
    if (item.isArray()) {
        JSValueIterator array(item);
        int updateFrom = INT_MAX;
        int updateTo = INT_MIN;

        bool rolesAdded = false;
        bool isFirstItem = true;
        while (array.next()) {
            if (!array.hasNext())
                break; // last value in array is an int with the length
            if ((m_dynamicRoles || isFirstItem) && extractRoles(array.value()))
                rolesAdded = true;
            int row = addItem(array.value());
            if (row >= 0 && row < originalSize) {
                updateFrom = qMin(updateFrom, row);
                updateTo = qMax(updateTo, row);
            }
            isFirstItem = false;
        }

        if (rolesAdded) {
            // this implies a model reset
            m_lock->unlock();
            emit rolesChanged();
            beginResetModel();
            endResetModel();
        } else {
            int newSize = m_keys.count();
            m_lock->unlock();

            // emit signals after the mutex is unlocked
            if (newSize > originalSize) {
                beginInsertRows(QModelIndex(), originalSize, newSize - 1);
                endInsertRows();
            } else {
                if (updateFrom != INT_MAX && updateTo != INT_MIN) {
                    emit dataChanged(createIndex(updateFrom, 0), createIndex(updateTo, 0));
                }
            }
        }
    } else {
        bool rolesAdded = extractRoles(item);
        int row = addItem(item);

        if (rolesAdded) {
            // this implies a model reset
            m_lock->unlock();
            emit rolesChanged();
            beginResetModel();
            endResetModel();
            return;
        }

        if (row >= 0 && row < originalSize) {
            QModelIndex index = createIndex(row, 0);
            m_lock->unlock();

            emit dataChanged(index, index);
            return;
        }

        int newSize = m_keys.count();
        m_lock->unlock();

        if (newSize > originalSize) {
            beginInsertRows(QModelIndex(), originalSize, newSize - 1);
            endInsertRows();
        }
    }
}

void JsonListModel::remove(const QJSValue &item)
{
    int index;
    {
        QString key;
        if (item.isString() || item.isNumber() || item.isDate()) {
            key = item.toString();
        } else if (item.hasProperty(m_idAttribute)){
            key = item.property(m_idAttribute).toString();
        } else {
            qWarning() << "Unable to remove item";
            return;
        }

        QWriteLocker writeLocker(m_lock);
        index = m_keys.indexOf(key);

        if (index == -1)
            return;

        m_keys.removeAt(index);
        m_items.remove(key);
    }
    beginRemoveRows(QModelIndex(), index, index);
    endRemoveRows();
}

void JsonListModel::clear()
{
    m_lock->lockForWrite();
    int originalSize = m_keys.length();
    m_keys.clear();
    m_items.clear();
    m_lock->unlock();

   beginRemoveRows(QModelIndex(), 0, originalSize);
   endRemoveRows();
}

QJSValue JsonListModel::at(int row) const
{
    QReadLocker locker(m_lock);
    if (row >= 0 && row < m_keys.count()) {
        return m_items[m_keys[row]];
    }
    return QJSValue();
}

QJSValue JsonListModel::get(const QJSValue &id) const
{
    QString key = id.toString();
    QReadLocker readLock(m_lock);
    return m_items[key];
}

QJSValue JsonListModel::asArray(bool deepCopy) const
{
    QQmlEngine *engine = qmlEngine(this);
    QReadLocker readLock(m_lock);
    QJSValue array = engine->newArray(m_keys.length());
    QList<QString>::const_iterator it = m_keys.cbegin();
    int i = 0;
    if (deepCopy) {
        while (it != m_keys.cend())
            array.setProperty(i++, clone(engine, m_items.value(*it++)));
    } else {
        while (it != m_keys.cend())
            array.setProperty(i++, m_items.value(*it++));
    }
    return array;
}

QModelIndex JsonListModel::index(int row, int column, const QModelIndex &) const
{
    QReadLocker readLock(m_lock);
    if (row >= 0 && row < m_keys.count()) {
        return createIndex(row, column);
    } else {
        qWarning() << "Out of bounds";
    }
    return QModelIndex();
}

QModelIndex JsonListModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

int JsonListModel::rowCount(const QModelIndex &) const
{
    QReadLocker locker(m_lock);
    return m_keys.count();
}

int JsonListModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QVariant JsonListModel::data(const QModelIndex &index, int role) const
{
    QReadLocker readLock(m_lock);
    int row = index.row();
    if (row < 0 || row > m_keys.count()) {
        qWarning() << "Out of bounds";
        return QVariant();
    }

    QJSValue item = m_items[m_keys[row]];
    QString roleName = getRole(role);

    if (item.isString() || item.isNumber() || item.isDate()) {
        return item.toVariant();
    } else {
        QStringList parts = roleName.split(".");
        roleName = parts.takeLast();
        for (QStringList::const_iterator p = parts.constBegin(); p != parts.end(); p++) {
            item = item.property(*p);
        }
        if (item.hasProperty(roleName))
            return item.property(roleName).toVariant();
        else if (m_attachedProperties.hasProperty(roleName)) {
            QJSValue generatedProp = m_attachedProperties.property(roleName);
            QJSValue generatedResult = generatedProp;
            if (generatedProp.isCallable())
                generatedResult = generatedProp.call(QJSValueList() << item << row);
            return generatedResult.toVariant();
        }
        return QJSValue().toVariant();
    }
}

QHash<int, QByteArray> JsonListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    for (int i = 0; i < m_roles.count(); ++i) {
        roles.insert(BASE_ROLE + i, m_roles.at(i).toUtf8());
    }
    return roles;
}

QString JsonListModel::idAttribute() const
{
    return m_idAttribute;
}

QString JsonListModel::getRole(int role) const
{
    role -= BASE_ROLE;
    if (role < 0 || role > m_roles.count())
        return "";
    return m_roles[role];
}

int JsonListModel::getRole(const QString &role) const
{
    int roleIndex = m_roles.indexOf(role);
    if (roleIndex < 0)
        return Qt::DisplayRole;
    return BASE_ROLE + roleIndex;
}

void JsonListModel::setIdAttribute(QString idAttribute)
{
    if (m_idAttribute == idAttribute)
        return;

    m_idAttribute = idAttribute;
    emit idAttributeChanged(idAttribute);
}

void JsonListModel::setAttachedProperties(QJSValue attachedProperties)
{
    if (m_attachedProperties.strictlyEquals(attachedProperties))
        return;

    m_attachedProperties = attachedProperties;

    JSValueIterator it(m_attachedProperties);
    while (it.next())
        addRole(it.name());

    emit attachedPropertiesChanged(attachedProperties);
}

bool JsonListModel::setData(const QModelIndex &, const QVariant &, int)
{
    return false;
}


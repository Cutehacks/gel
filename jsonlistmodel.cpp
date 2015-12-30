#include <QDebug>
#include <QtCore/QReadWriteLock>
#include <QtQml/QJSValueIterator>
#include "jsonlistmodel.h"

static const int BASE_ROLE = Qt::UserRole + 1;

JsonListModel::JsonListModel(QObject *parent) :
    QAbstractItemModel(parent),
    m_lock(new QReadWriteLock(QReadWriteLock::Recursive)),
    m_idAttribute("id")
{
}

void JsonListModel::extractRoles(const QJSValue &item)
{
    QJSValueIterator it(item);
    while (it.next()) {
        QString n = it.name();
        addRole(n);
    }
}

int JsonListModel::addItem(const QJSValue &item)
{
    int row = -1;

    QString id;
    if (item.isString() || item.isNumber() || item.isDate()) {
        id = item.toString();
        addRole("modelData");
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
        QJSValueIterator array(item);
        int updateFrom = INT_MAX;
        int updateTo = INT_MIN;

        // Only extract roles from the first item (assumes uniform data)
        if (array.next()) {
            if (array.hasNext()) { // ignore if last element
                extractRoles(array.value());
                int row = addItem(array.value());
                if (row >= 0 && row < originalSize) {
                    updateFrom = qMin(updateFrom, row);
                    updateTo = qMax(updateTo, row);
                }
            }
        }
        while (array.next()) {
            if (!array.hasNext())
                break; // last value in array is an int with the length
            int row = addItem(array.value());
            if (row >= 0 && row < originalSize) {
                updateFrom = qMin(updateFrom, row);
                updateTo = qMax(updateTo, row);
            }
        }
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
    } else {
        extractRoles(item);
        int row = addItem(item);

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
        QWriteLocker writeLocker(m_lock);
        QString key;
        if (item.isString() || item.isNumber() || item.isDate()) {
            key = item.toString();
            index = m_keys.indexOf(key);
        } else if (item.hasProperty(m_idAttribute)){
            key = item.property(m_idAttribute).toString();
            index = m_keys.indexOf(key);
        } else {
            qWarning() << "Unable to remove item";
            return;
        }

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
    m_roles.clear();
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
    if (item.isString() || item.isNumber() || item.isDate())
        return item.toVariant();
    else if (item.hasProperty(roleName))
        return item.property(roleName).toVariant();
    return QJSValue().toVariant();
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

void JsonListModel::addRole(const QString &r)
{
    foreach (const QString s, m_roles) {
        if (s == r)
            return;
    }
    m_roles.append(r);
    emit roleAdded(r);
    // TODO: emit dataChanged(); ?
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

bool JsonListModel::setData(const QModelIndex &, const QVariant &, int)
{
    return false;
}


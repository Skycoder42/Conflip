#include "settingsfilemodel.h"

#include <QDebug>

SettingsFileModel::SettingsFileModel(SettingsFile *settingsFile, QObject *parent) :
	QAbstractItemModel(parent),
	_settings(settingsFile),
	_root(new SettingsItem(QString(), nullptr))
{}

QVariant SettingsFileModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
		switch (section) {
		case 0:
			return tr("Keys");
		default:
			return QVariant();
		}
	} else
		return QVariant();
}

QModelIndex SettingsFileModel::index(int row, int column, const QModelIndex &parent) const
{
	auto item = getItem(parent);
	if(row < 0 || column < 0 ||
	   row >= item->children.size() || column != 0)
		return QModelIndex();

	return createIndex(row, column, item->children[row]);
}

QModelIndex SettingsFileModel::parent(const QModelIndex &index) const
{
	auto item = getItem(index);
	if(item->parent) {
		qDebug() << "parent from" << item->key << "to" << getItem(itemIndex(item->parent))->key;
		return itemIndex(item->parent);
	}

	return QModelIndex();
}

int SettingsFileModel::rowCount(const QModelIndex &parent) const
{
	auto item = getItem(parent);
	return item->children.size();
}

int SettingsFileModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return 1;
}

bool SettingsFileModel::hasChildren(const QModelIndex &parent) const
{
	return _settings->hasChildren(getItem(parent)->keyChain());
}

bool SettingsFileModel::canFetchMore(const QModelIndex &parent) const
{
	auto item = getItem(parent);
	return item->children.isEmpty() && hasChildren(parent);
}

void SettingsFileModel::fetchMore(const QModelIndex &parent)
{
	auto item = getItem(parent);
	QHash<QString, SettingsItem*> childCache;

	foreach(auto group, _settings->childGroups(item->keyChain())) {
		auto child = new SettingsItem(group, item);
		childCache.insert(group, child);
	}

	foreach (auto key, _settings->childKeys(item->keyChain())) {
		auto child = childCache.value(key);
		if(!child) {
			child = new SettingsItem(key, item);
			childCache.insert(key, child);
		}
		child->isKey = true;
	}

	beginInsertRows(parent, 0, childCache.size() - 1);
	item->children = childCache.values();
	endInsertRows();
}

QVariant SettingsFileModel::data(const QModelIndex &index, int role) const
{
	auto item = getItem(index);
	if(index.column() == 0 && role == Qt::DisplayRole)
		return item->key;
	else
		return QVariant();
}

SettingsFileModel::SettingsItem *SettingsFileModel::getItem(const QModelIndex &index) const
{
	if(!index.isValid())
		return _root.data();
	else
		return static_cast<SettingsItem*>(index.internalPointer());
}

QModelIndex SettingsFileModel::itemIndex(const SettingsItem *item) const
{
	if(item->parent) {
		auto row = item->parent->children.indexOf((SettingsItem*)item);
		if(row != -1)
			return createIndex(row, 0, (void*)item);
	}

	return QModelIndex();
}



SettingsFileModel::SettingsItem::SettingsItem(const QString &key, SettingsItem *parent) :
	key(key),
	isKey(false),
	synced(false),
	recursive(false),
	parent(parent),
	children()
{}

SettingsFileModel::SettingsItem::~SettingsItem()
{
	qDeleteAll(children);
}

QStringList SettingsFileModel::SettingsItem::keyChain() const
{
	if(parent)
		return parent->keyChain() << key;
	else
		return QStringList();
}

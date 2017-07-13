#include "settingsfilemodel.h"

#include <QDebug>
#include <QGuiApplication>
#include <QFont>
#include <QColor>

SettingsFileModel::SettingsFileModel(SettingsFile *settingsFile, QObject *parent) :
	QAbstractItemModel(parent),
	_settings(settingsFile),
	_store(new DataStore()),
	_root(new SettingsItem(QString(), nullptr))
{}

void SettingsFileModel::enablePreview(bool enable)
{
	if(_preview != enable) {
		_preview = enable;
		if(!_root->children.isEmpty()) {
			emit dataChanged(this->index(0, 1),
							 this->index(_root->children.size() - 1, 1));
		}
	}
}

QList<QPair<QStringList, bool> > SettingsFileModel::extractEntries() const
{
	QList<QPair<QStringList, bool>> entries;
	_root->addEntries(entries);
	return entries;
}

void SettingsFileModel::initialize(SettingsObject object)
{
	foreach(auto entry, object.entries) {
		auto item = loadTree(entry.keyChain, _root.data());
		item->synced = true;
		item->recursive = entry.recursive;
		auto index = itemIndex(item);
		emit dataChanged(index, index);
	}
}

QVariant SettingsFileModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
		switch (section) {
		case 0:
			return tr("Keys");
		case 1:
			return tr("Data Preview");
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
	   row >= item->children.size() || column >= 2)
		return QModelIndex();

	return createIndex(row, column, item->children[row]);
}

QModelIndex SettingsFileModel::parent(const QModelIndex &index) const
{
	auto item = getItem(index);
	if(item->parent)
		return itemIndex(item->parent);

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
	return 2;
}

bool SettingsFileModel::hasChildren(const QModelIndex &parent) const
{
	auto item = getItem(parent);
	if(item->children.isEmpty())
		return _settings->hasChildren(item->keyChain());
	else
		return true;
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

	switch (role) {
	case Qt::DisplayRole:
		switch (index.column()) {
		case 0:
			return item->key;
		case 1:
			if(_preview) {
				if(item->isKey)
					return _settings->value(item->keyChain());
				else
					return tr("<none>");
			}
		default:
			break;
		}
		break;
	case Qt::CheckStateRole:
		if(index.column() == 0) {
			if(item->recursive)
				return Qt::Checked;
			else if(item->synced)
				return Qt::PartiallyChecked;
			else
				return Qt::Unchecked;
		}
		break;
	case Qt::FontRole:
		if(index.column() == 1 && !item->isKey) {
			auto font = QGuiApplication::font();
			font.setItalic(true);
			return font;
		}
		break;
	case Qt::ForegroundRole:
		if(index.column() == 1 && !item->isKey)
			return QColor(Qt::lightGray);
		break;
	default:
		break;
	}

	return QVariant();
}

bool SettingsFileModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if(role != Qt::CheckStateRole || index.column() != 0)
		return false;
	auto item = getItem(index);
	switch ((Qt::CheckState)value.toInt()) {
	case Qt::Unchecked:
		item->synced = false;
		item->recursive = false;
		break;
	case Qt::PartiallyChecked:
		if(item->isKey) {
			item->synced = true;
			item->recursive = false;
		} else {
			item->synced = true;
			item->recursive = true;
		}
		break;
	case Qt::Checked:
		item->synced = true;
		item->recursive = true;
		break;
	default:
		Q_UNREACHABLE();
		break;
	}
	//update item
	emit dataChanged(index, index);
	//update all children
	emit dataChanged(this->index(0, 0, index),
					 this->index(item->children.size() - 1, columnCount() - 1, index));
	return true;
}

Qt::ItemFlags SettingsFileModel::flags(const QModelIndex &index) const
{
	auto item = getItem(index);
	if(item == _root.data())
		return Qt::NoItemFlags;

	Qt::ItemFlags flags = Qt::ItemIsSelectable;
	if(index.column() == 0)
		flags |= Qt::ItemIsTristate;

	if(!item->recurseOut()) {
		flags |= Qt::ItemIsEnabled;
		if(index.column() == 0)
			flags |= Qt::ItemIsUserTristate | Qt::ItemIsUserCheckable;
	}

	return flags;
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

SettingsFileModel::SettingsItem *SettingsFileModel::loadTree(QStringList keyTree, SettingsItem *parent)
{
	if(keyTree.isEmpty())
		return parent;

	auto index = itemIndex(parent);
	auto mKey = keyTree.takeFirst();
	if(parent->children.isEmpty())
		fetchMore(index);

	foreach(auto child, parent->children) {
		if(child->key == mKey)
			return loadTree(keyTree, child);
	}

	auto child = new SettingsItem(mKey, parent);
	child->isKey = true;
	beginInsertRows(index, parent->children.size(), parent->children.size());
	parent->children.append(child);
	endInsertRows();
	return loadTree(keyTree, child);
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

bool SettingsFileModel::SettingsItem::recurseOut() const
{
	if(!parent)
		return false;
	else if(parent->recursive)
		return true;
	else
		return parent->recurseOut();
}

void SettingsFileModel::SettingsItem::addEntries(QList<QPair<QStringList, bool> > &entries, const QStringList &baseChain) const
{
	foreach(auto child, children) {
		auto childChain = baseChain;
		childChain.append(child->key);
		if(child->synced) {
			entries.append({childChain, child->recursive});
			if(child->recursive)
				continue;
		}
		child->addEntries(entries, childChain);
	}
}

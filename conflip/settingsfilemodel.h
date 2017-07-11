#ifndef SETTINGSFILEMODEL_H
#define SETTINGSFILEMODEL_H

#include <QAbstractItemModel>
#include <QScopedPointer>
#include "settingsfile.h"

class SettingsFileModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	explicit SettingsFileModel(SettingsFile *settingsFile, QObject *parent = nullptr);

	// Header:
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	// Basic functionality:
	QModelIndex index(int row, int column,
					  const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex &index) const override;

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	// Fetch data dynamically:
	bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;
	bool canFetchMore(const QModelIndex &parent) const override;
	void fetchMore(const QModelIndex &parent) override;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
	class SettingsItem
	{
	public:
		SettingsItem(const QString &key, SettingsItem *parent);
		~SettingsItem();

		const QString key;
		QStringList keyChain() const;
		bool isKey;
		bool synced;
		bool recursive;

		const SettingsItem *parent;
		QList<SettingsItem*> children;
	};

	SettingsFile *_settings;
	QScopedPointer<SettingsItem> _root;

	SettingsItem *getItem(const QModelIndex &index) const;
	QModelIndex itemIndex(const SettingsItem *item) const;
};

#endif // SETTINGSFILEMODEL_H

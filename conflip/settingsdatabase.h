#ifndef SETTINGSDATABASE_H
#define SETTINGSDATABASE_H

#include <QJsonSerializer>
#include <QObject>
#include <QUuid>
#include <QVariant>
#include <QtDataSync/DataMerger>

class SettingsEntry
{
	Q_GADGET

	Q_PROPERTY(QStringList keyChain MEMBER keyChain)
	Q_PROPERTY(bool recursive MEMBER recursive)

public:
	QStringList keyChain;
	bool recursive;

	bool operator ==(const SettingsEntry &other) const;
	bool operator !=(const SettingsEntry &other) const;
};

class SettingsValue
{
	Q_GADGET

	Q_PROPERTY(QVariant value MEMBER value)
	Q_PROPERTY(QHash<QString, SettingsValue> children MEMBER children)

public:
	SettingsValue(const QVariant &data = {});

	QVariant value;
	QHash<QString, SettingsValue> children;

	bool operator ==(const SettingsValue &other) const;
	bool operator !=(const SettingsValue &other) const;
};

class SettingsObject
{
	Q_GADGET

	Q_PROPERTY(QUuid id MEMBER id USER true)
	Q_PROPERTY(QString type MEMBER type)
	Q_PROPERTY(QHash<QUuid, QString> paths MEMBER paths)
	Q_PROPERTY(QList<SettingsEntry> entries MEMBER entries)
	Q_PROPERTY(SettingsValue data MEMBER data)

public:
	QUuid id;
	QString type;
	QHash<QUuid, QString> paths;
	QList<SettingsEntry> entries;
	SettingsValue data;
};

class SettingsObjectMerger : public QtDataSync::DataMerger
{
	Q_OBJECT

public:
	explicit SettingsObjectMerger(QObject *parent = nullptr);

	QJsonObject merge(QJsonObject local, QJsonObject remote) override;

private:
	QJsonSerializer *_serializer;

	SettingsValue mergeValue(SettingsValue lo, SettingsValue ro5);
};

#endif // SETTINGSDATABASE_H

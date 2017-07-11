#ifndef SETTINGSDATABASE_H
#define SETTINGSDATABASE_H

#include <QJsonSerializer>
#include <QObject>
#include <QUuid>
#include <QVariant>
#include <QtDataSync/DataMerger>

class SettingsEntry : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QStringList keyChain MEMBER keyChain)
	Q_PROPERTY(bool recursive MEMBER recursive)

public:
	SettingsEntry(QObject *parent = nullptr);

	QStringList keyChain;
	bool recursive;
};

class SettingsValue : public QObject
{
	Q_OBJECT

	friend class SettingsObjectMerger;

	Q_PROPERTY(QVariant value MEMBER value)
	Q_PROPERTY(QHash<QString, SettingsValue*> children MEMBER children)

public:
	SettingsValue(QObject *parent = nullptr, const QVariant &data = {});

	QVariant value;
	QHash<QString, SettingsValue*> children;
};

class SettingsObject : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QUuid id MEMBER id USER true)
	Q_PROPERTY(QString type MEMBER type)
	Q_PROPERTY(QHash<QUuid, QString> paths MEMBER paths)
	Q_PROPERTY(QList<SettingsEntry*> entries MEMBER entries)
	Q_PROPERTY(SettingsValue* data MEMBER data)

public:
	SettingsObject(QObject *parent = nullptr);

	QUuid id;
	QString type;
	QHash<QUuid, QString> paths;
	QList<SettingsEntry*> entries;
	SettingsValue *data;
};

class SettingsObjectMerger : public QtDataSync::DataMerger
{
	Q_OBJECT

public:
	explicit SettingsObjectMerger(QObject *parent = nullptr);

	QJsonObject merge(QJsonObject local, QJsonObject remote) override;

private:
	QJsonSerializer *_serializer;

	void mergeValue(SettingsValue *lo, SettingsValue *ro);
};

#endif // SETTINGSDATABASE_H

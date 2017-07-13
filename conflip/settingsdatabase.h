#ifndef SETTINGSDATABASE_H
#define SETTINGSDATABASE_H

#include <QJsonSerializer>
#include <QObject>
#include <QUuid>
#include <QVariant>
#include <QtDataSync/DataMerger>

class SettingsValue
{
	Q_GADGET

	Q_PROPERTY(QString id READ id CONSTANT USER true)
	Q_PROPERTY(QUuid objectId MEMBER objectId)
	Q_PROPERTY(QStringList keyChain MEMBER keyChain)
	Q_PROPERTY(QStringList entryChain MEMBER entryChain)
	Q_PROPERTY(QString value READ getValue WRITE setValue)

public:
	SettingsValue();

	QUuid objectId;
	QStringList keyChain;
	QStringList entryChain;
	QVariant value;

	QString id() const;

	bool operator ==(const SettingsValue &other) const;
	bool operator !=(const SettingsValue &other) const;

private:
	QString getValue() const;
	void setValue(const QString &value);
};

class SettingsEntry
{
	Q_GADGET

	Q_PROPERTY(QStringList keyChain MEMBER keyChain)
	Q_PROPERTY(bool recursive MEMBER recursive)

public:
	SettingsEntry();

	QStringList keyChain;
	bool recursive;

	bool operator ==(const SettingsEntry &other) const;
	bool operator !=(const SettingsEntry &other) const;
};

class SettingsObject
{
	Q_GADGET

	Q_PROPERTY(QUuid id MEMBER id USER true)
	Q_PROPERTY(QString type MEMBER type)
	Q_PROPERTY(QMap<QString, QString> paths READ getPaths WRITE setPaths)
	Q_PROPERTY(bool syncAll MEMBER syncAll)
	Q_PROPERTY(QList<SettingsEntry> entries MEMBER entries)

public:
	SettingsObject();

	QUuid id;
	QString type;
	QHash<QUuid, QString> paths;
	bool syncAll;
	QList<SettingsEntry> entries;

	bool isValid() const;
	QString devicePath() const;

	bool operator ==(const SettingsObject &other) const;
	bool operator !=(const SettingsObject &other) const;

private:
	QMap<QString, QString> getPaths() const;
	void setPaths(const QMap<QString, QString> &value);
};

class SettingsObjectMerger : public QtDataSync::DataMerger
{
	Q_OBJECT

public:
	explicit SettingsObjectMerger(QObject *parent = nullptr);

	QJsonObject merge(QJsonObject local, QJsonObject remote) override;

private:
	QJsonSerializer *_serializer;
};

#endif // SETTINGSDATABASE_H

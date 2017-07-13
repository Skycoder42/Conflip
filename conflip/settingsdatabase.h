#ifndef SETTINGSDATABASE_H
#define SETTINGSDATABASE_H

#include <QJsonSerializer>
#include <QObject>
#include <QUuid>
#include <QVariant>
#include <QtDataSync/DataMerger>

class SettingsObject
{
	Q_GADGET

	Q_PROPERTY(QUuid id MEMBER id USER true)
	Q_PROPERTY(QString type MEMBER type)
	Q_PROPERTY(QMap<QString, QString> paths READ getPaths WRITE setPaths)
	Q_PROPERTY(bool syncAll MEMBER syncAll)
	Q_PROPERTY(QList<QUuid> entries READ getEntries WRITE setEntries)
	Q_PROPERTY(QList<QUuid> values READ getValues WRITE setValues)

public:
	SettingsObject();

	QUuid id;
	QString type;
	QHash<QUuid, QString> paths;
	bool syncAll;
	QSet<QUuid> entries;
	QSet<QUuid> values;

	bool isValid() const;
	QString devicePath() const;

	bool operator ==(const SettingsObject &other) const;
	bool operator !=(const SettingsObject &other) const;

private:
	QMap<QString, QString> getPaths() const;
	void setPaths(const QMap<QString, QString> &value);

	QList<QUuid> getEntries() const;
	void setEntries(const QList<QUuid> &value);

	QList<QUuid> getValues() const;
	void setValues(const QList<QUuid> &value);
};

class SettingsEntry
{
	Q_GADGET

	Q_PROPERTY(QUuid id READ id CONSTANT USER true)
	Q_PROPERTY(QUuid objectId MEMBER objectId)
	Q_PROPERTY(QStringList keyChain MEMBER keyChain)
	Q_PROPERTY(bool recursive MEMBER recursive)

public:
	SettingsEntry();

	QUuid objectId;
	QStringList keyChain;
	bool recursive;

	QUuid id() const;

	bool operator ==(const SettingsEntry &other) const;
	bool operator !=(const SettingsEntry &other) const;
};

class SettingsValue
{
	Q_GADGET

	Q_PROPERTY(QUuid id READ id CONSTANT USER true)
	Q_PROPERTY(QUuid objectId MEMBER objectId)
	Q_PROPERTY(QStringList keyChain MEMBER keyChain)
	Q_PROPERTY(QVariant value MEMBER value)

public:
	SettingsValue();

	QUuid objectId;
	QStringList keyChain;
	QVariant value;

	QUuid id() const;

	bool operator ==(const SettingsValue &other) const;
	bool operator !=(const SettingsValue &other) const;
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

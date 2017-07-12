#ifndef SETTINGSFILE_H
#define SETTINGSFILE_H

#include "libconflip_global.h"
#include <QObject>

class LIBCONFLIP_EXPORT SettingsFile : public QObject
{
	Q_OBJECT

public:
	explicit SettingsFile(QObject *parent = nullptr);

	virtual bool hasChildren(const QStringList &parentChain) = 0;
	virtual QStringList childGroups(const QStringList &parentChain) = 0;
	virtual QStringList childKeys(const QStringList &parentChain) = 0;
	virtual QVariant value(const QStringList &keyChain) = 0;
	virtual void setValue(const QStringList &keyChain, const QVariant &value) = 0;
};

#endif // SETTINGSFILE_H
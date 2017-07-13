#include "settingsfile.h"

SettingsFile::SettingsFile(QObject *parent) :
	QObject(parent)
{}

bool SettingsFile::hasChildren(const QStringList &parentChain)
{
	return !childGroups(parentChain).isEmpty() ||
			!childKeys(parentChain).isEmpty();
}

bool SettingsFile::isKey(const QStringList &keyChain)
{
	if(keyChain.isEmpty())
		return false;
	auto parentChain = keyChain;
	auto key = parentChain.takeLast();
	return childKeys(parentChain).contains(key);
}

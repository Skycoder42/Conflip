#include "fileplugin.h"
#include "filesettingsfile.h"

FilePlugin::FilePlugin(QObject *parent) :
	SettingsPlugin(parent)
{}

SettingsFile *FilePlugin::createSettings(const QString &path, const QString &type, QObject *parent)
{
	if(type == QStringLiteral("file"))
		return new FileSettingsFile(path, parent);
	else
		throw SettingsLoadException("Plugin must be used with the \"file\" format");
}

QString FilePlugin::displayName(const QString &type) const
{
	if(type == QStringLiteral("file"))
		return tr("Whole Files");
	else
		return QString();
}

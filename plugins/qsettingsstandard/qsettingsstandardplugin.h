#ifndef QSETTINGSSTANDARDPLUGIN_H
#define QSETTINGSSTANDARDPLUGIN_H

#include <qsettingsplugin.h>

class QSettingsStandardPlugin : public QSettingsPlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID SettingsPlugin_iid FILE "qsettingsstandard.json")

public:
	QSettingsStandardPlugin(QObject *parent = nullptr);

	QSettings::Format registerFormat(const QString &type) override;
	QString displayName(const QString &type) const override;
	QStringList fileFilters(const QString &type) const override;
};

#endif // QSETTINGSSTANDARDPLUGIN_H

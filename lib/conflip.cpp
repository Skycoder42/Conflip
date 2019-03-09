#include "conflip.h"
#include <QCoreApplication>
#include <QJsonSerializer>
#include <QLibraryInfo>
#include <QTranslator>
#include <QQueue>
#include <qpluginfactory.h>
#include "syncentry.h"
#include "synchelperplugin.h"
#include "settings.h"

namespace {

QSet<QString> loadedTranslations;

void conflip_lib_startup()
{
	QJsonSerializer::registerAllConverters<SyncEntry>();
}

}
Q_COREAPP_STARTUP_FUNCTION(conflip_lib_startup)

Q_GLOBAL_PLUGIN_OBJECT_FACTORY(SyncHelperPlugin, SyncHelper, "conflip", helperFactory)

QString Conflip::ConfigFileName()
{
	return QStringLiteral("config.json");
}

QStringList Conflip::listPlugins()
{
	return helperFactory->allKeys();
}

void Conflip::loadTranslations(const QString &type)
{
	if(!loadedTranslations.contains(type)) {
		auto plg = helperFactory->plugin(type);
		if(!plg)
			return;
		for(const auto &qmFile : plg->translations()) {
			auto translator = new QTranslator{qApp};
			if(translator->load(QLocale(),
								qmFile,
								QStringLiteral("_"),
								QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
				qApp->installTranslator(translator);
			}
		}
		loadedTranslations.insert(type);
	}
}

SyncHelper *Conflip::loadHelper(const QString &type, QObject *parent)
{
	return helperFactory->createInstance(type, parent);
}


bool Conflip::initConfDir()
{
	QDir pathDir{Settings::instance()->engine.dir};
	pathDir.makeAbsolute();
	if(!pathDir.exists()) {
		if(!pathDir.mkpath(QStringLiteral("."))) {
			qCritical().noquote() << "Failed to create config directory:"
								  << pathDir.absolutePath();
			return false;
		}
	}

	QFile confFile{pathDir.absoluteFilePath(Conflip::ConfigFileName())};
	if(!confFile.exists()) {
		if(!confFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
			qCritical().noquote() << "Failed to create config file:"
								  << confFile.fileName();
			return false;
		}
		confFile.write("{}\n"); //empty json object
		confFile.close();
	}

	return true;
}

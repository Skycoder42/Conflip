#ifndef SYNCENGINE_H
#define SYNCENGINE_H

#include <QObject>
#include <QTimer>
#include <QFileSystemWatcher>
#include <QJsonSerializer>
#include <QDir>
#include <synchelper.h>

#include "pathresolver.h"

class SyncEngine : public QObject
{
	Q_OBJECT

public:
	explicit SyncEngine(QObject *parent = nullptr);

	int start();

private slots:
	void triggerSync();

private:
	static const QString ConfigFileName;

	QTimer *_timer;
	QFileSystemWatcher *_watcher;
	QJsonSerializer *_serializer;
	PathResolver *_resolver;

	QHash<QString, SyncHelper*> _helpers;

	QDir _workingDir;
	bool _skipNextUpdate;

	SyncHelper *getHelper(const QString &type);
	void syncEntries(QList<SyncEntry> &entries, bool &changed);
	void removeUnsynced(QList<SyncEntry> &entries, bool &changed);
};

#endif // SYNCENGINE_H

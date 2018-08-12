#ifndef SYNCENGINE_H
#define SYNCENGINE_H

#include <QObject>
#include <QTimer>
#include <QFileSystemWatcher>
#include <QJsonSerializer>
#include <QDir>
#include <QThreadPool>
#include <synchelper.h>
#include <conflipdatabase.h>

#include "pathresolver.h"

class SyncEngine : public QObject
{
	Q_OBJECT

public:
	explicit SyncEngine(QObject *parent = nullptr);

	bool start();
	void pause();
	void resume();
	void reload();

private slots:
	void triggerSync();

	void syncDone(SyncTask *task, SyncTask::Result result);

private:
	QTimer *_timer;
	QFileSystemWatcher *_watcher;
	QJsonSerializer *_serializer;
	PathResolver *_resolver;
	QThreadPool *_threadPool;

	QHash<QString, SyncHelper*> _helpers;

	QDir _workingDir;
	bool _skipNextUpdate = false;

	ConflipDatabase _currentDb;
	bool _dbChanged = false;
	QHash<SyncTask*, SyncEntry*> _activeTasks;

	SyncHelper *getHelper(const QString &type);
	void syncEntries(QList<SyncEntry> &entries);
	void removeUnsynced(QList<SyncEntry> &entries);

	void completeSync();
};

#endif // SYNCENGINE_H

#ifndef SYNCENGINE_H
#define SYNCENGINE_H

#include <QObject>
#include <QTimer>
#include <QFileSystemWatcher>
#include <QJsonSerializer>
#include <QDir>

#include "pathresolver.h"
#include "synchelper.h"

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

	QHash<SyncEntry::PathMode, SyncHelper*> _helpers;

	QDir _workingDir;
	bool _skipNextUpdate;
};

#endif // SYNCENGINE_H

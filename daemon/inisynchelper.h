#ifndef INISYNCHELPER_H
#define INISYNCHELPER_H

#include <QObject>
#include "synchelper.h"

class IniSyncHelper : public QObject, public SyncHelper
{
	Q_OBJECT
	Q_INTERFACES(SyncHelper)

public:
	explicit IniSyncHelper(QObject *parent = nullptr);

public:
	void performSync(const QString &path, SyncEntry::PathMode mode, const QStringList &extras, bool isFirstUse) override;

private:
	using IniGroupMapping = QHash<QByteArray, QByteArray>;
	using IniEntryMapping = QHash<QByteArray, IniGroupMapping>;

	IniEntryMapping createMapping(const QFileInfo &file) const;
	void writeMapping(QIODevice *device, const IniEntryMapping &mapping, bool firstLine, bool &needSave) const;
	void writeMapping(const QFileInfo &file, const IniEntryMapping &mapping);
	bool shouldSync(const QByteArray &group, const QByteArray &key, const QByteArrayList &extras) const;

	void log(const QFileInfo &file, const char *msg, bool dbg = false) const;
	void log(const QFileInfo &file, const char *msg, const QByteArray &cGroup, const QByteArray &key, bool dbg = false) const;
};

#endif // INISYNCHELPER_H

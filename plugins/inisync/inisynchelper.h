#ifndef INISYNCHELPER_H
#define INISYNCHELPER_H

#include <QObject>
#include <synchelper.h>

class IniSyncHelper : public SyncHelper
{
	Q_OBJECT

public:
	static const QString ModeIni;

	explicit IniSyncHelper(QObject *parent = nullptr);

	bool pathIsPattern(const QString &mode) const override;
	void performSync(const QString &path, const QString &mode, const QStringList &extras, bool isFirstUse) override;
	void undoSync(const QString &path, const QString &mode) override;

private:
	using IniGroupMapping = QMap<QByteArray, QByteArray>;
	using IniEntryMapping = QMap<QByteArray, IniGroupMapping>;

	IniEntryMapping createMapping(const QFileInfo &file) const;
	void writeMapping(QIODevice *device, const IniEntryMapping &mapping, bool firstLine, bool &needSave, const QString &logStr = {}) const;
	void writeMapping(const QFileInfo &file, const IniEntryMapping &mapping);
	bool shouldSync(const QByteArray &group, const QByteArray &key, const QByteArrayList &extras) const;

	void log(const QFileInfo &file, const char *msg, bool dbg = false) const;
	void log(const QFileInfo &file, const char *msg, const QByteArray &cGroup, const QByteArray &key, bool dbg = false) const;
};

#endif // INISYNCHELPER_H

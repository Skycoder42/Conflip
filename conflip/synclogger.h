#ifndef SYNCLOGGER_H
#define SYNCLOGGER_H

#include <QFile>
#include <QObject>
#include <QTextStream>

class SyncLogger : public QObject
{
	Q_OBJECT

public:
	explicit SyncLogger(bool verbose, QObject *parent = nullptr);
	~SyncLogger();

	static void setup(bool verbose);
	static void openLogfile();

private:
	QFile *_logFile;
	QTextStream _stream;
	bool _verbose;

	static SyncLogger *_instance;
	static void handler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
};

#endif // SYNCLOGGER_H

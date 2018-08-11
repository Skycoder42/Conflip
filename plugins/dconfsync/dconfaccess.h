#ifndef DCONFREADER_H
#define DCONFREADER_H

#include <tuple>
#include <QByteArrayList>

using DConfClient = struct _DConfClient;

class DConfAccess
{
public:
	DConfAccess();
	~DConfAccess();

	void open(const QByteArray &path);

	QByteArrayList readAllKeys(const QByteArrayList &filters) const;
	std::tuple<QByteArray, QByteArray> readData(const QByteArray &key) const; //(type, value)
	bool writeData(const QByteArray &key, const QByteArray &type, const QByteArray &data, QByteArray *errorMsg = nullptr);

	void sync();

private:
	DConfClient *_client;
	QByteArray _path;
	bool _needsSync;

	QByteArrayList readAllKeysImpl(const QByteArray &base) const;
};

#endif // DCONFREADER_H

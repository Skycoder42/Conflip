#ifndef CONFLIPSERVICE_H
#define CONFLIPSERVICE_H

#include <QtService/Service>

#include "syncengine.h"

class ConflipService : public QtService::Service
{
	Q_OBJECT
public:
	explicit ConflipService(int &argc, char **argv);

	void setSlice(const QString &slice);

protected:
	CommandMode onStart() override;
	CommandMode onStop(int &exitCode) override;
	CommandMode onReload() override;
	CommandMode onPause() override;
	CommandMode onResume() override;

private:
	SyncEngine *_engine = nullptr;
};

#endif // CONFLIPSERVICE_H

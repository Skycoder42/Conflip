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
	CommandResult onStart() override;
	CommandResult onStop(int &exitCode) override;
	CommandResult onReload() override;
	CommandResult onPause() override;
	CommandResult onResume() override;

private:
	SyncEngine *_engine = nullptr;
};

#endif // CONFLIPSERVICE_H

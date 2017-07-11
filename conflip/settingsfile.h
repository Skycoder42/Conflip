#ifndef SETTINGSFILE_H
#define SETTINGSFILE_H

#include "libconflip_global.h"
#include <QObject>

class LIBCONFLIP_EXPORT SettingsFile : public QObject
{
	Q_OBJECT

public:
	explicit SettingsFile(QObject *parent = nullptr);
};

#endif // SETTINGSFILE_H

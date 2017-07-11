#include "genericplugin.h"

GenericPlugin::GenericPlugin(QObject *parent) :
	SettingsPlugin(parent)
{}

int GenericPlugin::baum() const
{
	return 42;
}

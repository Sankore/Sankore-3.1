#ifndef FREEZE_PLUGIN_H
#define FREEZE_PLUGIN_H

#include "PluginMaster.h"
class PluginMaster;
#include "IPlugin.h"
class IPlugin;
#include <QObject>

class FreezePlugin : public IPlugin
{
Q_OBJECT
public:
	FreezePlugin(PluginMaster* val);
	~FreezePlugin();
	void update(void* obj){}
private:
	int freezeFps;
public slots :
	void buttonListener();
};

#endif // FREEZE_PLUGIN_H

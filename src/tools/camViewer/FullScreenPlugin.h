#ifndef FULL_SCREEN_PLUGIN_H
#define FULL_SCREEN_PLUGIN_H

#include "PluginMaster.h"
class PluginMaster;
#include "IPlugin.h"
class IPlugin;
#include <QObject>

class FullScreenPlugin : public IPlugin
{
Q_OBJECT
public:
	FullScreenPlugin(PluginMaster* val);
	~FullScreenPlugin();
	void update(void* obj);
	bool observer;

private:
	bool fullScreen;
	std::string iconLock;
	void changeIcon();
public slots :
	void buttonListener();
	
};

#endif // FULL_SCREEN_PLUGIN_H
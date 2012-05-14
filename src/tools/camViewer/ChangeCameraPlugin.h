#ifndef CHANGE_CAMERA_PLUGIN_H
#define CHANGE_CAMERA_PLUGIN_H

#include "PluginMaster.h"
class PluginMaster;
#include "IPlugin.h"
class IPlugin;
#include <QObject>

class ChangeCameraPlugin : public IPlugin
{
Q_OBJECT
public:
	ChangeCameraPlugin(PluginMaster* val);
	~ChangeCameraPlugin();
	void update(void* obj){}
	int cameraIndex;
private:
	int freezeFps;
public slots :
	void buttonListener();
};

#endif // CHANGE_CAMERA_PLUGIN_H

#ifndef ZOOM_OUT_PLUGIN_H
#define ZOOM_OUT_PLUGIN_H

#include "PluginMaster.h"
class PluginMaster;
#include "IPlugin.h"
class IPlugin;
#include "ZoomInPlugin.h"
class ZoomInPlugin;
#include <QObject>

class ZoomOutPlugin : public IPlugin
{
Q_OBJECT
public:
	ZoomOutPlugin(PluginMaster* val, ZoomInPlugin* plug);
	~ZoomOutPlugin();
	void update(void* obj);
	int getZoomOutMax() { return this->nbZoomOutMax; }
private:
	ZoomInPlugin* myPlugin;
	int nbZoomOutMax;
	std::string iconLock;
public slots :
	void buttonListener();
	void changeIcon();
	
};

#endif // ZOOM_OUT_PLUGIN_H
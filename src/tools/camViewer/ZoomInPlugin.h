#ifndef ZOOM_IN_PLUGIN_H
#define ZOOM_IN_PLUGIN_H

#include "PluginMaster.h"
class PluginMaster;
#include "IPlugin.h"
class IPlugin;
#include "ZoomOutPlugin.h"
class ZoomOutPlugin;
#include <QObject>

class ZoomInPlugin : public IPlugin
{
Q_OBJECT
public:
	ZoomInPlugin(PluginMaster* val);
	~ZoomInPlugin();
	void update(void* obj);

	void setPlugin(ZoomOutPlugin* plug) { this->myPlugin = plug; }

	void addZoom();
	void subZoom();
	int getZoom() { return this->nbZoom; }

	void calculRegion();
private:
	ZoomOutPlugin* myPlugin;
	int nbZoomInMax;
	int nbZoom;
	CvRect* region;
	std::string iconLock;
public slots :
	void buttonListener();
	void changeIcon();
	
};

#endif // ZOOM_IN_PLUGIN_H
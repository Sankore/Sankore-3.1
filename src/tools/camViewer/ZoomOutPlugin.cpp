#include "ZoomOutPlugin.h"
#include <iostream>

ZoomOutPlugin::ZoomOutPlugin(PluginMaster* val, ZoomInPlugin* plug)
{ 
	this->myMaster = val;
	this->myPlugin = plug;
	this->myPlugin->setPlugin(this);
	this->name = "zoomOut";
	this->icon = "zoomOut.png";
	this->iconLock = "zoomOutLock.png";
	this->description = "zoom arrière sur l'image";
	this->observer = false;
	this->nbZoomOutMax = 1;
}
ZoomOutPlugin::~ZoomOutPlugin(){}
void ZoomOutPlugin::update(void* obj){}
void ZoomOutPlugin::buttonListener()
{
	std::cout << "Zoom arriere (" << this->myPlugin->getZoom() << ")" << std::endl;

	this->myPlugin->subZoom();
	this->myPlugin->changeIcon();
	changeIcon();

	this->myPlugin->calculRegion();

	if(this->myPlugin->getZoom() > 1)
		this->myMaster->changeCursor(Qt::CursorShape::OpenHandCursor);
	else
		this->myMaster->changeCursor(Qt::CursorShape::ArrowCursor);
}

void ZoomOutPlugin::changeIcon()
{
	/*
	if(this->myMaster->getZoom() != this->nbZoomOutMax)
	{
		QString iconeString = QString (":/images/");
		iconeString += this->iconLock.data();
		this->setIcon(QIcon(iconeString));
	}
	else
	{
		QString iconeString = QString (":/images/");
		iconeString += this->icon.data();
		this->setIcon(QIcon(iconeString));
	}
	*/
}
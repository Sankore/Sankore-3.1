#include "FullScreenPlugin.h"
#include <iostream>


FullScreenPlugin::FullScreenPlugin(PluginMaster* val) 
{ 
	this->myMaster = val;
	this->name = "fullscreen";
	this->icon = "fullscreen.png";
	this->iconLock = "fullscreen.png";
	this->description = "Plein ecran";
	this->fullScreen = false;
	this->observer = false;

	changeIcon();
}

FullScreenPlugin::~FullScreenPlugin(){}
void FullScreenPlugin::update(void* obj){}
void FullScreenPlugin::buttonListener()
{
	std::cout << "Fullscreen" << std::endl;

	this->fullScreen = !this->fullScreen;

	this->myMaster->setFullscreen(this->fullScreen);
}

void FullScreenPlugin::changeIcon()
{

}
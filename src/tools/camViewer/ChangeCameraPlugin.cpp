#include "ChangeCameraPlugin.h"
#include <iostream>
#include <string>

ChangeCameraPlugin::ChangeCameraPlugin(PluginMaster* val)
{ 
	this->myMaster = val;
	this->name = "changeCamera";
	this->icon = "camera.png";
	this->description = "bascule sur la caméra d'après.";
	this->observer = false;
	this->cameraIndex = 0;
}
ChangeCameraPlugin::~ChangeCameraPlugin(){}
void ChangeCameraPlugin::buttonListener()
{
	cameraIndex = this->myMaster->getListCamera();
}
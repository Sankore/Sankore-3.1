#include "FreezePlugin.h"
#include <iostream>
#include <string>

FreezePlugin::FreezePlugin(PluginMaster* val)
{ 
	this->myMaster = val;
	this->name = "freeze";
	this->icon = "pause.png";
	this->setCheckable(true);
	this->freezeFps = 25;
	this->description = "met la caméra en pause.";
	this->observer = false;
}
FreezePlugin::~FreezePlugin(){}
void FreezePlugin::buttonListener()
{
	if (this->isChecked())
	{
		this->freezeFps = this->myMaster->getFps();
		this->myMaster->setFps(0);
	}
	else
		this->myMaster->setFps(this->freezeFps);
}

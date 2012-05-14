#include "LeftRotatePlugin.h"
#include <iostream>
#include <string>

LeftRotatePlugin::LeftRotatePlugin(PluginMaster* val)
{ 
	this->myMaster = val;
	this->name = "leftRotate";
	this->icon = "left.png";
	this->description = "rotation de l'image sur la gauche. (90°)";
	//this->myRightRotate = RightRotatePlugin::getInstance();
	this->observer = true;
	this->angle = 0;
}
LeftRotatePlugin::~LeftRotatePlugin(){}
void LeftRotatePlugin::update(void* obj)
{
	this->myMaster->rotateImage(((IplImage*)obj), this->angle);
}
void LeftRotatePlugin::buttonListener()
{
	this->angle += 90; 
	this->myMaster->rotateImage( this->myMaster->getCurrentImage(), +90);
	this->myMaster->updateImage();
}

#include "RightRotatePlugin.h"
#include <iostream>
#include <string>

RightRotatePlugin::RightRotatePlugin(PluginMaster* val, LeftRotatePlugin* myRotate)
{ 
	this->myRotate = myRotate;
	this->myMaster = val;
	this->name = "rightRotate";
	this->icon = "right.png";
	this->description = "rotation de l'image sur la droite. (90°)";
	this->observer = false;

}
RightRotatePlugin::~RightRotatePlugin(){}
void RightRotatePlugin::update(void* obj){}
void RightRotatePlugin::buttonListener()
{
	this->myRotate->setAngle(this->myRotate->getAngle()-90);
	this->myMaster->rotateImage( this->myMaster->getCurrentImage(), -90);
	this->myMaster->updateImage();
}

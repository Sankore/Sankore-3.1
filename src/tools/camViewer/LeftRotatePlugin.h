#ifndef LEFT_ROTATE_PLUGIN_H
#define LEFT_ROTATE_PLUGIN_H

#include "PluginMaster.h"
class PluginMaster;
#include "IPlugin.h"
class IPlugin;
#include <QObject>
class RightRotatePlugin;

class LeftRotatePlugin : public IPlugin
{
Q_OBJECT
public:
	LeftRotatePlugin(PluginMaster* val);
	~LeftRotatePlugin();
	void update(void* obj);
	void setAngle(int angle){this->angle = angle;}
	int getAngle (){return this->angle;}
private:
	int angle;
public slots :
	void buttonListener();
};

#endif // LEFT_ROTATE_PLUGIN_H

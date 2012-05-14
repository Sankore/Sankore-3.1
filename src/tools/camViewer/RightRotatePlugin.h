#ifndef RIGHT_ROTATE_PLUGIN_H
#define RIGHT_ROTATE_PLUGIN_H

#include "PluginMaster.h"
class PluginMaster;
#include "IPlugin.h"
class IPlugin;
#include <QObject>

#include "LeftRotatePlugin.h"
class LeftRotatePlugin;

class RightRotatePlugin : public IPlugin
{
Q_OBJECT
public:
	RightRotatePlugin(PluginMaster* val, LeftRotatePlugin* myRotate );
	~RightRotatePlugin();
	void update(void* obj);
	
private:
	LeftRotatePlugin* myRotate;
public slots :
	void buttonListener();
};

#endif // RIGHT_ROTATE_PLUGIN_H

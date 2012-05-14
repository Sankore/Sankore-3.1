#ifndef INFO_PLUGIN_H
#define INFO_PLUGIN_H

#include "PluginMaster.h"
class PluginMaster;
#include "IPlugin.h"
class IPlugin;
#include <QObject>
#include <QWidget>
 #include <QTextEdit>

class InfoPlugin : public IPlugin
{
Q_OBJECT
public:
	InfoPlugin(PluginMaster* val);
	~InfoPlugin();
	void update(void* obj){}
private:
	QTextEdit* infoWidget;
public slots :
	void buttonListener();
};

#endif // INFO_PLUGIN_H

#ifndef I_PLUGIN_H
#define I_PLUGIN_H

#include <QPushButton>

#include "PluginMaster.h"


class IPlugin : public QPushButton
{
public:

	virtual void update(void* obj)=0;
	std::string name;
	std::string icon;
	std::string description;
	bool observer;
public slots :
	virtual void buttonListener()=0;
	
protected:
	PluginMaster* myMaster;
private:
};
#endif // I_PLUGIN_H
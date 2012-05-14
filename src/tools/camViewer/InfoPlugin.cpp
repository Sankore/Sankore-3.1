#include "InfoPlugin.h"
#include <iostream>
#include <string>

InfoPlugin::InfoPlugin(PluginMaster* val)
{ 
	this->myMaster = val;
	this->name = "info";
	this->icon = "aide.png";
	this->setCheckable(true);
	this->description = "Ouvre une bulle d'info. On trouvera des infos sur toutes les fonctionnalités de l'application.";	this->observer = false;
}
InfoPlugin::~InfoPlugin(){}
void InfoPlugin::buttonListener()
{
	if (!this->isChecked())
		this->infoWidget->~QTextEdit();
	else
	{
		this->infoWidget = new QTextEdit();
		this->infoWidget->setAcceptRichText (true);
		this->infoWidget->setReadOnly(true);
		QString strFinal = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\"><html><head><meta name=\"qrichtext\" content=\"1\" /><meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\" /></head><body> <style>p{font-size: 18px; }</style>");
		QString strIntro = QString("<h1> Visualiseur </h1><p>Le visualiseur à une multitude de fonctionnalités. Toutes ces fonctionnalités sont réparties dans les boutons au bord de l'interface.</p><h2> Les fonctionnalités :</h2><p>");
		
		strFinal.append(strIntro);
		
		std::vector<IPlugin*> listPlug = this->myMaster->getListPlugin();
		for (int i = 1; i < listPlug.size() ; i++)
		{	
			// style=\"background-image: \":/images/pause.jpg\"  \"
			//strContenu.append("");
			strFinal.append("<img src=\":/images/camViewer/");
			strFinal.append(QString(listPlug.at(i)->icon.data()));
			strFinal.append("\" width=25 height=25 /> : ");
			strFinal.append(QString::fromUtf8(listPlug.at(i)->description.data()));
			strFinal.append("<br/>");
			
		}
		strFinal.append(QString("</p></body></html>"));

		this->infoWidget->setHtml (strFinal);
		this->infoWidget->setGeometry(0,0,450,420);
		this->infoWidget->setStyleSheet("border-radius:20px;background-color :rgba(195, 195, 195, 200)");
		this->myMaster->addWidgetInFrame(this->infoWidget, 10, 10);
	}
}

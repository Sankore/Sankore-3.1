#include "ZoomInPlugin.h"
#include <iostream>


ZoomInPlugin::ZoomInPlugin(PluginMaster* val) 
{ 
	this->myMaster = val;
	this->nbZoom = 1;
	this->name = "zoomIn";
	this->icon = "zoomIn.png";
	this->iconLock = "zoomInLock.png";
	this->description = "zoom avant sur l'image";
	this->nbZoomInMax = 5;
	this->observer = true;
	this->region = NULL;
	
	changeIcon();
}

ZoomInPlugin::~ZoomInPlugin(){}
void ZoomInPlugin::update(void* obj)
{
	if(this->region == NULL)
	{
		this->region = (CvRect*)malloc(sizeof(CvRect));
		this->region->x = 0;
		this->region->y = 0;
		this->region->width = this->myMaster->getCurrentImage()->width;
		this->region->height = this->myMaster->getCurrentImage()->height;
	}

	this->calculRegion();
	this->myMaster->zoomImage(((IplImage*)obj), region);
}
void ZoomInPlugin::buttonListener()
{
	std::cout << "Zoom avant (" << this->nbZoom << ")" << std::endl;
	
	this->addZoom();
	this->myPlugin->changeIcon();
	changeIcon();

	this->calculRegion();

	if(this->nbZoom > 1)
		this->myMaster->changeCursor(Qt::CursorShape::OpenHandCursor);
	else
		this->myMaster->changeCursor(Qt::CursorShape::ArrowCursor);

	this->myMaster->zoomImage(this->myMaster->getCurrentImage(), region);

	this->myMaster->updateImage();
}

void ZoomInPlugin::calculRegion()
{
	//On recupere la taille de la fenetre
	std::pair<int, int> window = this->myMaster->getWindowSize();

	//On recupere la position de la souris
	std::pair<int, int> mouse = this->myMaster->getMousePosition();

	//On recupere la position précédente de la souris
	std::pair<int, int> mousePrevious = this->myMaster->getMousePositionPrevious();

	std::pair<int, int> delta;

	if(mousePrevious.first != 0 && mousePrevious.second != 0)
	{
		//sensibilité du déplacement de l'image (réduire cette valeur, augmente la sensibilité)
		int sensibility = 10;
		//On calcul le delta
		delta = std::pair<int, int>((mouse.first - mousePrevious.first) / sensibility, (mouse.second - mousePrevious.second) / sensibility);
	}
	else
	{
		delta = std::pair<int, int>(0, 0);
	}
		
		
	mouse.first = mouse.first - ((window.first - this->myMaster->getCurrentImage()->width)/2);
	mouse.second = mouse.second - ((window.second - this->myMaster->getCurrentImage()->height)/2);

	if(mouse.first < 0)
		mouse.first = 0;

	if(mouse.second < 0)
		mouse.second = 0;

	//Position du rectangle de zoom
	double x = region->x - delta.first;
	double y = region->y - delta.second;

	//Création du rectangle pour récupérer une partie de l'image
	this->region->x = x;
	this->region->y = y;
	this->region->width = this->myMaster->getCurrentImage()->width / this->nbZoom;
	this->region->height = this->myMaster->getCurrentImage()->height / this->nbZoom;
	
	if(this->region->x < 0)
		this->region->x = 0;

	if(region->y < 0)
		region->y = 0;

	if(this->region->x + this->region->width > this->myMaster->getCurrentImage()->width)
		this->region->x = this->myMaster->getCurrentImage()->width - this->region->width;

	if(this->region->y + this->region->height > this->myMaster->getCurrentImage()->height)
		this->region->y = this->myMaster->getCurrentImage()->height - this->region->height;

}

void ZoomInPlugin::changeIcon()
{
	/*
	if(this->myMaster->getZoom() == this->nbZoomInMax)
	{
		QString iconeString = QString (":/images/");
		iconeString += this->iconLock.data();
		this->setIcon(QIcon(iconeString));
	}
	else
	{
		QString iconeString = QString (":/images/");
		iconeString += this->icon.data();
		this->setIcon(QIcon(iconeString));
	}
	*/
}

void ZoomInPlugin::addZoom() 
{
	if(this->nbZoom < this->nbZoomInMax)
		this->nbZoom++;
}
void ZoomInPlugin::subZoom() 
{
	if(this->nbZoom > this->myPlugin->getZoomOutMax())
		this->nbZoom--;
}
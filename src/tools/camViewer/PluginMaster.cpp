#include "PluginMaster.h"
#include <iostream>
#include <string>
#include <QPushButton>
#include <QLayout>
#include <QApplication>

PluginMaster::PluginMaster(QopenCvWidget* val)
{
	
	this->myWidget = val;
	windowSize = std::pair<int, int>(0, 0);
	mousePosition = std::pair<int, int>(0, 0);
    std::cout << "Creation camera" << std::endl;
	this->myCameraModel = new CameraModel(this);
	
	this->pluginVector.push_back(new InfoPlugin(this));

	//this->pluginVector.push_back(new FullScreenPlugin(this));
	this->pluginVector.push_back(new FreezePlugin(this));

	ZoomInPlugin* zoomIn = new ZoomInPlugin(this);
	ZoomOutPlugin* zoomOut = new ZoomOutPlugin(this, zoomIn);
	this->pluginVector.push_back(zoomIn);
	this->pluginVector.push_back(zoomOut);


	LeftRotatePlugin* LeftRotate = new LeftRotatePlugin(this);
	this->pluginVector.push_back(new RightRotatePlugin(this, LeftRotate));
	this->pluginVector.push_back(LeftRotate);
	this->pluginVector.push_back(new ChangeCameraPlugin(this));

	int indexPlugin;
	for ( indexPlugin = 0; indexPlugin < this->pluginVector.size() ; indexPlugin++)
	{
		QString iconeString = QString (":/images/camViewer/");
		iconeString += this->pluginVector.at(indexPlugin)->icon.data();

		this->pluginVector.at(indexPlugin)->setObjectName(this->pluginVector.at(indexPlugin)->name.data());
		this->pluginVector.at(indexPlugin)->setStyleSheet("QPushButton {\n	background-color : rgba(255, 255, 255, 0);\n	border-radius: 0px;\n	border-color: beige;background-position:center;  background-image : url("+iconeString+"); padding:5px; \n background-repeat : no-repeat;\n}\nQPushButton:checked {background-color : rgb(218, 220, 215);\n	border-radius: 19px;\n background-image : url("+iconeString+"); background-position:center; }");
		QObject::connect( this->pluginVector.at(indexPlugin), SIGNAL(clicked()), this->pluginVector.at(indexPlugin), SLOT(buttonListener()));
	}
	this->replaceButton();
	
}

void PluginMaster::replaceButton()
{

	this->pluginVector.at(0)->setParent(this->myWidget->findChild<QWidget*>("bottom"));
	int bottomWidth = this->myWidget->findChild<QWidget*>("bottom")->width();
	this->pluginVector.at(0)->setGeometry(0, 5, 40, 40);
	this->pluginVector.at(0)->setVisible(true);
	
	int indexPlugin;
	for ( indexPlugin = 1; indexPlugin < this->pluginVector.size() ; indexPlugin++)
	{
		this->pluginVector.at(indexPlugin)->setParent(this->myWidget->findChild<QWidget*>("bottom"));
		this->pluginVector.at(indexPlugin)->setGeometry(bottomWidth-5-45*(indexPlugin), 5, 40, 40);
		this->pluginVector.at(indexPlugin)->setVisible(true);
	}
}		

PluginMaster::~PluginMaster()
{
	std::cout << this->pluginVector.size() << std::endl;
	this->myCameraModel->~CameraModel();
	int indexPlugin;
	for ( indexPlugin = 0; indexPlugin < this->pluginVector.size() ; indexPlugin++)
	{
		this->pluginVector.at(indexPlugin)->~IPlugin();
	}
	std::cout << "n'a plus" << std::endl;
	//cvReleaseImage(&(this->currentImage));
}

void PluginMaster::setFps(int fps){ this->myCameraModel->setFps(fps); }
int PluginMaster::getFps(){ return this->myCameraModel->getFps(); }
int PluginMaster::getListCamera(){ return this->myCameraModel->getListCamera(); }
std::vector<IPlugin*> PluginMaster::getListPlugin(){ return this->pluginVector; }

void PluginMaster::addWidgetInFrame(QWidget* widget, int x, int y)
{
	widget->setParent(this->myWidget);
	widget->setGeometry(x, y, widget->width() , widget->height() );

	widget->setVisible(true);
	this->myWidget->QWidget::setTabOrder(widget, this->myWidget->findChild<QWidget*>("label"));
	this->myWidget->repaint();
}

void PluginMaster::setCurrentImage(IplImage* img)
{ 
	this->currentImage = img; 
    notify();
	//On créé la nouvelle image a la bonne taille
	IplImage* image;

	int result = (int)(this->currentImage->height * this->myWidget->width() / this->currentImage->width);
	if (result <  this->myWidget->height())
        image = cvCreateImage(cvSize(this->myWidget->width() , result), this->currentImage->depth, this->currentImage->nChannels);
	else
	{
		result = (int)(this->currentImage->width * this->myWidget->height() / this->currentImage->height);
        image = cvCreateImage(cvSize(result, this->myWidget->height()), this->currentImage->depth, this->currentImage->nChannels);
    }
    cvResize(this->currentImage, image, cv::INTER_LINEAR);
	this->myWidget->putImage(this->myCameraModel->IplImage2QImage(image));
    cvReleaseImage(&image);
}
void PluginMaster::updateImage()
{ 
	//On créé la nouvelle image a la bonne taille
	IplImage* image;

	int result = (int)(this->currentImage->height * this->myWidget->width() / this->currentImage->width);
	if (result <  this->myWidget->height())
        image = cvCreateImage(cvSize(this->myWidget->width() , result), this->currentImage->depth, this->currentImage->nChannels);
	else
	{
		result = (int)(this->currentImage->width * this->myWidget->height() / this->currentImage->height);
        image = cvCreateImage(cvSize(result, this->myWidget->height()), this->currentImage->depth, this->currentImage->nChannels);
    }
	cvResize(this->currentImage, image, cv::INTER_LINEAR);
	this->myWidget->putImage(this->myCameraModel->IplImage2QImage(image));
	cvReleaseImage(&image);
}

void PluginMaster::rotateImage(IplImage* img, int angle)
{
	this->myCameraModel->rotate(img, angle);
}

void PluginMaster::zoomImage(IplImage* img, CvRect* region)
{
	this->myCameraModel->zoom(img, region);
}

void PluginMaster::notify()
{
	
	std::vector<IPlugin*>::iterator itePlugin;

	for(itePlugin = this->pluginVector.begin(); itePlugin != this->pluginVector.end(); itePlugin++)
	{
		if((*itePlugin)->observer)
		{
			(*itePlugin)->update(this->currentImage);
		}
    }
}

void PluginMaster::setFullscreen(bool full) 
{ 
	if(full)
		this->myWidget->showFullScreen(); 
	else
		this->myWidget->showNormal();
}

void PluginMaster::changeCursor(const QCursor &e)
{ 
	this->myWidget->getLabel()->setCursor(e); 
}

void PluginMaster::stop()
{
	//this->setFps(0);
	this->myCameraModel->stop();
}

void PluginMaster::resume()
{
	this->myCameraModel->resume();
}
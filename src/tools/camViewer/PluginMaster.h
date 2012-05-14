/******************************************************************************
 * File:	PluginMaster.h
 *
 * Author:  lysfith, kam
 * Purpose: Declaration of the class PluginMaster
 *****************************************************************************/
#ifndef PLUGIN_MASTER_H
#define PLUGIN_MASTER_H

#include "QopenCvWidget.h"
class QopenCvWidget;
#include "CameraModel.h"
class CameraModel;
#include "IPlugin.h"
class IPlugin;
#include <vector>

//////////////////////////////
//  inclusion des plugins   //
//////////////////////////////
#include "FreezePlugin.h"
class FreezePlugin;
#include "LeftRotatePlugin.h"
class LeftRotatePlugin;
#include "RightRotatePlugin.h"
class RightRotatePlugin;
#include "ChangeCameraPlugin.h"
class ChangeCameraPlugin;
#include "InfoPlugin.h"
class InfoPlugin;
#include "ZoomInPlugin.h"
class ZoomInPlugin;
#include "ZoomOutPlugin.h"
class ZoomOutPlugin;
#include "FullScreenPlugin.h"
class FullScreenPlugin;

namespace Ui {
	class QopenCvWidget;
}

class PluginMaster
{
    
public:
    PluginMaster(QopenCvWidget* val);
    ~PluginMaster();
	void setCurrentImage(IplImage* img);
	IplImage* getCurrentImage(){return this->currentImage; }
	void notify();

	//fps
	void setFps(int fps);
	int getFps();

	//rotation
	void rotateImage(IplImage* img, int angle);
	

	//zoom
	void zoomImage(IplImage* img, CvRect* region);

	//Fullscreen
	void setFullscreen(bool full);

	//autres
	void changeCursor(const QCursor &e);
	void setWindowSize(std::pair<int, int> p) { this->windowSize = p; }
	std::pair<int, int> getWindowSize() { return this->windowSize; }
	void setMousePositionPrevious(std::pair<int, int> p) { this->mousePositionPrevious = p; }
	std::pair<int, int> getMousePositionPrevious() { return this->mousePositionPrevious; }
	void setMousePosition(std::pair<int, int> p) { this->mousePosition = p; }
	std::pair<int, int> getMousePosition() { return this->mousePosition; }
	int getListCamera();
	void addWidgetInFrame(QWidget* widget, int x, int y);
	std::vector<IPlugin*> getListPlugin();
	void updateImage();
	void replaceButton();

	void stop();
	void resume();
private:
	QopenCvWidget* myWidget;
	CameraModel* myCameraModel;
	IplImage* currentImage;
	std::vector<IPlugin*> pluginVector;
	std::pair<int, int> windowSize;
	std::pair<int, int> mousePositionPrevious;
	std::pair<int, int> mousePosition;
};

#endif // PLUGIN_MASTER_H

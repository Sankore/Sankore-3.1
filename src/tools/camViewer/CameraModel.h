/******************************************************************************
 * File:	CameraModel.h
 *
 * Author:  lysfith, kam
 * Purpose: Declaration of the class CameraModel
 *****************************************************************************/

#ifndef CAMERA_MODEL_H
#define CAMERA_MODEL_H

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "PluginMaster.h"
class PluginMaster;


class CameraModel : public QWidget
{
    
public:
    CameraModel(PluginMaster* val);
    ~CameraModel();
    QImage IplImage2QImage(const IplImage* iplImage);
	void setFps(int fps);
	int getFps(){return this->fps;}
	
	
	
	int getListCamera();

	//rotate
    void rotate(IplImage* cvimage, int angle);

	//zoom
	void zoom(IplImage* image, CvRect* region);

	void stop();
	void resume();

private:
	PluginMaster* myPluginMaster;
	void timerEvent(QTimerEvent*);
	int timerId;
	int fps;
	CvCapture* camera;
	cv::VideoCapture videoCapture;
	int cvNumDevice;
    
};

#endif // CAMERA_MODEL_H

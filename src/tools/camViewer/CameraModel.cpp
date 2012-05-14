/******************************************************************************
 * File:	CameraModel.cpp
 *
 * Author:  lysfith, kam
 * Purpose:		Implementation of the class CameraModel.
 *****************************************************************************/
#include "CameraModel.h"

#include <iostream>
#include <string>

///////////////////////////////////////////////////////////////////////////////
// Name:        CameraModel::CameraModel()
// Purpose:     construct of CameraModel	
///////////////////////////////////////////////////////////////////////////////
CameraModel::CameraModel(PluginMaster* val)
{
	
	this->timerId = 0;
	this->camera = 0;
    this->cvNumDevice = -1;
	this->myPluginMaster = val;

	this->setFps(25);
	this->getListCamera();
}


///////////////////////////////////////////////////////////////////////////////
// Name:        CameraModel::~CameraModel()
// Purpose:     destruct of CameraModel	
///////////////////////////////////////////////////////////////////////////////
CameraModel::~CameraModel()
{
	if (camera != 0)
		cvReleaseCapture(&camera);
	std::cout << "destruction de la camera" << std::endl;
	this->killTimer(this->timerId);
}

///////////////////////////////////////////////////////////////////////////////
// Name:        CameraModel::setFps(int fps)
// Purpose:     this function set new fps.
// Parameters:
// - fps				New value.
///////////////////////////////////////////////////////////////////////////////
void CameraModel::setFps(int fps)
{
	this->fps = fps;
	if (this->timerId != 0)
		this->killTimer(this->timerId);
	this->timerId = 0;
	if (this->fps != 0)
		this->timerId = this->startTimer(1000/this->fps);
    std::cout << "Fin setFps" << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
// Name:        CameraModel::getListCamera()
// Purpose:     this function get the list of camera
///////////////////////////////////////////////////////////////////////////////
int CameraModel::getListCamera()
{
	int tempFps = this->getFps();
	this->setFps(0);

    this->camera = cvCaptureFromCAM(this->cvNumDevice +1);
	if (!this->camera)
	{
        this->camera = cvCaptureFromCAM( 0 );
		this->cvNumDevice = 0;
	}
	else
		this->cvNumDevice++;

	std::cout << "device numero : " << this->cvNumDevice << "--" << (this->camera) << std::endl; 
	this->setFps(tempFps);

	return this->cvNumDevice;
}

///////////////////////////////////////////////////////////////////////////////
// Name:        CameraModel::timerEvent(QTimerEvent* e)
// Purpose:     this function will be call in loop
// Parameters:
// - e			QTimerEvent of Qt.
///////////////////////////////////////////////////////////////////////////////
void CameraModel::timerEvent(QTimerEvent* e)
{
	//On récupère l'image
	IplImage* img = cvQueryFrame(camera);

	//On affiche l'image dans le label de l'interface
	this->myPluginMaster->setCurrentImage(img);
}

///////////////////////////////////////////////////////////////////////////////
// Name:        CameraModel::IplImage2QImage(const IplImage *iplImage)
// Purpose:     this function convert a IplImage in QImage.
// Parameters:
// - iplImage			IplImage to convert.
///////////////////////////////////////////////////////////////////////////////
QImage CameraModel::IplImage2QImage(const IplImage *iplImage)
{
    int height = iplImage->height;
    int width = iplImage->width;
    if(iplImage->depth == IPL_DEPTH_8U && iplImage->nChannels == 3)
    {
        const uchar *qImageBuffer = (const uchar*)iplImage->imageData;
        QImage img(qImageBuffer, width, height, QImage::Format_RGB888);
        return img.rgbSwapped();
    }
    else
	{
        return QImage();
    }
}

///////////////////////////////////////////////////////////////////////////////
// Name:        CameraModel::rotate(IplImage* image)
// Purpose:     this function rotate the image.	
// Parameters:
// - iplImage	Image to rotate
///////////////////////////////////////////////////////////////////////////////
void CameraModel::rotate(IplImage* image, int angle)
{
    // Set up variables
    CvMat* rot_mat = cvCreateMat(2,3,CV_32FC1);
    IplImage *dst = cvCloneImage( image );
    IplImage *newImage = cvCloneImage( image );

    // Compute rotation matrix
    CvPoint2D32f center = cvPoint2D32f( image->width/2, image->height/2 );
    cv2DRotationMatrix( center, angle, 1, rot_mat );

    // Do the transformation
    cvWarpAffine( newImage, dst, rot_mat );

    cvCopy ( dst, image );
    cvReleaseImage( &dst );
    cvReleaseImage( &newImage );
    cvReleaseMat( &rot_mat );
}

///////////////////////////////////////////////////////////////////////////////
// Name:        CameraModel::zoom(IplImage* image)
// Purpose:     this function zoom on the image
// Parameters:
// - iplImage	Image to zoom
///////////////////////////////////////////////////////////////////////////////
void CameraModel::zoom(IplImage* image, CvRect* region)
{
	//Image temporaire
    IplImage *tempImage = cvCloneImage(image);

    //On met le ROI (region of interest ou rectangle) a partir de la sourcel
    cvSetImageROI(tempImage, *region);
	//On resize l'image pris dans l'image original pour la redimensionner a la bonne taille
    cvResize(tempImage, image, cv::INTER_LINEAR);

	//On libere la memoire de l'image temporaire
    cvReleaseImage( &tempImage );
}

void CameraModel::stop()
{
	this->setFps(0);
	if (camera != 0)
		cvReleaseCapture(&camera);
}

void CameraModel::resume()
{
	this->getListCamera();
	this->setFps(25);

}
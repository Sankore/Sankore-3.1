#include "QopenCvWidget.h"
#include <iostream>
#include <string>
#include "domain/UBGraphicsDelegateFrame.h"
#define MARGIN_PANNEL_BUTTON 20
#define SIZE_PANNEL_BUTTON 50

QopenCvWidget::QopenCvWidget(QWidget *widgetParent, QGraphicsItem *GraphicParent) :
	QWidget(widgetParent),
	UBGraphicsProxyWidget(GraphicParent)
{
	//Q_INIT_RESOURCE(ressource);	

	this->MyPlugMaster = NULL;
	this->setStyleSheet(QString::fromUtf8("border-radius: 25px;"));
	
	this->QWidget::setMinimumSize(QSize(40, 40));
	this->QWidget::resize(500, 500);
	
    this->QWidget::setMinimumSize(QSize(500, 500));
    label = new QLabel(this);
    label->setObjectName(QString::fromUtf8("label"));
    label->setStyleSheet(QString::fromUtf8("background-color : rgba(0, 0, 0, 250)"));
	label->setAlignment(Qt::AlignCenter);
	label->resize(500, 500);
	label->lower();

	top = new QWidget(this);
    top->setObjectName(QString::fromUtf8("top"));
    top->setGeometry(QRect(20, 20, 309, 40));
    top->setMinimumSize(QSize(50, 50));
    top->setMaximumSize(QSize(16777215, 50));
    top->setStyleSheet(QString::fromUtf8("background-color: rgba(151, 255, 65, 00);"));
    top->raise();
	
	bottom = new QWidget(this);
    bottom->setObjectName(QString::fromUtf8("bottom"));
	bottom->setGeometry(QRect(MARGIN_PANNEL_BUTTON, this->height()-MARGIN_PANNEL_BUTTON, this->width()-MARGIN_PANNEL_BUTTON*2, SIZE_PANNEL_BUTTON));
    bottom->setMinimumSize(QSize(50, 50));
    bottom->setMaximumSize(QSize(16777215, 50));
    bottom->setStyleSheet(QString::fromUtf8("background-color: rgba(151, 255, 65, 0);"));
    bottom->setVisible(true);
	
	right = new QWidget(this);
    right->setObjectName(QString::fromUtf8("right"));
    right->setGeometry(QRect(290, 70, 40, 179));
    right->setMinimumSize(QSize(50, 50));
    right->setMaximumSize(QSize(50, 16777215));
    right->setStyleSheet(QString::fromUtf8("background-color: rgba(151, 255, 65, 00);"));
    right->raise();
	
	left = new QWidget(this);
    left->setObjectName(QString::fromUtf8("left"));
    left->setGeometry(QRect(20, 80, 40, 179));
    left->setMinimumSize(QSize(50, 50));
    left->setMaximumSize(QSize(50, 16777215));
    left->setStyleSheet(QString::fromUtf8("background-color: rgba(151, 255, 65, 00);"));
	left->raise();

	//this->QWidget::setVisible(true);

	

    std::cout << "Fin QopenCvWidget" << std::endl;
	
	
	this->setWidget ( this );
	UBGraphicsItemDelegate* delegate = new UBGraphicsItemDelegate ( this );
	delegate->init();
    setDelegate ( delegate );
	mDelegate->frame()->setOperationMode ( UBGraphicsDelegateFrame::Resizing );
	this->MyPlugMaster = new PluginMaster(this);
}

 void QopenCvWidget::resizeEvent( QResizeEvent * ev )  
{  
	
    std::cout << "Debut resizeEvent" << std::endl;

	this->label->setGeometry(0, 0, this->width(), this->height());
	this->bottom->setGeometry(MARGIN_PANNEL_BUTTON,this->height()-MARGIN_PANNEL_BUTTON - SIZE_PANNEL_BUTTON, this->width() - 2*MARGIN_PANNEL_BUTTON, SIZE_PANNEL_BUTTON);
	this->top->setGeometry(MARGIN_PANNEL_BUTTON, MARGIN_PANNEL_BUTTON, this->width() - 2*MARGIN_PANNEL_BUTTON, SIZE_PANNEL_BUTTON);
	this->left->setGeometry(MARGIN_PANNEL_BUTTON, 2*MARGIN_PANNEL_BUTTON + SIZE_PANNEL_BUTTON, SIZE_PANNEL_BUTTON, this->height() - (4*MARGIN_PANNEL_BUTTON + 2*SIZE_PANNEL_BUTTON));
	this->right->setGeometry(this->width() - (MARGIN_PANNEL_BUTTON + SIZE_PANNEL_BUTTON), 2*MARGIN_PANNEL_BUTTON + SIZE_PANNEL_BUTTON, SIZE_PANNEL_BUTTON, this->height() - (4*MARGIN_PANNEL_BUTTON + 2*SIZE_PANNEL_BUTTON));
	
	if(this->MyPlugMaster != NULL)
		this->MyPlugMaster->replaceButton();
    std::cout << "Fin resizeEvent" << std::endl;
	
}

void QopenCvWidget::mouseMoveEvent( QMouseEvent * ev )  
{  
	this->MyPlugMaster->setMousePosition(std::pair<int, int>(ev->x(), ev->y()));
}

void QopenCvWidget::mousePressEvent( QMouseEvent * ev )  
{  
	if(ev->button() == Qt::LeftButton)
	{
		this->MyPlugMaster->setMousePositionPrevious(std::pair<int, int>(ev->x(), ev->y()));
		this->MyPlugMaster->setMousePosition(std::pair<int, int>(ev->x(), ev->y()));
	}
}

void QopenCvWidget::mouseReleaseEvent( QMouseEvent * ev )  
{  
	if(ev->button() == Qt::LeftButton)
	{
		this->MyPlugMaster->setMousePositionPrevious(std::pair<int, int>(0, 0));
		this->MyPlugMaster->setMousePosition(std::pair<int, int>(ev->x(), ev->y()));
	}
}

QopenCvWidget::~QopenCvWidget()
{

	if (this->MyPlugMaster)
	{
		this->MyPlugMaster->~PluginMaster();
		
		this->MyPlugMaster = 0;
	}
	this->setWidget ( NULL );
}

void QopenCvWidget::putImage(QImage img)
{
	//On affiche l'image dans le label
    this->label->setPixmap(QPixmap::fromImage(img));
}

UBItem* QopenCvWidget::deepCopy() const
{
	/*
   UBGraphicsCompass* copy = new UBGraphicsCompass();

    copy->setPos(this->pos());
    copy->setRect(this->rect());
//    copy->setZValue(this->zValue());
    UBGraphicsItem::assignZValue(copy, this->zValue());
    copy->setTransform(this->transform());

   // TODO UB 4.7 ... complete all members ?
*/
   return NULL;
   
}

void QopenCvWidget::pause() 
{ 
	this->MyPlugMaster->stop(); 
}

void QopenCvWidget::resume() 
{ 
	this->MyPlugMaster->resume(); 
}

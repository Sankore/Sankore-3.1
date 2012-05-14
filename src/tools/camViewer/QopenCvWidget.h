#ifndef Q_OPENCV_WIDGET_H
#define Q_OPENCV_WIDGET_H

#include <QWidget>

#include <QCloseEvent>
#include <QtCore/QVariant>
#include <iostream>
#include <QtGui/QHBoxLayout>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QWidget>
//#include "domain/UBItem.h"
#include "domain/UBGraphicsScene.h"
#include "PluginMaster.h"
#include "domain/UBGraphicsProxyWidget.h"
class PluginMaster;

class QopenCvWidget : public QWidget, public UBGraphicsProxyWidget
{
    Q_OBJECT
public:
    explicit QopenCvWidget(QWidget *widgetParent = 0, QGraphicsItem *GraphicParent = 0);
    ~QopenCvWidget();
	void putImage(QImage img);

	QLabel *getLabel() { return this->label; }
	void remove(){std::cout <<"zzaza" << std::endl;}
    void clearSource(){this->~QopenCvWidget();}
	void pause();
	void resume();
public slots:
	void _q_removeWidgetSlot(){this->~QopenCvWidget();}
//protected:
	//QVariant itemChange(GraphicsItemChange change, const QVariant &value){return NULL;}

	UBItem* QopenCvWidget::deepCopy() const;
private:
	void resizeEvent( QResizeEvent * ev );
	void mouseMoveEvent( QMouseEvent * ev ); 
	void mousePressEvent( QMouseEvent * ev );
	void mouseReleaseEvent( QMouseEvent * ev );
	PluginMaster* MyPlugMaster;

	QLabel *label;
    QWidget *top;
    QWidget *bottom;
    QWidget *right;
    QWidget *left;
protected:
	QVariant  itemChange(GraphicsItemChange change, const QVariant &value)
	{

		   if ((change == QGraphicsItem::ItemEnabledChange)
				|| (change == QGraphicsItem::ItemSceneChange)
				|| (change == QGraphicsItem::ItemVisibleChange))
		{
				this->resume();
				std::cout << "lance" << std::endl;
		}
		else if (change == QGraphicsItem::ItemSceneHasChanged)
		{
			if (!scene())
			{
				this->pause();
				std::cout << "pause" << std::endl;
			}
			else
			{
				std::cout << "je sais aps trop quand ca arrive" << std::endl;
			}
		}

		return UBGraphicsProxyWidget::itemChange(change, value);
	}
};

#endif // Q_OPENCV_WIDGET_H

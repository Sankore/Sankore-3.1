#ifndef UBSHAPEFACTORY_H
#define UBSHAPEFACTORY_H

#include <QObject>
#include <QColor>
#include <QGraphicsItem>

class UBShape;
class UBBoardView;
class QMouseEvent;
class UBDrawingController;

class UBShapeFactory : public QObject
{
    Q_OBJECT

public:
    UBShapeFactory();
    void init();

    static bool isShape(QGraphicsItem *item);

    enum ShapeType
    {
        Ellipse,
        Circle,
        Rectangle,
        Square,
        Line,
        Pen,
        Polygon
    };
public slots:
    void createEllipse(bool create);
    void createPolygon(bool create);
    void createCircle(bool create);
    void createRectangle(bool create);
    void createSquare(bool create);
    void createLine(bool create);
    void createPen(bool create);

    void changeFillColor(bool ok);

    void onMouseMove(QMouseEvent *event);
    void onMousePress(QMouseEvent *event);
    void onMouseRelease(QMouseEvent *event);

    void desactivate();

private:
    UBShape* mCurrentShape;
    UBBoardView* mBoardView;

    bool mIsCreating;
    bool mIsPress;
    bool mIsRegularShape;

    ShapeType mShapeType;

    QColor mCurrentStrokeColor;
    QColor mCurrentFillFirstColor;

    UBDrawingController *mDrawingController;

protected:
    UBShape *instanciateCurrentShape();

    template<typename C>
    C *instanciate();

};

#endif // UBSHAPEFACTORY_H

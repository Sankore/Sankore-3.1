#ifndef UBSHAPEFACTORY_H
#define UBSHAPEFACTORY_H

#include <QObject>
#include <QColor>

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

public slots:
    void createEllipse(bool create);
    void createPolygon(bool create);
    void createCircle(bool create);

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

    enum ShapeType
    {
        Ellipse,
        Circle,
        Polygon
    };

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

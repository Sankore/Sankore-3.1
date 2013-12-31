#ifndef UBSHAPEFACTORY_H
#define UBSHAPEFACTORY_H

#include <QObject>
#include <QColor>

class UBShape;
class UBBoardView;
class QMouseEvent;

class UBShapeFactory : public QObject
{
    Q_OBJECT

public:
    UBShapeFactory();
    void init();

public slots:
    void createEllipse(bool create);
    void createCircle(bool create);

    void changeFillColor(bool ok);

    void onMouseMove(QMouseEvent *event);
    void onMousePress(QMouseEvent *event);
    void onMouseRelease(QMouseEvent *event);

private:
    UBShape* mCurrentShape;
    UBBoardView* mBoardView;

    bool mIsCreating;
    bool mIsPress;
    bool mIsRegularShape;
};

#endif // UBSHAPEFACTORY_H

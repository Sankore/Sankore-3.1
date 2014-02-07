#ifndef UBEDITABLE_H
#define UBEDITABLE_H

#include <QVector>

#include "UBAbstractHandle.h"



class UBEditable
{
public:
    explicit UBEditable();

    virtual ~UBEditable();

    void addHandle(UBAbstractHandle *mhandle);

    void showEditMode(bool show);

    virtual void updateHandle(UBAbstractHandle *handle) = 0;

    virtual void onBackgroundSceneClick(){ }

    virtual void deactivateEditionMode();

    bool isInEditMode();
protected:
    QVector<UBAbstractHandle*> mHandles;

private:
    bool mEditMode;
};

#endif // UBEDITABLE_H

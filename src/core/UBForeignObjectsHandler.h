#ifndef UBFOREIGHNOBJECTSHANDLER_H
#define UBFOREIGHNOBJECTSHANDLER_H

#include <QList>
#include <QUrl>

class UBForeighnObjectsHandlerPrivate;

class UBForeighnObjectsHandler
{
public:
    UBForeighnObjectsHandler();
    ~UBForeighnObjectsHandler();

    void cure(const QList<QUrl> &dirs);
    void cure(const QUrl &dir);

private:
    UBForeighnObjectsHandlerPrivate *d;

    friend class UBForeighnObjectsHandlerPrivate;
};

#endif // UBFOREIGHNOBJECTSHANDLER_H

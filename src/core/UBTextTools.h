#ifndef UBTEXTTOOLS_H
#define UBTEXTTOOLS_H

#include <QString>
#include <QRegExp>

class UBTextTools
{
public:
    UBTextTools(){}
    virtual ~UBTextTools(){}

    static QString cleanHtmlCData(const QString& _html);
    static QString cleanHtml(const QString& _html);


    //issue 1539 - NNE - 20131022
    static QString formatStyle(QString html);
    static QString removeEmptyBalises(QString html);
    static QString removeSpanWithSpace(QString html);
    static QString removeIdsAndClasses(QString html);
    static QString removeImg(QString html);
    //issue 1539 - NNE - 20131022

    //issue 1539 - NNE - 20131018
    static QString cleanFormatHtml(QString html);
};

#endif // UBTEXTTOOLS_H

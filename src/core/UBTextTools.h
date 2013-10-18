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

    //issue 1539 - NNE - 20131018
    static QString cleanFontFormatHtml(QString html);
};

#endif // UBTEXTTOOLS_H

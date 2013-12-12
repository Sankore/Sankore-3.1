#ifndef UBTEXTTOOLS_H
#define UBTEXTTOOLS_H

#include <QString>
#include <QApplication>
#include <QMimeData>
#include <QTextDocument>
#include <QClipboard>

class UBTextTools{
public:
    UBTextTools(){}
    virtual ~UBTextTools(){}

    static QString cleanHtmlCData(const QString& _html);
    static QString cleanHtml(const QString& _html);
    static void cleanHtmlClipboard();
};

#endif // UBTEXTTOOLS_H

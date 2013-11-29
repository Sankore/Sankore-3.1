#include <QDebug>
#include "UBTextTools.h"

QString UBTextTools::cleanHtmlCData(const QString &_html){

    QString clean = "";


    for(int i = 0; i < _html.length(); i+=1){
        if(_html.at(i) != '\0')
            clean.append(_html.at(i));
    }
    return clean;
}


QString UBTextTools::cleanHtml(const QString& _html)
{
    const QString START_TAG = "<body";
    const QString END_TAG = "</body";

    QString cleanSource = "";
    QString simplifiedHtml = _html;

    int start = simplifiedHtml.toLower().indexOf(START_TAG);
    int end = simplifiedHtml.toLower().indexOf(END_TAG) + END_TAG.size();

    if(start != -1 && end != -1)
        cleanSource = simplifiedHtml.mid(start, end);
    else
        cleanSource = _html;

    return cleanSource;
}

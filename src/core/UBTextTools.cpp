#include <QDebug>
#include "UBTextTools.h"
#include <QStringList>

QString UBTextTools::cleanHtmlCData(const QString &_html)
{

    QString clean = "";

    for(int i = 0; i < _html.length(); i+=1){
        if(_html.at(i) != '\0')
            clean.append(_html.at(i));
    }
    return clean;
}


QString UBTextTools::cleanHtml(const QString& _html)
{
    const QString START_TAG = "<!doctype";
    const QString END_TAG = "</html";

    QString cleanSource = "";
    QString simplifiedHtml = _html;

    int start = simplifiedHtml.toLower().indexOf(START_TAG);
    int end = simplifiedHtml.toLower().indexOf(END_TAG) + END_TAG.size();
    cleanSource = simplifiedHtml.mid(start, end);

    return cleanSource;
}

QString UBTextTools::cleanFormatHtml(QString html)
{
    html = UBTextTools::removeIdsAndClasses(html);
    html = UBTextTools::removeImg(html);
    html = UBTextTools::removeEmptyBalises(html);
    html = UBTextTools::formatStyle(html);
    html = UBTextTools::removeSpanWithSpace(html);

    qDebug() << html;

    return html;
}

QString UBTextTools::formatStyle(QString html)
{
    int start = 0,
        end = 0,
        size = 0;

    QString content = "",
            newContent = "",
            styleTokenStart = "style=\"",
            styleTokenEnd = "\"";

    QRegExp boldRegex("font-weight: bold;"),
            italicRegex("font-style: italic");

    while((start = html.indexOf(styleTokenStart, start)) > 0){

        end = html.indexOf(styleTokenEnd, start+ styleTokenStart.size() + 1);
        size = end - start;

        content = html.mid(start, end);

        if(content.contains(boldRegex)){
            newContent = "font-weight: bold;";
        }

        if(content.contains(italicRegex)){
            newContent = "font-style: italic;";
        }

        if(!newContent.isEmpty()){
            html.replace(start, size, "style=\""+newContent);
            newContent = "";
            start += newContent.size() + 7;
        }else{
            html.replace(start, size+1, "");
            start++;
        }
    }

    return html;
}

QString UBTextTools::removeEmptyBalises(QString html)
{
    QString startOpenToken = "<",
            endToken = ">",
            startCloseToken = "</";

    int start = 0,
        end = 0;

    while((start = html.indexOf(startOpenToken, start)) > 0){
        end = html.indexOf(endToken, start+1);

        if(end == html.indexOf(startCloseToken, end+1) -1){
            //The balise is empty, so remove it
            int len = html.indexOf(endToken, end+1) - start;

            html.replace(start, len, "");
        }
    }

    return html;
}

QString UBTextTools::removeSpanWithSpace(QString html)
{
    QRegExp span("<span[aA-zZ ]*> </span>");
    html.replace(span, " ");

    return html;
}

QString UBTextTools::removeIdsAndClasses(QString html)
{
    QRegExp idClassRegex("(id|class)[aA-zZ=\"-]*");
    html.remove(idClassRegex);

    return html;
}

QString UBTextTools::removeImg(QString html)
{
    QRegExp imageRegex("<img[aA-zZ =\":/\\.\\d,_-]*/?>");
    html.remove(imageRegex);

    return html;
}

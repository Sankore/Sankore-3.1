#ifndef UBCREATEHYPERLINKPALETTE_H
#define UBCREATEHYPERLINKPALETTE_H

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QString>
#include <QPushButton>
#include <QMouseEvent>

#include "gui/UBFloatingPalette.h"

class UBCreateHyperLinkPalette : public UBFloatingPalette
{
    Q_OBJECT
public:
    explicit UBCreateHyperLinkPalette(QWidget *parent = 0);
    ~UBCreateHyperLinkPalette();

    QString link() const;
    QString text() const;

    void setPos(QPoint pos);

    signals:
        void validationRequired();

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent * event);

private:

    QVBoxLayout* mVLayout;
    QHBoxLayout* mLinkLayout;
    QHBoxLayout* mTextLayout;
    QLabel* mLinkLabel;
    QLineEdit* mLinkInput;
    QLabel* mTextLabel;
    QLineEdit* mTextInput;
    QPushButton* mValidateButton;
    QPixmap mClosePixmap;
};

#endif // UBCREATELINKPALETTE_H

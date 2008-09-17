#ifndef QSLIDELABEL_H
#define QSLIDELABEL_H

#include <QString>
#include <QWidget>
#include <QTimeLine>
#include <QQueue>
#include <QFrame>

class QSlideLabel: public QFrame
{
  Q_OBJECT
  public:
    QSlideLabel(const QString &text, QWidget *parent=NULL);
    virtual ~QSlideLabel() {};
    void updateText(const QString &newText);
  protected:
    virtual void paintEvent(QPaintEvent * event);
  private slots:
    void animationStep(qreal);
    void animationOver();
  private:
    QTimeLine *time;
    QString cur_str;
    QQueue<QString> new_str;
};

#endif

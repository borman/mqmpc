#include "qslidelabel.h"
#include <QPainter>

QSlideLabel::QSlideLabel(const QString &text, QWidget *parent)
  : QFrame(parent)
{
  time = new QTimeLine(500, this);
  cur_str = text;
  setMinimumHeight(fontMetrics().height()+3);
  connect(time, SIGNAL(valueChanged(qreal)), SLOT(animationStep(qreal)));
  connect(time, SIGNAL(finished()), SLOT(animationOver()));
}

void QSlideLabel::updateText(const QString &newText)
{
  new_str.enqueue(newText);
  if (time->state()!=QTimeLine::Running)
    time->start();
}

void QSlideLabel::paintEvent(QPaintEvent * event)
{
  QPainter p(this);

  QLinearGradient fontGrad(0, 0, width(), 0);
  fontGrad.setColorAt(0.0, QColor(255, 255, 255, 255));
  fontGrad.setColorAt(0.8, QColor(255, 255, 255, 255));
  fontGrad.setColorAt(1.0, QColor(255, 255, 255, 0));
  QPen pen(QBrush(fontGrad), 1);
  p.setPen(pen);

  if (time->state()==QTimeLine::Running)
  {
    int h1 = height()*time->currentValue();
    int h2 = height()-h1;
    p.drawText(0, -h1, width(), height(), Qt::AlignLeft|Qt::AlignVCenter, cur_str);
    p.drawText(0, h2, width(), height(), Qt::AlignLeft|Qt::AlignVCenter, new_str.head());
  }
  else
  {
    p.drawText(rect(), Qt::AlignLeft|Qt::AlignVCenter, cur_str);
  }
}

void QSlideLabel::animationStep(qreal)
{
  repaint();
}

void QSlideLabel::animationOver()
{
  cur_str = new_str.dequeue();
  if (!new_str.isEmpty())
    time->start();
}

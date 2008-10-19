#include <cstdio>

#include <QPainter>
#include <QPaintEvent>

#include "qmedialabel.h"

struct QMediaLabelContents
{
  QString title;
  QString artist;
  QString album;
};

QMediaLabel::QMediaLabel(QWidget *parent)
  : QFrame(parent)
{
  firstLineFont = new QFont("Helvetica", 9, QFont::Bold);
  secondLineFont = new QFont("Helvetica", 7);

  flFontMetrics = new QFontMetrics(*firstLineFont);
  slFontMetrics = new QFontMetrics(*secondLineFont);

  current = new QMediaLabelContents;
  current->title = current->artist = current->album = QString::null;

  transTimeLine = new QTimeLine(300, this);
  connect(transTimeLine, SIGNAL(finished()), SLOT(finishTransition()));
  connect(transTimeLine, SIGNAL(valueChanged(qreal)), SLOT(updateTransition(qreal)));
}

QMediaLabel::~QMediaLabel()
{
}

QSize QMediaLabel::sizeHint() const
{
  return QSize(-1, flFontMetrics->height()+slFontMetrics->height()+3);
}

void QMediaLabel::updateContents(const QString &title,
                                 const QString &artist,
                                 const QString &album)
{
  QMediaLabelContents *cont = new QMediaLabelContents;
  cont->title = title;
  cont->artist = artist;
  cont->album = album;

  pending.enqueue(cont);
  checkPending();
}

void QMediaLabel::paintEvent(QPaintEvent *event)
{
  QPainter p(this);

  QLinearGradient fontGrad(0, 0, width(), 0);
  fontGrad.setColorAt(0.0, QColor(255, 255, 255, 255));
  fontGrad.setColorAt(0.8, QColor(255, 255, 255, 255));
  fontGrad.setColorAt(1.0, QColor(255, 255, 255, 0));
  QBrush gradBrush(fontGrad);
  QPen pen(gradBrush, 1.0);
  p.setPen(pen);

  QRect flRect(0, 1, width(), flFontMetrics->height());
  p.setFont(*firstLineFont);
  if (transTimeLine->state()==QTimeLine::Running && (previous->title!=current->title))
  {
    int flShift2 = (1.0-transTimeLine->currentValue())*width();
    int flShift1 = -transTimeLine->currentValue()*width();
    QRect flRect1 = flRect.translated(flShift1, 0);
    QRect flRect2 = flRect.translated(flShift2, 0);
    p.drawText(flRect1, Qt::TextSingleLine|Qt::AlignVCenter|Qt::AlignLeft, previous->title);
    p.drawText(flRect2, Qt::TextSingleLine|Qt::AlignVCenter|Qt::AlignLeft, current->title);
  }
  else
  {
    p.drawText(flRect, Qt::TextSingleLine|Qt::AlignVCenter|Qt::AlignLeft, current->title);
  }

  QRect slRect(0, height()-slFontMetrics->height()-1, width(), slFontMetrics->height());
  p.setFont(*secondLineFont);
  if (transTimeLine->state()==QTimeLine::Running && (previous->album!=current->album))
  {
    int slShift2 = (1.0-transTimeLine->currentValue())*width();
    int slShift1 = -transTimeLine->currentValue()*width();
    QRect slRect1 = slRect.translated(slShift1, 0);
    QRect slRect2 = slRect.translated(slShift2, 0);
    p.drawText(slRect1, Qt::TextSingleLine|Qt::AlignVCenter|Qt::AlignLeft, previous->album);
    p.drawText(slRect2, Qt::TextSingleLine|Qt::AlignVCenter|Qt::AlignLeft, current->album);
  }
  else
  {
    p.drawText(slRect, Qt::TextSingleLine|Qt::AlignVCenter|Qt::AlignLeft, current->album);
  }
}

void QMediaLabel::checkPending()
{
  if (transTimeLine->state()==QTimeLine::NotRunning)
    if (!pending.isEmpty())
    {
      previous = current;
      current = pending.dequeue();
      transTimeLine->start();
    }
}

void QMediaLabel::finishTransition()
{
  delete previous;
  checkPending();
}

void QMediaLabel::updateTransition(qreal value)
{
  update();
}

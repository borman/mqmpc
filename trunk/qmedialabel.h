#ifndef QMEDIALABEL_H
#define QMEDIALABEL_H

#include <QFrame>
#include <QFont>
#include <QFontMetrics>
#include <QQueue>
#include <QString>
#include <QTimeLine>

struct QMediaLabelContents;

class QMediaLabel: public QFrame
{
  Q_OBJECT
  public:
    QMediaLabel(QWidget *parent);
    virtual ~QMediaLabel();

    virtual QSize sizeHint() const;

    void updateContents(const QString &title,
                        const QString &artist = QString::null,
                        const QString &album = QString::null);
  protected:
    virtual void paintEvent(QPaintEvent *event);

  public slots:
    void updateTransition(qreal value);
    void finishTransition();
    void checkPending();

  private:
    QFont *firstLineFont;
    QFontMetrics *flFontMetrics;
    QFont *secondLineFont;
    QFontMetrics *slFontMetrics;

    QMediaLabelContents *current;
    QMediaLabelContents *previous;
    QQueue<QMediaLabelContents *> pending;

    QTimeLine *transTimeLine;
};

#endif // QMEDIALABEL_H

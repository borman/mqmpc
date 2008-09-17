#ifndef UI_GENERAL_H
#define UI_GENERAL_H

#include <QFrame>
#include <QLabel>
#include <QListWidget>
#include <QToolButton>
#include <QPushButton>
#include <QProgressBar>

#define UI_PROGRESSBAR_SIZE 512

QIcon loadMultiIcon(const QString &name);

class GeneralUI: public QFrame
{
  Q_OBJECT
public:
  GeneralUI(QWidget *parent);
  virtual ~GeneralUI();

  QListWidget *playlist;

  QPushButton *prevButton;
  QPushButton *stopButton;
  QPushButton *playButton;
  QPushButton *nextButton;

  QProgressBar *progressBar;

  QLabel *nowPlaying;
};

#endif


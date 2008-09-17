#ifndef UI_GENERAL_H
#define UI_GENERAL_H

#ifdef EZX

#include <qframe.h>
#include <qlabel.h>
#include <UTIL_ListBox.h>
#include <UTIL_PushButton.h>
#include <UTIL_ProgressBar.h>

#else

#include <QFrame>
#include <QLabel>
#include <QListWidget>
#include <QToolButton>
#include <QPushButton>
#include <QProgressBar>

#endif

#define UI_PROGRESSBAR_SIZE 512

#ifdef EZX
QPixmap loadGraphics(const QString &name);
#else
QIcon loadMultiIcon(const QString &name);
#endif

class GeneralUI: public QFrame
{
  Q_OBJECT
public:
  GeneralUI(QWidget *parent);
  virtual ~GeneralUI();

#ifdef EZX

  UTIL_ListBox *playlist;

  UTIL_PushButton *prevButton;
  UTIL_PushButton *stopButton;
  UTIL_PushButton *playButton;
  UTIL_PushButton *nextButton;

  UTIL_ProgressBar *progressBar;

#else

  QListWidget *playlist;

  QPushButton *prevButton;
  QPushButton *stopButton;
  QPushButton *playButton;
  QPushButton *nextButton;

  QProgressBar *progressBar;

#endif

  QLabel *nowPlaying;
};

#endif


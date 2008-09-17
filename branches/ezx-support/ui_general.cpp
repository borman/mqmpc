#include "ui_general.h"

#ifdef EZX

#include <qlayout.h>
#include <qfont.h>
#include <qhbox.h>
#include <qpixmapcache.h>

#include <UTIL_ProgressBar.h>
#include <UTIL_PushButton.h>

QPixmap loadGraphics(const QString &name)
{
  QPixmap img;
  if (!QPixmapCache::find(name, img))
  {
    img.load(QString("skin/%1.png").arg(name));
    QPixmapCache::insert(name, img);
  }
  return img;
}

#else

#include <QIcon>
#include <QPixmapCache>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QtopiaItemDelegate>

QIcon loadMultiIcon(const QString &name)
{
  // Seems like package's sandbox is application working directory
  QString path = QString("./pics/mqmpc/icons/%1").arg(name);
  QIcon icon(path);
  return icon;
}

#endif

GeneralUI::GeneralUI(QWidget *parent)
#ifdef EZX
: QFrame(parent, "GeneralUI")
#else
: QFrame(parent)
#endif
{
  setFrameStyle(QFrame::NoFrame);
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  mainLayout->addSpacing(5);

  nowPlaying = new QLabel(this);
  nowPlaying->setAlignment(Qt::AlignVCenter);
  nowPlaying->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
  mainLayout->addWidget(nowPlaying);

#ifdef EZX
  progressBar = new UTIL_ProgressBar(UI_PROGRESSBAR_SIZE, this);
  progressBar->setCenterIndicator(true);
  progressBar->setIndicatorVisible(true);
  progressBar->setProgress(0);
#else
  progressBar = new QProgressBar(this);
  progressBar->setRange(0, UI_PROGRESSBAR_SIZE);
  progressBar->reset();
#endif
  mainLayout->addWidget(progressBar);

  mainLayout->addSpacing(5);

  QFrame *buttonBarWidget = new QFrame(this);
  QHBoxLayout *buttonBarLayout = new QHBoxLayout(buttonBarWidget);
  #ifdef EZX
  prevButton = new UTIL_PushButton(buttonBarWidget, 0, 48, 48);
  prevButton->setNormal(loadGraphics("media-skip-backward"));
  prevButton->setActive(loadGraphics("media-skip-backwardA"));
  prevButton->setDisabled(loadGraphics("media-skip-backwardD"));
  stopButton = new UTIL_PushButton(buttonBarWidget, 0, 48, 48);
  stopButton->setNormal(loadGraphics("media-playback-stop"));
  stopButton->setActive(loadGraphics("media-playback-stopA"));
  stopButton->setDisabled(loadGraphics("media-playback-stopD"));
  playButton = new UTIL_PushButton(buttonBarWidget, 0, 48, 48);
  playButton->setNormal(loadGraphics("media-playback-start"));
  playButton->setActive(loadGraphics("media-playback-startA"));
  playButton->setDisabled(loadGraphics("media-playback-startD"));
  nextButton = new UTIL_PushButton(buttonBarWidget, 0, 48, 48);
  nextButton->setNormal(loadGraphics("media-skip-forward"));
  nextButton->setActive(loadGraphics("media-skip-forwardA"));
  nextButton->setDisabled(loadGraphics("media-skip-forwardD"));
  #else
  prevButton = new QPushButton(buttonBarWidget);
  prevButton->setIcon(loadMultiIcon("media-skip-backward"));
  prevButton->setIconSize(QSize(48, 48));
  prevButton->setFlat(true);
  stopButton = new QPushButton(buttonBarWidget);
  stopButton->setIcon(loadMultiIcon("media-playback-stop"));
  stopButton->setIconSize(QSize(48, 48));
  stopButton->setFlat(true);
  playButton = new QPushButton(buttonBarWidget);
  playButton->setIcon(loadMultiIcon("media-playback-start"));
  playButton->setIconSize(QSize(48, 48));
  playButton->setFlat(true);
  nextButton = new QPushButton(buttonBarWidget);
  nextButton->setIcon(loadMultiIcon("media-skip-forward"));
  nextButton->setIconSize(QSize(48, 48));
  nextButton->setFlat(true);
  #endif
  buttonBarLayout->addWidget(prevButton);
  buttonBarLayout->addWidget(stopButton);
  buttonBarLayout->addWidget(playButton);
  buttonBarLayout->addWidget(nextButton);
  mainLayout->addWidget(buttonBarWidget);

  mainLayout->addSpacing(5);

#ifdef EZX
  playlist = new UTIL_ListBox("%I%T", this);
  playlist->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
  playlist->setFrameStyle(QFrame::Sunken);
  playlist->setFrameShape(QFrame::Panel);
  playlist->setLineWidth(1);
#else
  playlist = new QListWidget(this);
  playlist->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
  playlist->setFrameStyle(QFrame::Plain | QFrame::StyledPanel);
  playlist->setIconSize(QSize(22, 22));
  playlist->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  playlist->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  playlist->setItemDelegate(new QtopiaItemDelegate);
#endif
  mainLayout->addWidget(playlist);
}

GeneralUI::~GeneralUI() {}

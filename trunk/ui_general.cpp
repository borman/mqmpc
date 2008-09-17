#include "ui_general.h"

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

GeneralUI::GeneralUI(QWidget *parent)
: QFrame(parent)
{
  setFrameStyle(QFrame::NoFrame);
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  mainLayout->addSpacing(5);

  nowPlaying = new QLabel(this);
  nowPlaying->setAlignment(Qt::AlignVCenter);
  nowPlaying->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
  mainLayout->addWidget(nowPlaying);

  progressBar = new QProgressBar(this);
  progressBar->setRange(0, UI_PROGRESSBAR_SIZE);
  progressBar->reset();

  mainLayout->addWidget(progressBar);

  mainLayout->addSpacing(5);

  QFrame *buttonBarWidget = new QFrame(this);
  QHBoxLayout *buttonBarLayout = new QHBoxLayout(buttonBarWidget);

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

  buttonBarLayout->addWidget(prevButton);
  buttonBarLayout->addWidget(stopButton);
  buttonBarLayout->addWidget(playButton);
  buttonBarLayout->addWidget(nextButton);
  mainLayout->addWidget(buttonBarWidget);

  mainLayout->addSpacing(5);

  playlist = new QListWidget(this);
  playlist->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
  playlist->setFrameStyle(QFrame::Plain | QFrame::StyledPanel);
  playlist->setIconSize(QSize(22, 22));
  playlist->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  playlist->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  playlist->setItemDelegate(new QtopiaItemDelegate);

  mainLayout->addWidget(playlist);
}

GeneralUI::~GeneralUI() {}

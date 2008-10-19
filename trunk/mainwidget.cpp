#include "mainwidget.h"

#include <cstdio>
#include <cstring>

#include "libmpdclient.h"

#include <QSoftMenuBar>
#include <QMenu>
#include <QApplication>
#include <QListWidgetItem>
#include <QFile>
#include <QTextStream>
#include <QDialog>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QtopiaApplication>
#include <QFormLayout>

//#define MPDCLIENT_DEBUG

#ifdef MPDCLIENT_DEBUG
#define MPD(x) x; checkConnection(); printf(#x "\n")
#else
#define MPD(x) x; checkConnection()
#endif

#define NRETRIES 0

MainWidget::MainWidget(QWidget *parent, Qt::WindowFlags)
  : QStackedWidget(parent), general_ui(NULL), collection(NULL)
{
  QCoreApplication::setOrganizationName("Borman");
  QCoreApplication::setApplicationName("mqmpc");

  setWindowTitle("mqmpc");

  general_ui = new GeneralUI(this);

  addWidget(general_ui);
  setCurrentWidget(general_ui);
  mainMenu = QSoftMenuBar::menuFor(general_ui);
  mainMenu->addAction(tr("Connect"), this, SLOT(doConnect()));
  mainMenu->addAction(tr("Disconnect"), this, SLOT(doDisconnect()));
  mainMenu->addAction(tr("Set MPD host"), this, SLOT(showHostInputDialog()));
  mainMenu->addAction(tr("Collection"), this, SLOT(showCollectionDialog()));
  mainMenu->addSeparator();
  mainMenu->addAction(tr("Exit"), this, SLOT(close()));

  connect(general_ui->prevButton, SIGNAL(clicked()), SLOT(prevSong()));
  connect(general_ui->nextButton, SIGNAL(clicked()), SLOT(nextSong()));
  connect(general_ui->playButton, SIGNAL(clicked()), SLOT(playPause()));
  connect(general_ui->stopButton, SIGNAL(clicked()), SLOT(stopPlayback()));

  connect(&statusTimer, SIGNAL(timeout()), SLOT(updateStatus()));
  connect(general_ui->playlist, SIGNAL(itemActivated(QListWidgetItem *)), SLOT(switchToSong(QListWidgetItem *)));

  memset(&mpd_status, 0, sizeof(mpd_Status));
  mpd_state = MPD_STATUS_STATE_UNKNOWN;
  nowPlayingNum = -1;
  mpd_playlist = 0;

  QSettings settings;
  savedHost = settings.value("Host/hostname", "127.0.0.1").toString();
  savedPort = settings.value("Host/port", 6600).toInt();

  setConnected(false);
  setHostPort(savedHost, savedPort);
  retries = NRETRIES;
  general_ui->nowPlaying->updateContents(tr("Not connected"));
  QTimer::singleShot(0, this, SLOT(doConnect()));
};

MainWidget::~MainWidget()
{
  if (isConnected)
    doDisconnect();
};

void MainWidget::setHostPort(QString host, int port)
{
  this->host = host;
  this->port = port;
}

void MainWidget::doDisconnect()
{
  if (conn)
  {
    mpd_closeConnection(conn);
    conn = NULL;
    printf("Disconnect()\n");
    setConnected(false);
    general_ui->nowPlaying->updateContents(tr("Not connected"));
    //qApp->processEvents();
  }
}

void MainWidget::doConnect()
{
  if (isConnected)
    return;

  printf("Trying to connect...\n");
  general_ui->nowPlaying->updateContents(QString(tr("Connecting to \"%1\"...")).arg(host));
  qApp->processEvents();

  conn = mpd_newConnection(host.toUtf8().constData(), port, 1);
  if (checkConnection())
  {
    general_ui->nowPlaying->updateContents(tr("Connected"));
    qApp->processEvents();
    printf("Connect()\n");
    setConnected(true);
    updateStatus();
    statusTimer.start(1000);
    //updatePlaylist();
    mpd_playlist = 0;
  }
}

void MainWidget::setConnected(bool state)
{
  printf("Connected==%d\n", state);
  isConnected = state;
  if (!isConnected)
    conn = NULL;
  if (!isConnected)
  {
    general_ui->prevButton->setEnabled(false);
    general_ui->stopButton->setEnabled(false);
    general_ui->playButton->setEnabled(false);
    general_ui->nextButton->setEnabled(false);
    general_ui->playlist->clear();
    general_ui->progressBar->reset();
  }
  else
  {
    retries = NRETRIES;

    general_ui->prevButton->setEnabled(true);
    general_ui->stopButton->setEnabled(true);
    general_ui->playButton->setEnabled(true);
    general_ui->nextButton->setEnabled(true);
  }
}

bool MainWidget::checkConnection()
{
  if (!conn)
  {
    return false;
  }
  if (conn->error)
  {
    printf("MPD connection error\n [error=%d, errorCode=%d, errorAt=%d]\n Reason: \"%s\"\n",
      conn->error, conn->errorCode, conn->errorAt,
      conn->errorStr);
    retries--;
    if (retries<0)
      doDisconnect();
    else
      doConnect();
    return false;
  }
  else
  {
    return true;
  }
}

void MainWidget::prevSong()
{
  if (!isConnected)
    return;
  MPD(mpd_sendPrevCommand(conn));
  MPD(mpd_finishCommand(conn));
}

void MainWidget::nextSong()
{
  if (!isConnected)
    return;
  MPD(mpd_sendNextCommand(conn));
  MPD(mpd_finishCommand(conn));
}

void MainWidget::playPause()
{
  if (!isConnected)
    return;
  switch (mpd_status.state)
  {
    case MPD_STATUS_STATE_PAUSE:
    MPD(mpd_sendPauseCommand(conn, 0));
    break;
    case MPD_STATUS_STATE_PLAY:
    MPD(mpd_sendPauseCommand(conn, 1));
    break;
    case MPD_STATUS_STATE_STOP:
    MPD(mpd_sendPlayCurrentCommand(conn));
    break;
  }
  if (mpd_status.state!=MPD_STATUS_STATE_UNKNOWN)
    MPD(mpd_finishCommand(conn));
}

void MainWidget::stopPlayback()
{
  if (!isConnected)
    return;
  MPD(mpd_sendStopCommand(conn));
  MPD(mpd_finishCommand(conn));
}

void MainWidget::switchToSong(QListWidgetItem *item)
{
  if (!isConnected)
    return;
  MPD(mpd_sendPlayCommand(conn, general_ui->playlist->row(item)));
  MPD(mpd_finishCommand(conn));
}

void MainWidget::volumeDown()
{
  if (!isConnected)
    return;
  MPD(mpd_sendCVolumeCommand(conn, "-5"));
  MPD(mpd_finishCommand(conn));
}

void MainWidget::volumeUp()
{
  if (!isConnected)
    return;
  MPD(mpd_sendCVolumeCommand(conn, "+5"));
  MPD(mpd_finishCommand(conn));
}

void MainWidget::updateStatus()
{
  if (!isConnected)
  {
    statusTimer.stop();
    return;
  }
  MPD(mpd_sendStatusCommand(conn));
  if (!isConnected)
    return;
  MPD(mpd_Status *tmp = mpd_getStatus(conn));
  if (tmp)
    memcpy(&mpd_status, tmp, sizeof(mpd_Status));
  mpd_freeStatus(tmp);

  if (mpd_status.song!=nowPlayingNum || mpd_status.playlist!=mpd_playlist)
  {
    if (nowPlayingNum>=0)
    {
      QListWidgetItem *previtem = general_ui->playlist->item(nowPlayingNum);
      if (previtem)
        previtem->setIcon(loadMultiIcon("media-playlist-none"));
    }
    QListWidgetItem *item = general_ui->playlist->item(mpd_status.song);
    if (item)
    {
      item->setIcon(loadMultiIcon("media-playlist-current"));
      general_ui->playlist->scrollToItem(item);
    }

    // Update current song
    MPD(mpd_sendCurrentSongCommand(conn));
    MPD(mpd_nextListOkCommand(conn));
    MPD(mpd_InfoEntity *tmp2 = mpd_getNextInfoEntity(conn));

    if (tmp2 && tmp2->type==MPD_INFO_ENTITY_TYPE_SONG)
    {
      mpd_Song *mpd_song = tmp2->info.song;
      QString np_title;
      QString np_artist;
      QString np_album;
      if (mpd_song->title)
        np_title = QString::fromUtf8(mpd_song->title).simplified();
      else
        np_title = QString::fromUtf8(mpd_song->file).simplified();
      if (mpd_song->artist)
        np_artist = QString::fromUtf8(mpd_song->artist).simplified();
      if (mpd_song->album)
        np_album = QString::fromUtf8(mpd_song->album).simplified();

      general_ui->nowPlaying->updateContents(np_title, np_artist, np_album);
      mpd_freeInfoEntity(tmp2);
    }

    nowPlayingNum = mpd_status.song;
  }

  if (mpd_status.state==MPD_STATUS_STATE_PLAY || mpd_status.state == MPD_STATUS_STATE_PAUSE)
    general_ui->progressBar->setValue((mpd_status.elapsedTime*UI_PROGRESSBAR_SIZE)/mpd_status.totalTime);
  else
    general_ui->progressBar->reset();

  if (mpd_status.state==MPD_STATUS_STATE_PLAY && mpd_state!=MPD_STATUS_STATE_PLAY)
  {
    general_ui->playButton->setIcon(loadMultiIcon("media-playback-pause"));
  }
  else
  if (mpd_status.state!=MPD_STATUS_STATE_PLAY && mpd_state==MPD_STATUS_STATE_PLAY)
  {
    general_ui->playButton->setIcon(loadMultiIcon("media-playback-start"));
  }
  if (mpd_status.state==MPD_STATUS_STATE_STOP && mpd_state!=MPD_STATUS_STATE_STOP)
    general_ui->stopButton->setEnabled(false);
  if (mpd_status.state!=MPD_STATUS_STATE_STOP && mpd_state==MPD_STATUS_STATE_STOP)
    general_ui->stopButton->setEnabled(true);
  mpd_state = mpd_status.state;

  if (mpd_status.playlist!=mpd_playlist)
  {
    updatePlaylist();
    mpd_playlist = mpd_status.playlist;
  }
}

void  MainWidget::updatePlaylist()
{
  if (!isConnected)
    return;
  general_ui->playlist->clear();

  MPD(mpd_sendPlaylistInfoCommand(conn, -1));

  if (!isConnected)
    return;

  MPD(mpd_nextListOkCommand(conn));
  mpd_InfoEntity *data;
  while (data = mpd_getNextInfoEntity(conn))
  {
    if (data->type==MPD_INFO_ENTITY_TYPE_SONG)
    {
      mpd_Song *song = data->info.song;
      QString pl_str;
      //if (song->track)
      //  pl_str += QString::fromUtf8(song->track) + ". ";
      if (song->title)
        pl_str += QString::fromUtf8(song->title).simplified();
      else
        pl_str += QString::fromUtf8(song->file).simplified();

      QListWidgetItem *item = new QListWidgetItem(pl_str, general_ui->playlist);
      if (song->pos==nowPlayingNum)
        item->setIcon(loadMultiIcon("media-playlist-current"));
      else
        item->setIcon(loadMultiIcon("media-playlist-none"));
      general_ui->playlist->addItem(item);
    }
    else
      printf("wrong type\n");
    mpd_freeInfoEntity(data);
  }
  general_ui->playlist->scrollToItem(general_ui->playlist->item(nowPlayingNum));
}

void MainWidget::showHostInputDialog()
{
  QDialog *inputDialog = new QDialog(this);
  inputDialog->setWindowTitle(tr("MPD host"));
  QSoftMenuBar::setLabel(inputDialog, Qt::Key_Back, QSoftMenuBar::Ok);

  QLineEdit *hostInput = new QLineEdit(inputDialog);
  QLineEdit *portInput = new QLineEdit(inputDialog);
  portInput->setInputMask("00009");

  QFormLayout *inputLayout = new QFormLayout(inputDialog);
  inputLayout->addRow(tr("Host"), hostInput);
  inputLayout->addRow(tr("Port"), portInput);

  hostInput->setText(savedHost);
  portInput->setText(QString::number(savedPort));

  if (QtopiaApplication::execDialog(inputDialog)  == QDialog::Accepted)
  {
    QString newHost = hostInput->text().trimmed();
    QString newPort = portInput->text().trimmed();
    if (!newHost.isEmpty())
    {
      savedHost = newHost;
      if (!newPort.isEmpty())
      {
        savedPort = newPort.toInt();
      }
      QSettings settings;
      settings.setValue("Host/hostname", savedHost);
      settings.setValue("Host/port", savedPort);
      setHostPort(savedHost);
    }
  }
}

QStringList MainWidget::fetchArtistList()
{
  MPD(mpd_sendListCommand(conn, MPD_TABLE_ARTIST, NULL));
  QStringList artistList;
  char *artist;
  while((artist = mpd_getNextArtist(conn)))
  {
    artistList << QString::fromUtf8(artist);
    free(artist);
  }
  MPD(mpd_finishCommand(conn));
  return artistList;
}

QStringList MainWidget::fetchAlbumList(QString artist)
{
  MPD(mpd_sendListCommand(conn, MPD_TABLE_ALBUM, artist.toUtf8().constData()));
  QStringList albumList;
  char *album;
  while((album = mpd_getNextAlbum(conn)))
  {
    albumList << QString::fromUtf8(album);
    free(album);
  }
  MPD(mpd_finishCommand(conn));
  return albumList;
}

QStringList MainWidget::fetchTitleList(QString artist, QString album)
{
  /* // Not correct
  MPD(mpd_sendListCommand(conn, MPD_TABLE_TITLE, album.toUtf8().constData()));
  QStringList titleList;
  char *title;
  while((title = mpd_getNextTitle(conn)))
  {
    titleList << QString::fromUtf8(title);
    free(title);
  }
  MPD(mpd_finishCommand(conn));
  return titleList;
  */
  if (!isConnected)
    return QStringList();
  MPD(mpd_startSearch(conn, 1));
  if (!artist.isEmpty())
  {
    MPD(mpd_addConstraintSearch(conn, MPD_TAG_ITEM_ARTIST, artist.toUtf8().constData()));
  }
  if (!album.isEmpty())
  {
    MPD(mpd_addConstraintSearch(conn, MPD_TAG_ITEM_ALBUM, album.toUtf8().constData()));
  }
  MPD(mpd_commitSearch(conn));

  QStringList titleList;
  mpd_InfoEntity *entity;
  while(entity = mpd_getNextInfoEntity(conn))
  {
    if(entity->type!=MPD_INFO_ENTITY_TYPE_SONG)
    {
      mpd_freeInfoEntity(entity);
      continue;
    }
    mpd_Song *song = entity->info.song;
    titleList << QString::fromUtf8(song->title);
    mpd_freeInfoEntity(entity);
  }
  MPD(mpd_finishCommand(conn));
  return titleList;
}

void MainWidget::showCollectionDialog()
{
  if (!isConnected)
    return;
  if (!collection)
  {
    collection = new CollectionDialog(this);
    connect(collection, SIGNAL(itemSelected(QString, int)),
            SLOT(collectionItemSelected(QString, int)));
  }

  collection->feedList(fetchArtistList(), CollectionDialog::Global);
  QtopiaApplication::showDialog(collection);
}

void MainWidget::collectionItemSelected(const QString &str, int role)
{
  switch(role)
  {
    case CollectionDialog::Artist:
    {
      collectionArtist = str;
      QStringList albumList = fetchAlbumList(str);
      albumList.prepend("..");
      collection->feedList(albumList, CollectionDialog::Artist);
    } break;
    case CollectionDialog::Album:
    {
       if (str=="..")
         collection->feedList(fetchArtistList(), CollectionDialog::Global);
       else
       {
         collectionAlbum = str;
         QStringList titleList = fetchTitleList(collectionArtist, str);
         titleList.prepend("..");
         collection->feedList(titleList, CollectionDialog::Album);
       }
    } break;
    case CollectionDialog::Title:
    {
      if (str=="..")
      {
        QStringList albumList = fetchAlbumList(collectionArtist);
        albumList.prepend("..");
        collection->feedList(albumList, CollectionDialog::Artist);
      }
      else
      {
        printf("Selected title \"%s\"\n", str.toUtf8().constData());
        addToPlaylist(collectionArtist, collectionAlbum, str);
        collection->showInfo(tr("\"%1\"").arg(str));
      }
    } break;
  }
}

void MainWidget::clearPlaylist()
{
  if (!isConnected)
    return;
  MPD(mpd_sendClearCommand(conn));
  MPD(mpd_finishCommand(conn));
}

void MainWidget::addToPlaylist(const QString &artist, const QString &album, const QString &title)
{
  if (!isConnected)
    return;
  MPD(mpd_startSearch(conn, 1));
  if (!artist.isEmpty())
  {
    MPD(mpd_addConstraintSearch(conn, MPD_TAG_ITEM_ARTIST, artist.toUtf8().constData()));
  }
  if (!album.isEmpty())
  {
    MPD(mpd_addConstraintSearch(conn, MPD_TAG_ITEM_ALBUM, album.toUtf8().constData()));
  }
  if (!title.isEmpty())
  {
    MPD(mpd_addConstraintSearch(conn, MPD_TAG_ITEM_TITLE, title.toUtf8().constData()));
  }
  MPD(mpd_commitSearch(conn));

  QStringList fileList;
  mpd_InfoEntity *entity;
  while(entity = mpd_getNextInfoEntity(conn))
  {
    if(entity->type!=MPD_INFO_ENTITY_TYPE_SONG)
    {
      mpd_freeInfoEntity(entity);
      continue;
    }
    mpd_Song *song = entity->info.song;
    fileList << QString::fromUtf8(song->file);
    mpd_freeInfoEntity(entity);
  }
  MPD(mpd_finishCommand(conn));
  for (QStringList::const_iterator it=fileList.constBegin(); it!=fileList.constEnd(); it++)
  {
    QString file = *it;
    printf("\"%s\" matches\n", file.toUtf8().constData());
    MPD(mpd_sendAddCommand(conn, file.toUtf8().constData()));
    MPD(mpd_finishCommand(conn));
  }
}



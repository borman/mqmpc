#include "mainwidget.h"

#include <cstdio>
#include <cstring>

#include "libmpdclient.h"

#ifdef EZX
#include <qpopupmenu.h>
#include <ZMessageBox.h>
#include <ZApplication.h>
#include <ZPushButton.h>
#include <qfile.h>
#include <qtextstream.h>
#include <UTIL_Dialog.h>
#include <qlineedit.h>
#include <UTIL_DlgCST.h>

#define trimmed() stripWhiteSpace()
#define simplified() simplifyWhiteSpace()

#else
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
#endif

//#define MPDCLIENT_DEBUG

#ifdef MPDCLIENT_DEBUG
#define MPD(x) x; checkConnection(); printf(#x "\n")
#else
#define MPD(x) x; checkConnection()
#endif

#define NRETRIES 0

#ifdef EZX
MainWidget::MainWidget()
    : ZMainWidget(0, "EZMPC", 0)
#else
MainWidget::MainWidget(QWidget *parent, Qt::WindowFlags)
  : QStackedWidget(parent), general_ui(NULL), collection(NULL)
#endif
{
  QCoreApplication::setOrganizationName("Borman");
  QCoreApplication::setApplicationName("mqmpc");

  general_ui = new GeneralUI(this);
#ifdef EZX
  UTIL_CST *cst = new UTIL_CST(this, "mqmpc");
  setCSTWidget(cst);
  connect(cst->getRightBtn(), SIGNAL(clicked()), SLOT(close()));
  setContentWidget(general_ui);

  mainMenu = new QPopupMenu;
  mainMenu->insertItem("Connect", this, SLOT(doConnect()));
  mainMenu->insertItem("Disconnect", this, SLOT(doDisconnect()));
  mainMenu->insertItem("Set MPD host", this, SLOT(showHostInputDialog()));
  mainMenu->insertSeparator();
  mainMenu->insertItem("Exit", this, SLOT(close()));
  cst->getLeftBtn()->setPopup(mainMenu);
#else
  addWidget(general_ui);
  setCurrentWidget(general_ui);
  mainMenu = QSoftMenuBar::menuFor(general_ui);
  mainMenu->addAction(tr("Connect"), this, SLOT(doConnect()));
  mainMenu->addAction(tr("Disconnect"), this, SLOT(doDisconnect()));
  mainMenu->addAction(tr("Set MPD host"), this, SLOT(showHostInputDialog()));
  mainMenu->addAction(tr("Collection"), this, SLOT(showCollectionDialog()));
  //mainMenu->insertSeparator();
  mainMenu->addAction(tr("Exit"), this, SLOT(close()));
#endif

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

#ifdef EZX
  QFile hostFile("mqmpc.host");
  savedHost = "127.0.0.1";

  if (hostFile.open(IO_ReadOnly))
  {
    QTextStream in(&hostFile);
    in >> savedHost;
    savedHost = savedHost.trimmed();
  }
#else
  QSettings settings;
  savedHost = settings.value("Host/hostname", "127.0.0.1").toString();
  savedPort = settings.value("Host/port", 6600).toInt();
#endif

  setConnected(false);
  setHostPort(savedHost, savedPort);
  retries = NRETRIES;
  general_ui->nowPlaying->setText(tr("Not connected"));
  QTimer::singleShot(0, this, SLOT(doConnect()));

  installEventFilter(this);
  general_ui->playlist->installEventFilter(this);
};

MainWidget::~MainWidget()
{
  if (isConnected)
    doDisconnect();
#ifdef EZX
  QFile hostFile("mqmpc.host");
  if (hostFile.open(IO_WriteOnly | IO_Truncate))
  {
    QTextStream out(&hostFile);
    out << savedHost.trimmed();
  }
#endif
};


bool MainWidget::eventFilter( QObject *o, QEvent *e )
{
#ifdef EZX
  if ( e->type() == QEvent::KeyPress )    // key press
  {
    QKeyEvent *k = (QKeyEvent*)e;
    printf("key %d\n", k->key());
    if (k->key() == 4115)
    {
      volumeUp();
      return true;
    }
    if (k->key() == 4117)
    {
      volumeDown();
      return true;
    }
    if (k->key() == 4101)
    {
      playPause();
      return true;
    }
  }
#else
#endif
  return false;
}

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
    general_ui->nowPlaying->setText(tr("Not connected"));
    //qApp->processEvents();
  }
}

void MainWidget::doConnect()
{
  if (isConnected)
    return;

  printf("Trying to connect...\n");
  general_ui->nowPlaying->setText(QString(tr("Connecting to \"%1\"...")).arg(host));
  qApp->processEvents();
#ifdef EZX
  conn = mpd_newConnection(host.utf8(), port, 1);
#else
  conn = mpd_newConnection(host.toUtf8().constData(), port, 1);
#endif
  if (checkConnection())
  {
    general_ui->nowPlaying->setText(tr("Connected"));
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
#ifdef EZX
    general_ui->progressBar->setProgress(0);
#else
    general_ui->progressBar->reset();
#endif
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

  if (mpd_status.song!=nowPlayingNum)
  {
#ifdef EZX
    if (nowPlayingNum>=0)
    {
      UTIL_ListBoxItem *previtem = (UTIL_ListBoxItem *)general_ui->playlist->item(nowPlayingNum);
      if (previtem)
        previtem->setPixmap(0, loadGraphics("media-playlist-none"));
    }
    UTIL_ListBoxItem *item = (UTIL_ListBoxItem *)general_ui->playlist->item(mpd_status.song);
    if (item)
    {
      item->setPixmap(0, loadGraphics("media-playlist-current"));
    }
#else
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
#endif
    nowPlayingNum = mpd_status.song;
  }

  MPD(mpd_sendCurrentSongCommand(conn));
  MPD(mpd_nextListOkCommand(conn));
  MPD(mpd_InfoEntity *tmp2 = mpd_getNextInfoEntity(conn));

  if (tmp2 && tmp2->type==MPD_INFO_ENTITY_TYPE_SONG)
  {
    mpd_Song *mpd_song = tmp2->info.song;
    QString np_str;
    if (mpd_song->title)
    {
      np_str = "<big>" + QString::fromUtf8(mpd_song->title).simplified() + "</big>";
      if (mpd_song->artist || mpd_song->album)
        np_str += "<br />";
      if (mpd_song->artist && mpd_song->album)
        np_str += QString("%1 - \"%2\"").arg(QString::fromUtf8(mpd_song->artist).simplified()).arg(QString::fromUtf8(mpd_song->album).simplified());
      else
      {
        if (mpd_song->artist)
          np_str += QString("%1").arg(QString::fromUtf8(mpd_song->artist).simplified());
        else
          np_str += QString("\"%2\"").arg(QString::fromUtf8(mpd_song->album).simplified());
      }
    }
    else
    {
      np_str = mpd_song->file;
    }
    general_ui->nowPlaying->setText(np_str);
    mpd_freeInfoEntity(tmp2);
  }
#ifdef EZX
  if (mpd_status.state==MPD_STATUS_STATE_PLAY || mpd_status.state == MPD_STATUS_STATE_PAUSE)
    general_ui->progressBar->setProgress((mpd_status.elapsedTime*UI_PROGRESSBAR_SIZE)/mpd_status.totalTime);
  else
    general_ui->progressBar->setProgress(0);
#else
  if (mpd_status.state==MPD_STATUS_STATE_PLAY || mpd_status.state == MPD_STATUS_STATE_PAUSE)
    general_ui->progressBar->setValue((mpd_status.elapsedTime*UI_PROGRESSBAR_SIZE)/mpd_status.totalTime);
  else
    general_ui->progressBar->reset();
#endif

  if (mpd_status.state==MPD_STATUS_STATE_PLAY && mpd_state!=MPD_STATUS_STATE_PLAY)
  {
#ifdef EZX
    general_ui->playButton->setNormal(loadGraphics("media-playback-pause"));
    general_ui->playButton->setActive(loadGraphics("media-playback-pauseA"));
    general_ui->playButton->setDisabled(loadGraphics("media-playback-pauseD"));
    general_ui->playButton->reload();
#else
    general_ui->playButton->setIcon(loadMultiIcon("media-playback-pause"));
#endif
  }
  else
  if (mpd_status.state!=MPD_STATUS_STATE_PLAY && mpd_state==MPD_STATUS_STATE_PLAY)
  {
#ifdef EZX
    general_ui->playButton->setNormal(loadGraphics("media-playback-start"));
    general_ui->playButton->setActive(loadGraphics("media-playback-startA"));
    general_ui->playButton->setDisabled(loadGraphics("media-playback-startD"));
    general_ui->playButton->reload();
#else
    general_ui->playButton->setIcon(loadMultiIcon("media-playback-start"));
#endif
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
#ifdef EZX
      UTIL_ListBoxItem *item = new UTIL_ListBoxItem(general_ui->playlist);
      item->setString(1, pl_str);
      if (song->pos==nowPlayingNum)
        item->setPixmap(0, loadGraphics("media-playlist-current"));
      else
        item->setPixmap(0, loadGraphics("media-playlist-none"));
      general_ui->playlist->insertItem(item);
#else
      QListWidgetItem *item = new QListWidgetItem(pl_str, general_ui->playlist);
      if (song->pos==nowPlayingNum)
        item->setIcon(loadMultiIcon("media-playlist-current"));
      else
        item->setIcon(loadMultiIcon("media-playlist-none"));
      general_ui->playlist->addItem(item);
#endif
    }
    else
      printf("wrong type\n");
    mpd_freeInfoEntity(data);
  }
  general_ui->playlist->scrollToItem(general_ui->playlist->item(nowPlayingNum));
}

void MainWidget::showHostInputDialog()
{
#ifdef EZX
  UTIL_Dialog *inputDialog = new UTIL_Dialog(UTIL_Dialog::SmallDialog, true, this);
  inputDialog->setDlgTitle("Enter MPD host:");
  QLineEdit *inputField = new QLineEdit(inputDialog);
  inputField->setText(savedHost);
  inputDialog->setDlgContent(inputField);
  UTIL_DlgCST *cst = new UTIL_DlgCST(inputDialog, UTIL_DlgCST::Cst2a);
  //cst->getBtn1()
  inputDialog->setDlgCst(cst);
  connect(cst->getBtn1(), SIGNAL(clicked()), inputDialog, SLOT(reject()));
  connect(cst->getBtn2(), SIGNAL(clicked()), inputDialog, SLOT(accept()));
  if (inputDialog->exec())
  {
    QString newHost = inputField->text().stripWhiteSpace();
    if (!newHost.isEmpty())
    {
      savedHost = newHost;
      setHostPort(savedHost);
    }
  }
#else
  QDialog *inputDialog = new QDialog(this);
  inputDialog->setWindowTitle(tr("MPD host"));

  QLineEdit *hostInput = new QLineEdit(inputDialog);
  QLineEdit *portInput = new QLineEdit(inputDialog);
  portInput->setInputMask("00009");
  QLabel *hostLabel = new QLabel(tr("Host: "));
  QLabel *portLabel = new QLabel(tr("Port: "));

  QGridLayout *inputLayout = new QGridLayout(inputDialog);
  inputLayout->addWidget(hostLabel, 0, 0);
  inputLayout->addWidget(hostInput, 0, 1);
  inputLayout->addWidget(portLabel, 1, 0);
  inputLayout->addWidget(portInput, 1, 1);
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
#endif
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
        collection->showInfo(tr("added \"%1\"").arg(str));
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



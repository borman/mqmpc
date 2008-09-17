#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#ifdef EZX

#include <ZMainWidget.h>
#include <ZMessageBox.h>
#include <UTIL_CST.h>
#include <qstringlist.h>

#else

#include <QStackedWidget>
#include <QTimer>
#include <QStringList>
#define ZMainWidget QStackedWidget

#endif

#include "libmpdclient.h"

#include "ui_general.h"
#include "collection.h"

class MainWidget : public ZMainWidget
{
  Q_OBJECT
public:
#ifdef EZX
  MainWidget();
#else
  MainWidget(QWidget *parent= 0, Qt::WindowFlags flags= 0);
#endif
  virtual ~MainWidget();
  void setHostPort(QString host = "127.0.0.1", int port = 6600);
public slots:
  void prevSong();
  void nextSong();
  void stopPlayback();
  void playPause();

  void switchToSong(QListWidgetItem *item);

  void volumeUp();
  void volumeDown();

  void updateStatus();
  void updatePlaylist();

  void clearPlaylist();
  void addToPlaylist(const QString &artist, const QString &album, const QString &title);

  void doConnect();
  void doDisconnect();

  void showHostInputDialog();
  void showCollectionDialog();

  void collectionItemSelected(const QString &str, int role);
protected:
  bool eventFilter(QObject *o, QEvent *e);
private:
  bool checkConnection();
  void setConnected(bool state);

  QStringList fetchArtistList();
  QStringList fetchAlbumList(QString artist);
  QStringList fetchTitleList(QString artist, QString album);

  QString host;
  int port;
  mpd_Connection *conn;
  int retries;
  bool isConnected;
  GeneralUI *general_ui;

  mpd_Status mpd_status;
  int mpd_state;
  mpd_Song mpd_song;
  int nowPlayingNum;
  QTimer statusTimer;
  long long mpd_playlist;

  QString savedHost;
  int savedPort;

  CollectionDialog *collection;
  QString collectionAlbum;
  QString collectionArtist;

#ifdef EZX
  QPopupMenu *mainMenu;
#else
  QMenu *mainMenu;
#endif
};

#endif

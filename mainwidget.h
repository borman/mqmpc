#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QStackedWidget>
#include <QTimer>
#include <QStringList>

#include "libmpdclient.h"

#include "ui_general.h"
#include "collection.h"

class MainWidget : public QStackedWidget
{
  Q_OBJECT
public:
  MainWidget(QWidget *parent= 0, Qt::WindowFlags flags= 0);
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

  QMenu *mainMenu;
};

#endif

#ifndef COLLECTION_H
#define COLLECTON_H

#include <QStringList>
#include <QString>
#include <QDialog>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLabel>
#include <QTimer>

#include "qslidelabel.h"

class CollectionDialog: public QDialog
{
  Q_OBJECT
  public:
    enum CollectionRole
    {
      Global,
      Artist,
      Album,
      Title,
      Unknown
    };
    CollectionDialog(QWidget *parent);
    virtual ~CollectionDialog() {};
    void feedList(const QStringList &list, CollectionRole role);
    void showInfo(const QString &info);
  signals:
    void itemSelected(const QString &str, int role);
  private slots:
    void listItemActivated(QListWidgetItem *item);
    void resetInfo();
  private:
    QListWidget *listWidget;
    //QLabel *infoLabel;
    QSlideLabel *infoLabel;
    CollectionRole current_role;
    QTimer infoResetTimer;
};

#endif //COLLECTION_H

#include "collection.h"

#include <QListWidget>
#include <QVBoxLayout>
#include <QtopiaApplication>
#include <QtopiaItemDelegate>
#include <QAction>

CollectionDialog::CollectionDialog(QWidget *parent)
  : QDialog(parent)
{
  setModal(true);
  QVBoxLayout *layout = new QVBoxLayout(this);
  listWidget = new QListWidget(this);
  QtopiaApplication::setStylusOperation(listWidget, QtopiaApplication::RightOnHold);
  listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  listWidget->setFrameShape(QFrame::NoFrame);
  listWidget->setFrameStyle(QFrame::StyledPanel|QFrame::Plain);
  listWidget->setItemDelegate(new QtopiaItemDelegate());
  //infoLabel = new QLabel("", this);
  infoLabel = new QSlideLabel("", this);
  infoResetTimer.setInterval(3000);
  infoResetTimer.setSingleShot(true);
  layout->addWidget(infoLabel);
  layout->addWidget(listWidget);
  connect(listWidget, SIGNAL(itemActivated(QListWidgetItem *)), SLOT(listItemActivated(QListWidgetItem *)));
  connect(&infoResetTimer, SIGNAL(timeout()), SLOT(resetInfo()));
  current_role = Global;

  //QAction *action = new QAction("hahaha!", infoLabel);
  //QAction *action2 = new QAction("Bgg", infoLabel);
  //infoLabel->addAction(action);
  //infoLabel->addAction(action2);
}

void CollectionDialog::listItemActivated(QListWidgetItem *item)
{
  emit itemSelected(item->text(), current_role+1);
}

void CollectionDialog::feedList(const QStringList &list, CollectionRole role)
{
  listWidget->clear();
  current_role = role;
  listWidget->addItems(list);
}

void CollectionDialog::showInfo(const QString &info)
{
  infoLabel->updateText(info);
  infoResetTimer.start();
}

void CollectionDialog::resetInfo()
{
  infoLabel->updateText("");
}

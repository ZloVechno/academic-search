#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "controller.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

signals:
  void search(const QString& name, const QString& type);

public slots:
  void searchRequest();
  void errorSearch(const QString& reason);
  void compliteSearch(const QString& result);
  void closeTab(int index);
  void info(const QString& info);

private:
  Ui::MainWindow* ui;
  Controller* controller;
  void addNewTab(const QString& text);
  QVector<QTextBrowser*> pages;
};

#endif // MAINWINDOW_H

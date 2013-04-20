#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow) {

  ui->setupUi(this);
  this->setWindowTitle("Academic Search");

  this->controller = new Controller(this);
  QList<QString> engines = this->controller->engines();
  this->ui->engineBox->insertItems(1, engines);

  connect(this->ui->okBtn, SIGNAL(clicked()), this, SLOT(searchRequest()));
  connect(this, SIGNAL(search(QString,QString)), this->controller, SLOT(search(QString,QString)), Qt::QueuedConnection);
  connect(this->controller, SIGNAL(complite(QString)), this, SLOT(compliteSearch(QString)));
  connect(this->controller, SIGNAL(error(QString)), this, SLOT(errorSearch(QString)));
  connect(this->controller,  SIGNAL(info(QString)), this, SLOT(info(QString)));
  connect(this->ui->tabs, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
  connect(this->ui->nameEdit, SIGNAL(returnPressed()), this->ui->okBtn, SIGNAL(clicked()));

  connect(this->ui->actionHelp, SIGNAL(triggered()), this->controller, SLOT(getInfo()));
  connect(this->ui->actionAbout, SIGNAL(triggered()), this->controller, SLOT(getAbout()));
  this->controller->getInfo();
}

MainWindow::~MainWindow() {
  delete ui;
  delete this->controller;
}

void MainWindow::searchRequest() {
  QString engine = this->ui->engineBox->currentText();
  QString name = this->ui->nameEdit->text();
  this->ui->nameEdit->setEnabled(false);
  this->ui->okBtn->setEnabled(false);

  emit this->search(name, engine);
}

void MainWindow::errorSearch(const QString &reason) {
  this->addNewTab(reason);
}

void MainWindow::compliteSearch(const QString &result) {
  this->addNewTab(result);
}

void MainWindow::addNewTab(const QString &text) {
  QTextBrowser* textBrowser = new QTextBrowser(this->ui->tabs);
  this->pages.push_back(textBrowser);
  textBrowser->setText(text);
  this->ui->tabs->insertTab(this->ui->tabs->count(), textBrowser , this->ui->nameEdit->text());
  this->ui->tabs->setCurrentIndex(this->ui->tabs->count() - 1);

  this->ui->nameEdit->setEnabled(true);
  this->ui->okBtn->setEnabled(true);
}

void MainWindow::closeTab(int index) {
  delete this->pages.at(index);
  this->pages.remove(index);
}

void MainWindow::info(const QString &info) {
  this->ui->info->setText(info);
}

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QtCore>
#include <QtGui>

class Controller : public QThread {
  Q_OBJECT
public:
  explicit Controller(QObject *parent = 0);
  QList<QString> engines() const;

signals:
  void complite(const QString& result);
  void error(const QString& reason);
  void info(const QString& info);

public slots:
  void search(const QString& name, const QString& type);
  void getInfo();
  void getAbout();

private:
  typedef QMap<QString, QString> TemplateKeys;

  void loadTemplate(const QString& tepmplatePath, const QString& name);
  QString renderTemplate(const QString& templateName, const TemplateKeys& keys);
  QString postName(const QString& name, const QString& type) const;
  QString getGraph(const QList<int>& real, const QList<double>& approx) const;

  QMap<QString, QString> handlers;
  QMap<QString, QString> commands;
  QMap<QString, QString> templates;
  QString workingPath;
};

#endif // CONTROLLER_H

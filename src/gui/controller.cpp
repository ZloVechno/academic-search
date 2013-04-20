#include "controller.h"

Controller::Controller(QObject *parent) :
  QThread(parent) {
  this->workingPath = QDir::current().absoluteFilePath("../../data/");
  this->handlers.insert("Google Scholar", "google");
  this->handlers.insert("Microsoft Bing", "ms");

  this->commands.insert("google", "google.r");
  this->commands.insert("ms", "ms.r");

  this->loadTemplate(this->workingPath + "person.html", "person");
  this->loadTemplate(this->workingPath + "error.html", "error");
  this->loadTemplate(this->workingPath + "fatal.html", "fatal");
  this->loadTemplate(this->workingPath + "info.html", "info");
  this->loadTemplate(this->workingPath + "about.html", "about");
}

QList<QString> Controller::engines() const {
  return this->handlers.keys();
}

void Controller::search(const QString &name, const QString &type) {
  QString engine = this->handlers[type];
  TemplateKeys keys;
  keys.insert("engine", engine);
  keys.insert("service", type);
  QString postName = this->postName(name, engine);
  emit info("Search by name " + postName);
  keys.insert("name", postName);

  QFile inputs(this->workingPath + "input.txt");
  inputs.open(QIODevice::WriteOnly);
  if (engine == "google") {
    inputs.write(QString("\"" + postName + "\"").toUtf8());
  } else {
    inputs.write(QString(postName).toUtf8());
  }
  inputs.close();

  QProcess worker;
  worker.setWorkingDirectory(this->workingPath);
  worker.start("R", QStringList() << "CMD" << "BATCH" << this->commands[engine]);
  worker.waitForFinished(1000 * 60 * 3);
  inputs.remove();

  if (worker.exitStatus() != QProcess::NormalExit) {
    keys.insert("reason", "Timeout.");
    emit this->error(this->renderTemplate("error", keys));
    return;
  }

  QFile outputs(this->workingPath + "output.txt");
  outputs.open(QIODevice::ReadOnly);
  QStringList out = QString(outputs.readAll()).split("\n");
  if (!outputs.isOpen() || !outputs.isReadable() || out.length() < 5) {
    QString reason;
    QFile rout(this->workingPath + this->commands[engine] + ".Rout");
    if (!rout.open(QIODevice::ReadOnly)) {
      reason = QString::fromUtf8(worker.readAll().data());
    } else {
      reason = QString(rout.readAll());
    }
    keys.insert("reason", reason);
    emit this->error(this->renderTemplate("fatal", keys));
    outputs.close();
    outputs.remove();
    QFile(this->workingPath + this->commands[engine] + ".Rout").remove();
    return;
  }
  QFile(this->workingPath + this->commands[engine] + ".Rout").remove();

  outputs.close();
  outputs.remove();

  try {
    int articlesCount = out.at(0).toInt();
    if (articlesCount == 0) {
      emit this->error(this->renderTemplate("error", keys));
      return;
    }

    int processArticles = out.at(1).toInt();
    bool isApprox = out.at(2).toInt() == 1;
    int h_index = out.at(3).toInt();
    int years = out.at(4).toInt();
    int i;
    QList<int> citations;
    for (i = 0; i < processArticles && 5 + i < out.length(); i++) {
      citations.push_back(out.at(5 + i).toInt());
    }
    QList<double> approxs;
    if (isApprox) {
      for (i = 0; i < processArticles && 5 + processArticles + i < out.length(); i++) {
        approxs.push_back(out.at(5 + processArticles + i).toDouble());
      }
    }
    QString graph_path = this->getGraph(citations, approxs);

    keys.insert("articles", QString::number(articlesCount));
    keys.insert("hindex", QString::number(h_index));
    keys.insert("years", QString::number(years));
    keys.insert("process", QString::number(processArticles));
    keys.insert("graph", graph_path);

    if (citations.at(0) == 0) {
      keys.insert("reason", "Can not found.");
      emit this->error(this->renderTemplate("error", keys));
    } else {
      emit this->complite(this->renderTemplate("person", keys));
    }
  } catch (...) {
    keys.insert("reason", "Bad output file.");
    emit error(this->renderTemplate("fatal", keys));
  }
}

void Controller::loadTemplate(const QString &templatePath, const QString &name) {
  try {
    QFile temp(templatePath);
    temp.open(QIODevice::ReadOnly);
    this->templates.insert(name, QString(temp.readAll()));
    temp.close();
  } catch (...) {
    emit this->error(QString("Controller can not load template file: %1!").arg(templatePath));
  }
}

QString Controller::postName(const QString &name, const QString &type) const {
  QStringList nameParts = name.simplified().split(" ");
  if (type == "google") {
    int i = 0;
    for (i = 0; i < nameParts.length() - 1; i++) {
      nameParts[i] = nameParts[i].length() > 0 ? QString(nameParts[i][0]) : "";
    }
    QString lastName = nameParts.takeLast();
    return nameParts.join("") + " " + lastName;
  } else if (type == "ms" && nameParts.length() == 3) {
    nameParts[1] = QString(nameParts[1][0]);
  }
  return nameParts.join(" ");
}

QString Controller::renderTemplate(const QString &templateName, const QMap<QString, QString> &keys) {
  QString rendered = this->templates[templateName];
  for (TemplateKeys::ConstIterator it = keys.constBegin(); it != keys.constEnd(); ++it) {
    rendered = rendered.replace("[" + it.key() + "]", it.value());
  }
  return rendered;
}

QString Controller::getGraph(const QList<int> &real, const QList<double> &approx) const {
  QFile csv(this->workingPath + "approx.csv");
  csv.open(QIODevice::WriteOnly);
  QStringList realStr, approxStr;
  for (QList<int>::ConstIterator it = real.constBegin(); it != real.constEnd(); ++it) {
    realStr.push_back(QString::number(*it));
  }

  for (QList<double>::ConstIterator it = approx.constBegin(); it != approx.constEnd(); ++it) {
    approxStr.push_back(QString::number(*it));
  }

  csv.write(QString(realStr.join("; ") + "\n").toUtf8());
  csv.write(QString(approxStr.join("; ") + "\n").toUtf8());
  csv.close();

  QProcess worker;
  worker.setWorkingDirectory(this->workingPath);
  worker.start("R", QStringList() << "CMD" << "BATCH" << "graph.r");
  worker.waitForFinished(1000);
  csv.remove();
  if (worker.exitStatus() != QProcess::NormalExit) {
    return "no-graph.jpg";
  } else {
    return "graph.jpg";
  }
}

void Controller::getInfo() {
  emit complite(this->renderTemplate("info", TemplateKeys()));
}

void Controller::getAbout() {
  emit complite(this->renderTemplate("about", TemplateKeys()));
}

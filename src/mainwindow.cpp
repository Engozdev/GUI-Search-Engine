#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(int argc, char* argv[], QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    engine.setHomeDir(argc, argv);
    engine.run();
    auto ext = engine.getExtensions();
    QList<QString> extensions = {"-"};
    for (auto& el: ext) {
        extensions.append(QString::fromStdString(el.string()));
    }
    ui->fileExtensions->addItems(extensions);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_search_clicked()
{
    QString top = ui->querySize->text();
    engine.setTopK(top.toInt());

    QString query = ui->query->text();
    QString ext = ui->fileExtensions->currentText();
    std::vector<std::string> resp = engine.getRequest(query.toStdString(), ext.toStdString());
    QString repr;
    for (int i = 0; i < resp.size(); ++i) {
        if (i % 2 == 0) {
            repr += QString::fromStdString(resp[i]);
        } else {
            if (resp[i][0] != '\t') repr += "\t";
            repr += QString::fromStdString(resp[i]);
            repr += "\n";
        }
        repr += "\n";
    }
    ui->response->setPlainText(repr);
}


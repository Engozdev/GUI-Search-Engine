#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Engine.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(int argc, char* argv[], QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_search_clicked();

private:
    Ui::MainWindow *ui;
    Engine engine;
};
#endif // MAINWINDOW_H

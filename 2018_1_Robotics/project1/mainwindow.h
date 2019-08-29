/* Copyright 2018 HuaTsai */
#ifndef PROJECT1_MAINWINDOW_H_
#define PROJECT1_MAINWINDOW_H_

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

 public slots:
  void Task1();
  void Task2();
  void Task1Sample1();
  void Task1Sample2();
  void Task2Sample1();
  void Task2Sample1_2();
  void Task2Sample2();

 private:
  Ui::MainWindow *ui;
};

#endif  // PROJECT1_MAINWINDOW_H_

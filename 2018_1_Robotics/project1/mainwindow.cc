/* Copyright 2018 HuaTsai */
#include "mainwindow.h"
#include <Eigen/Dense>
#include <string>
#include <vector>
#include "project1.hpp"
#include "ui_mainwindow.h"
#include <QStandardItemModel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::Task1() {
  Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
  T(0, 0) = std::stod(ui->le_nx->text().toUtf8().constData());
  T(1, 0) = std::stod(ui->le_ny->text().toUtf8().constData());
  T(2, 0) = std::stod(ui->le_nz->text().toUtf8().constData());
  T(0, 1) = std::stod(ui->le_ox->text().toUtf8().constData());
  T(1, 1) = std::stod(ui->le_oy->text().toUtf8().constData());
  T(2, 1) = std::stod(ui->le_oz->text().toUtf8().constData());
  T(0, 2) = std::stod(ui->le_ax->text().toUtf8().constData());
  T(1, 2) = std::stod(ui->le_ay->text().toUtf8().constData());
  T(2, 2) = std::stod(ui->le_az->text().toUtf8().constData());
  T(0, 3) = std::stod(ui->le_px->text().toUtf8().constData());
  T(1, 3) = std::stod(ui->le_py->text().toUtf8().constData());
  T(2, 3) = std::stod(ui->le_pz->text().toUtf8().constData());
  auto ans = DoTask1(T);
  ui->lb_variables->setText("Joint Variables (theta 1 to 6) (unit: degree)");
  QStandardItemModel *model = new QStandardItemModel();
  model->setHorizontalHeaderItem(0, new QStandardItem(tr("th1")));
  model->setHorizontalHeaderItem(1, new QStandardItem(tr("th2")));
  model->setHorizontalHeaderItem(2, new QStandardItem(tr("th3")));
  model->setHorizontalHeaderItem(3, new QStandardItem(tr("th4")));
  model->setHorizontalHeaderItem(4, new QStandardItem(tr("th5")));
  model->setHorizontalHeaderItem(5, new QStandardItem(tr("th6")));
  for (int i = 0; i < ans.size(); ++i) {
    for (int j = 0; j < 6; ++j) {
      auto item_deg = ans.at(i) * 180 / M_PI;
      auto item = std::to_string(item_deg(j)).c_str();
      model->setItem(i, j, new QStandardItem(item));
    }
  }
  ui->tableView->setModel(model);
  ui->tableView->show();
}

void MainWindow::Task2() {
  std::vector<double> theta;
  theta.push_back(std::stod(ui->le_th1->text().toUtf8().constData()));
  theta.push_back(std::stod(ui->le_th2->text().toUtf8().constData()));
  theta.push_back(std::stod(ui->le_th3->text().toUtf8().constData()));
  theta.push_back(std::stod(ui->le_th4->text().toUtf8().constData()));
  theta.push_back(std::stod(ui->le_th5->text().toUtf8().constData()));
  theta.push_back(std::stod(ui->le_th6->text().toUtf8().constData()));
  auto ans = DoTask2(theta);
  ui->lb_variables->setText("Cartesian Point (x, y, z, phi, theta, phi)");
  ui->lb_nx->setText(std::to_string(ans(0, 0)).c_str());
  ui->lb_ny->setText(std::to_string(ans(1, 0)).c_str());
  ui->lb_nz->setText(std::to_string(ans(2, 0)).c_str());
  ui->lb_ox->setText(std::to_string(ans(0, 1)).c_str());
  ui->lb_oy->setText(std::to_string(ans(1, 1)).c_str());
  ui->lb_oz->setText(std::to_string(ans(2, 1)).c_str());
  ui->lb_ax->setText(std::to_string(ans(0, 2)).c_str());
  ui->lb_ay->setText(std::to_string(ans(1, 2)).c_str());
  ui->lb_az->setText(std::to_string(ans(2, 2)).c_str());
  ui->lb_px->setText(std::to_string(ans(0, 3)).c_str());
  ui->lb_py->setText(std::to_string(ans(1, 3)).c_str());
  ui->lb_pz->setText(std::to_string(ans(2, 3)).c_str());
  QStandardItemModel *model = new QStandardItemModel();
  model->setHorizontalHeaderItem(0, new QStandardItem(tr("x")));
  model->setHorizontalHeaderItem(1, new QStandardItem(tr("y")));
  model->setHorizontalHeaderItem(2, new QStandardItem(tr("z")));
  model->setHorizontalHeaderItem(3, new QStandardItem(tr("phi")));
  model->setHorizontalHeaderItem(4, new QStandardItem(tr("theta")));
  model->setHorizontalHeaderItem(5, new QStandardItem(tr("phi")));
  Eigen::Matrix3d rot = ans.block(0, 0, 3, 3);
  Eigen::Vector3d angle = rot.eulerAngles(2, 1, 2);
  model->setItem(0, 0, new QStandardItem(std::to_string(ans(0, 3)).c_str()));
  model->setItem(0, 1, new QStandardItem(std::to_string(ans(1, 3)).c_str()));
  model->setItem(0, 2, new QStandardItem(std::to_string(ans(2, 3)).c_str()));
  model->setItem(0, 3, new QStandardItem(std::to_string(angle(0)).c_str()));
  model->setItem(0, 4, new QStandardItem(std::to_string(angle(1)).c_str()));
  model->setItem(0, 5, new QStandardItem(std::to_string(angle(2)).c_str()));
  ui->tableView->setModel(model);
  ui->tableView->show();
}

void MainWindow::Task1Sample1() {
  ui->le_nx->setText("-0.2032");
  ui->le_ny->setText("-0.9485");
  ui->le_nz->setText("-0.2429");
  ui->le_ox->setText("-0.6320");
  ui->le_oy->setText("-0.0624");
  ui->le_oz->setText("0.7724");
  ui->le_ax->setText("-0.7478");
  ui->le_ay->setText("0.3105");
  ui->le_az->setText("-0.5868");
  ui->le_px->setText("0.4072");
  ui->le_py->setText("0.2351");
  ui->le_pz->setText("-0.3465");
}

void MainWindow::Task1Sample2() {
  ui->le_nx->setText("0.9788");
  ui->le_ny->setText("-0.1343");
  ui->le_nz->setText("0.1546");
  ui->le_ox->setText("0.0222");
  ui->le_oy->setText("-0.6808");
  ui->le_oz->setText("-0.7321");
  ui->le_ax->setText("0.2036");
  ui->le_ay->setText("0.7200");
  ui->le_az->setText("-0.6634");
  ui->le_px->setText("0.3083");
  ui->le_py->setText("0.0826");
  ui->le_pz->setText("-0.4171");
}

void MainWindow::Task2Sample1() {
  ui->le_th1->setText("30.000313");
  ui->le_th2->setText("59.989966");
  ui->le_th3->setText("-29.986011");
  ui->le_th4->setText("-80.005295");
  ui->le_th5->setText("-49.997587");
  ui->le_th6->setText("-160.002139");
}

void MainWindow::Task2Sample1_2() {
  ui->le_th1->setText("30.000313");
  ui->le_th2->setText("59.989966");
  ui->le_th3->setText("-29.986011");
  ui->le_th4->setText("99.994705");
  ui->le_th5->setText("49.997587");
  ui->le_th6->setText("19.997861");
}

void MainWindow::Task2Sample2() {
  ui->le_th1->setText("14.998485");
  ui->le_th2->setText("90.004345");
  ui->le_th3->setText("-50.011472");
  ui->le_th4->setText("20.008355");
  ui->le_th5->setText("49.999341");
  ui->le_th6->setText("-59.997067");
}
#pragma once
#include <QMainWindow>

#include <llvmes/NES/CPU.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void Reset();
    void Step();
private:
    Ui::MainWindow *ui;
    llvmes::CPU cpu;
    std::vector<std::uint8_t> memory;
};

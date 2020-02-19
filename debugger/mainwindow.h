#pragma once
#include <QMainWindow>

#include <llvmes/NES/CPU.h>
#include <sstream>
#include <iomanip>

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
    template<typename T>
    std::string toHex(T i)
    {
        std::stringstream stream;
        stream << "$"
               << std::uppercase
               << std::setfill ('0')
               << std::setw(sizeof(T)*2)
               << std::hex << (unsigned)i;
        return stream.str();
    }

    Ui::MainWindow *ui;
    llvmes::CPU cpu;
    std::vector<std::uint8_t> memory;
};

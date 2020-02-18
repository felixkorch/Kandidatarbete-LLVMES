#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QIntValidator>

std::vector<std::uint8_t> program {
        0xA0, 0x0A, // LDY, # 0x0A
        0xE8,       // INX
        0x88,       // DEY
        0xD0, 0xFC, // BNE, Begin
        0xEA        // NOP
};

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
        , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->Button_Reset, SIGNAL (released()), this, SLOT (Reset()));
    connect(ui->Button_Push, SIGNAL (released()), this, SLOT (Step()));

    // Setup memory
    memory = std::vector<std::uint8_t> (0xFFFF);
    memory[0xFFFC] = 0x20;
    memory[0xFFFD] = 0x40;
    std::copy(program.begin(), program.end(), &memory[0x4020]);

    // Setup CPU
    cpu.read = [this](std::uint16_t addr){ return memory[addr]; };
    cpu.write = [this](std::uint16_t addr, std::uint8_t value){ memory[addr] = value; };
    cpu.reset();
}

void MainWindow::Reset()
{
    cpu.reset();

    QMessageBox::information(
            this,
            tr("LLVMES - Debugger"),
            tr("CPU Reset!") );
}

void MainWindow::Step()
{
    cpu.step();
}

MainWindow::~MainWindow()
{
    delete ui;
}


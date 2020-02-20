#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QIntValidator>
#include <QtConcurrent/QtConcurrent>
#include <QtDebug>
#include <QMessageLogger>

#include <string>
#include <iostream>
#include <ios>
#include <sstream>
#include <fstream>

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
        , m_ui(new Ui::MainWindow)
{
    m_ui->setupUi(this);
    connect(m_ui->Button_Reset, SIGNAL (released()), this, SLOT (Reset()));
    connect(m_ui->Button_Push, SIGNAL (released()), this, SLOT (Step()));
    connect(m_ui->Button_Run, SIGNAL (released()), this, SLOT (Run()));
    connect(m_ui->Button_Run_BP, SIGNAL (released()), this, SLOT (RunBP()));

    try {
        m_debugger = std::make_shared<Debugger>("6502_functional_test.bin");
        m_debugger->GetCPU()->regPC = 0x0400;
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

void MainWindow::Run()
{
    bool run = m_debugger->Run([this](){ RunFinished(); });
    if(!run) {
        m_ui->Button_Run->setText("Run");
        return;
    }

    m_ui->Button_Run->setText("Stop");
}

void MainWindow::RunBP()
{
    QString bp_string = m_ui->Edit_Breakpoint->text();

    bool ok;
    std::uint16_t bp_addr = bp_string.toUInt(&ok, 16);

    if(!ok) {
        qWarning("Breakpoint invalid format");
        return;
    }

    bool run = m_debugger->RunWithBP(bp_addr, [this](){ RunFinished(); });
}

void MainWindow::RunFinished()
{
    DisplayRegisters();
}

void MainWindow::Reset()
{
    bool ok = m_debugger->Reset();

    if(!ok)
        return;

    m_debugger->GetCPU()->regPC = 0x0400;
    DisplayRegisters();
}

void MainWindow::DisplayRegisters()
{
    auto cpu = m_debugger->GetCPU();
    m_ui->Label_RegA->setText("A: " + QString::fromStdString(ToHexString(cpu->regA)));
    m_ui->Label_RegX->setText("X: " + QString::fromStdString(ToHexString(cpu->regX)));
    m_ui->Label_RegY->setText("Y: " + QString::fromStdString(ToHexString(cpu->regY)));
    m_ui->Label_RegSP->setText("SP: " + QString::fromStdString(ToHexString(cpu->regSP)));
    m_ui->Label_RegPC->setText("PC: " + QString::fromStdString(ToHexString(cpu->regPC)));
}

void MainWindow::Step()
{
    bool ok = m_debugger->Step();
    if(!ok)
        return;
    DisplayRegisters();
}

MainWindow::~MainWindow()
{
    delete m_ui;
}


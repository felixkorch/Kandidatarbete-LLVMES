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
        , m_good_state(false)
{
    m_ui->setupUi(this);
    connect(m_ui->Button_Reset, SIGNAL (released()), this, SLOT (Reset()));
    connect(m_ui->Button_Push, SIGNAL (released()), this, SLOT (Step()));
    connect(m_ui->Button_Run, SIGNAL (released()), this, SLOT (Run()));
    connect(m_ui->Button_Run_BP, SIGNAL (released()), this, SLOT (RunBP()));
    connect(m_ui->Button_Browse, SIGNAL (released()), this, SLOT (Browse()));

    this->statusBar()->setSizeGripEnabled(false);
    QWidget::setWindowTitle(TITLE);

    m_ui->Label_Loaded->setStyleSheet("QLabel { color : red; }");
    m_ui->Label_Loaded->setVisible(true);

    m_ui->Label_Breakpoint->setStyleSheet("QLabel { margin-left:40px; }");
}

void MainWindow::Browse()
{
    QString path =
        QFileDialog::getOpenFileName(this, "Open a file", "directoryToOpen",
            "Binary files (*.bin)");
    if(path.isNull() || path.isEmpty())
        return;

    try {
        m_debugger = std::make_shared<Debugger>(path.toStdString());
        m_debugger->GetCPU()->regPC = 0x0400;
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    m_ui->Label_Loaded->setStyleSheet("QLabel { color : green; }");
    m_ui->Label_Loaded->setText(QFileInfo(path).fileName());
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
    m_ui->Label_Value_RegA->setText(QString::fromStdString(ToHexString(cpu->regA)));
    m_ui->Label_Value_RegX->setText(QString::fromStdString(ToHexString(cpu->regX)));
    m_ui->Label_Value_RegY->setText(QString::fromStdString(ToHexString(cpu->regY)));
    m_ui->Label_Value_RegSP->setText(QString::fromStdString(ToHexString(cpu->regSP)));
    m_ui->Label_Value_RegPC->setText(QString::fromStdString(ToHexString(cpu->regPC)));

    m_ui->Label_Value_C->setText(QString::fromStdString(ToHexString((bool)cpu->regStatus.C)));
    m_ui->Label_Value_Z->setText(QString::fromStdString(ToHexString((bool)cpu->regStatus.Z)));
    m_ui->Label_Value_I->setText(QString::fromStdString(ToHexString((bool)cpu->regStatus.I)));
    m_ui->Label_Value_D->setText(QString::fromStdString(ToHexString((bool)cpu->regStatus.D)));
    m_ui->Label_Value_B->setText(QString::fromStdString(ToHexString((bool)cpu->regStatus.B)));
    m_ui->Label_Value_U->setText(QString::fromStdString(ToHexString((bool)cpu->regStatus.Unused)));
    m_ui->Label_Value_V->setText(QString::fromStdString(ToHexString((bool)cpu->regStatus.V)));
    m_ui->Label_Value_N->setText(QString::fromStdString(ToHexString((bool)cpu->regStatus.N)));

    m_ui->Label_PrevInst->setText(QString::fromStdString(PrettyPrintPC()));
    //m_ui->Label_CurrInst->setText(QString::fromStdString(PrettyPrintPC()));
    //m_ui->Label_NextInst->setText(QString::fromStdString(PrettyPrintPC()));
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


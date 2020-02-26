#include "mainwindow.h"

#include <llvmes/interpreter/cpu.h>

#include <QAbstractSlider>
#include <QDir>
#include <QFileDialog>
#include <QIntValidator>
#include <QMessageBox>
#include <QMessageLogger>
#include <QScrollBar>
#include <QtConcurrent/QtConcurrent>
#include <QtDebug>
#include <fstream>
#include <ios>
#include <iostream>
#include <sstream>
#include <string>

#include "ui_mainwindow.h"

using namespace llvmes;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_ui(new Ui::MainWindow),
      m_disassembly_view(new DisassemblyView)
{
    m_ui->setupUi(this);
    connect(m_ui->Button_Reset, SIGNAL(released()), this, SLOT(Reset()));
    connect(m_ui->Button_Push, SIGNAL(released()), this, SLOT(Step()));
    connect(m_ui->Button_Run, SIGNAL(released()), this, SLOT(Run()));
    connect(m_ui->Button_Run_BP, SIGNAL(released()), this, SLOT(RunBP()));
    connect(m_ui->Button_Browse, SIGNAL(released()), this, SLOT(Browse()));
    connect(m_ui->Button_Stop, SIGNAL(released()), this, SLOT(Stop()));

    this->statusBar()->setSizeGripEnabled(false);
    QWidget::setWindowTitle(TITLE);

    m_ui->Label_Loaded->setStyleSheet("QLabel { color : red; }");
    m_ui->Label_Loaded->setVisible(true);

    m_ui->Label_Breakpoint->setStyleSheet("QLabel { margin-left:40px; }");

    m_ui->scrollArea->setWidget(m_disassembly_view);
    m_ui->scrollArea->setWidgetResizable(true);
}

void MainWindow::UpdateUI()
{
    auto cpu = m_debugger->GetCPU();
    m_ui->Label_Value_RegA->setText(
        QString::fromStdString(ToHexString(cpu->reg_a)));
    m_ui->Label_Value_RegX->setText(
        QString::fromStdString(ToHexString(cpu->reg_x)));
    m_ui->Label_Value_RegY->setText(
        QString::fromStdString(ToHexString(cpu->reg_y)));
    m_ui->Label_Value_RegSP->setText(
        QString::fromStdString(ToHexString(cpu->reg_sp)));
    m_ui->Edit_PC->setText(QString::fromStdString(ToHexString(cpu->reg_pc)));

    m_ui->Label_Value_C->setText(
        QString::fromStdString(ToHexString((bool)cpu->reg_status.C)));
    m_ui->Label_Value_Z->setText(
        QString::fromStdString(ToHexString((bool)cpu->reg_status.Z)));
    m_ui->Label_Value_I->setText(
        QString::fromStdString(ToHexString((bool)cpu->reg_status.I)));
    m_ui->Label_Value_D->setText(
        QString::fromStdString(ToHexString((bool)cpu->reg_status.D)));
    m_ui->Label_Value_B->setText(
        QString::fromStdString(ToHexString((bool)cpu->reg_status.B)));
    m_ui->Label_Value_U->setText(
        QString::fromStdString(ToHexString((bool)cpu->reg_status.Unused)));
    m_ui->Label_Value_V->setText(
        QString::fromStdString(ToHexString((bool)cpu->reg_status.V)));
    m_ui->Label_Value_N->setText(
        QString::fromStdString(ToHexString((bool)cpu->reg_status.N)));

    m_ui->scrollArea->verticalScrollBar()->setSliderPosition(
        m_disassembly_view->height());

    auto find = m_disassembly.find(cpu->reg_pc);
    if (find == m_disassembly.end())
        return;
    m_ui->Label_Value_NextInstr->setText(QString::fromStdString(find->second));
    m_ui->Label_NextInstr->setText(
        QString::fromStdString(ToHexString(cpu->reg_pc)));
}

void MainWindow::Stop()
{
    m_debugger->Stop();
}

void MainWindow::Browse()
{
    QString path = QFileDialog::getOpenFileName(
        this, "Open a file", "directoryToOpen", "Binary files (*.bin)");
    if (path.isNull() || path.isEmpty())
        return;

    try {
        m_debugger = std::make_shared<Debugger>(path.toStdString());
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    m_ui->Label_Loaded->setStyleSheet("QLabel { color : green; }");
    m_ui->Label_Loaded->setText(QFileInfo(path).fileName());

    connect(m_debugger.get(), &Debugger::Signal_Reset, this,
            &MainWindow::OnReset);
    connect(m_debugger.get(), &Debugger::Signal_RunStart, this,
            &MainWindow::OnRunStart);
    connect(m_debugger.get(), &Debugger::Signal_RunStop, this,
            &MainWindow::OnRunStop);
    connect(m_debugger.get(), &Debugger::Signal_Step, this,
            &MainWindow::OnStep);

    auto cpu = m_debugger->GetCPU();
    cpu->reg_pc = 0x0400;
    m_disassembly = cpu->Disassemble(0x0400, 0xFFFF);

    UpdateUI();
}

void MainWindow::Run()
{
    if (m_debugger->IsRunning())
        return;

    m_debugger->Run();
}

void MainWindow::RunBP()
{
    if (m_debugger->IsRunning())
        return;

    QString bp_string = m_ui->Edit_Breakpoint->text();

    bool ok;
    std::uint16_t bp_addr = bp_string.toUInt(&ok, 16);

    if (!ok) {
        qWarning("Breakpoint invalid format");
        return;
    }

    m_debugger->RunWithBP(bp_addr);
}

void MainWindow::Reset()
{
    if (m_debugger->IsRunning())
        return;

    m_debugger->Reset();
}

void MainWindow::Step()
{
    if (m_debugger->IsRunning())
        return;

    int pc;
    try {
        std::string pc_string = m_ui->Edit_PC->text().toStdString();
        pc = HexStringToInt(pc_string);
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return;
    }
    m_debugger->GetCPU()->reg_pc = pc;
    m_debugger->Step();
}

void MainWindow::OnReset()
{
    m_debugger->GetCPU()->reg_pc = 0x0400;
    UpdateUI();

    // Clears the disassembly view by replacing it with a new object
    DisassemblyView* temp = new DisassemblyView;
    m_ui->scrollArea->setWidget(temp);
    m_disassembly_view = temp;
}

void MainWindow::OnRunStart()
{
    int pc;
    try {
        std::string pc_string = m_ui->Edit_PC->text().toStdString();
        pc = HexStringToInt(pc_string);
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return;
    }
    m_debugger->GetCPU()->reg_pc = pc;
}

void MainWindow::OnRunStop()
{
    UpdateUI();

    auto& cache = m_debugger->GetCache();

    while (cache.size() > 0) {
        std::uint16_t addr = cache.front();

        auto find = m_disassembly.find(addr);
        if (find == m_disassembly.end())
            continue;

        m_disassembly_view->AddLine(m_ui->Label_NextInstr->text(),
                                    m_ui->Label_Value_NextInstr->text());
        cache.pop();
    }
}

void MainWindow::OnStep()
{
    const std::uint16_t pc = m_debugger->GetCPU()->reg_pc;
    auto find = m_disassembly.find(pc);
    if (find == m_disassembly.end())
        return;
    m_disassembly_view->AddLine(m_ui->Label_NextInstr->text(),
                                m_ui->Label_Value_NextInstr->text());

    m_ui->Label_Value_NextInstr->setText(QString::fromStdString(find->second));
    UpdateUI();
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

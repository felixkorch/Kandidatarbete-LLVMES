#pragma once
#include <llvmes/interpreter/cpu.h>

#include <QFutureWatcher>
#include <QMainWindow>
#include <QVBoxLayout>

#include <iomanip>
#include <iostream>
#include <sstream>

#include "debugger.h"
#include "disassemblyview.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

   public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

   private slots:
    // These are events from the user e.g. "Click on Reset"
    void Reset();
    void Step();
    void Run();
    void RunBP();
    void Stop();
    void Browse();

    // These are responses on events from the debugger e.g. "When step is done, update the UI"
    void UpdateUI();
    void OnReset();
    void OnRunStart();
    void OnRunStop();
    void OnStep();

   private:
    Ui::MainWindow* m_ui;
    std::shared_ptr<Debugger> m_debugger;
    llvmes::DisassemblyMap m_disassembly;
    DisassemblyView* m_disassembly_view;

   private:
    static constexpr const char* TITLE = "LLVMES - Debugger";
};

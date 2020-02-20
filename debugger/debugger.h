#pragma once
#include <llvmes/NES/CPU.h>

#include <memory>
#include <string>
#include <functional>

#include <QFutureWatcher>

class Debugger : public QObject {
Q_OBJECT
public:
    Debugger(const std::string& path);

    bool Step();
    bool Reset();
    bool Run(std::function<void()> callback);
    // Run the CPU with a breakpoint set.
    bool RunWithBP(std::uint16_t addr, std::function<void()> callback);

    std::shared_ptr<llvmes::CPU> GetCPU() { return m_cpu; }
private slots:
    // Gets called when the worker thread is done.
    void RunFinished();
private:
    // CPU as shared ptr makes it thread safe.
    std::shared_ptr<llvmes::CPU> m_cpu;
    std::vector<char> m_memory;

    // Tells if the CPU is currently running.
    // Volatile to make it thread safe.
    volatile bool m_running;
    QFutureWatcher<void> m_run_watcher;
    std::function<void()> m_callback;
};

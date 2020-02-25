#pragma once
#include <llvmes/interpreter/cpu.h>

#include <QFutureWatcher>
#include <functional>
#include <iomanip>
#include <map>
#include <memory>
#include <queue>
#include <sstream>
#include <string>

class Debugger : public QObject {
    Q_OBJECT
   public:
    Debugger(const std::string& path);

    void Step();
    void Reset();
    void Run();
    void Stop();
    void RunWithBP(std::uint16_t addr);
    bool IsRunning() { return m_running; }

    std::shared_ptr<llvmes::CPU> GetCPU() { return m_cpu; }
    std::vector<char>& GetMemory() { return m_memory; }
    std::queue<std::uint16_t>& GetCache() { return m_cache; }

   signals:
    void Signal_Reset();
    void Signal_RunStart();
    void Signal_RunStop();
    void Signal_Step();

   private slots:
    // Gets called when the worker thread is done.
    void RunStop();

   private:
    // Adds current PC to cache to track the path
    // Keeps the 10 latest addresses
    constexpr static std::size_t CACHE_SIZE = 10;
    void AddToCache(std::uint16_t addr);

   private:
    // CPU as shared ptr makes it thread safe.
    std::shared_ptr<llvmes::CPU> m_cpu;
    std::vector<char> m_memory;
    std::queue<std::uint16_t> m_cache;

    // Tells if the CPU is currently running.
    // Volatile to make it thread safe.
    volatile bool m_running;
    QFutureWatcher<void> m_run_watcher;
};

#include "debugger.h"

#include <QDebug>
#include <QMainWindow>
#include <QString>
#include <QtConcurrent/QtConcurrent>
#include <fstream>

Debugger::Debugger(const std::string& path)
    : m_cpu(std::make_shared<llvmes::CPU>()),
      m_memory(0x10000),
      m_running(false)
{
    connect(&this->m_run_watcher, SIGNAL(finished()), this,
            SLOT(RunStop()));

    // Setup memory
    std::ifstream in{path, std::ios::binary};
    if (in.fail())
        throw std::runtime_error("Debugger: Couldn't find program!");
    m_memory = std::vector<char>{std::istreambuf_iterator<char>(in),
                                 std::istreambuf_iterator<char>()};

    // Setup CPU
    m_cpu->Read = [this](std::uint16_t addr) { return m_memory[addr]; };
    m_cpu->Write = [this](std::uint16_t addr, std::uint8_t value) {
        m_memory[addr] = value;
    };
    m_cpu->Reset();
}

void Debugger::Step()
{
    if (m_running) {
        qCritical("Debugger: Cant step while CPU is running!");
        return;
    }
    m_cpu->Step();
    emit Signal_Step();
}

void Debugger::Run()
{
    if (m_running) {
        qCritical("Debugger: Cant do this while CPU is running!");
        return;
    }

    m_running = true;
    emit Signal_RunStart();
    qInfo("Debugger: CPU Started");

    auto future = QtConcurrent::run([this]() {
        while (m_running) {
            m_cpu->Step();
            AddToCache(m_cpu->reg_pc);
        }
        qInfo("Debugger: CPU Stopped");
    });

    this->m_run_watcher.setFuture(future);
}

void Debugger::Stop()
{
    m_running = false;
    emit Signal_RunStop();
}

void Debugger::RunWithBP(std::uint16_t addr)
{
    if (m_running) {
        qCritical("Debugger: Cant do this while CPU is running!");
        return;
    }

    m_running = true;
    emit Signal_RunStart();
    qInfo("Debugger: CPU Started");

    auto future = QtConcurrent::run([this, addr]() {
        std::uint16_t pc = m_cpu->reg_pc;
        while (pc != addr && m_running) {
            pc = m_cpu->reg_pc;
            m_cpu->Step();
            AddToCache(pc);
        }
        qInfo("Debugger: Stopped at breakpoint");
        Stop();
    });

    this->m_run_watcher.setFuture(future);
}

void Debugger::RunStop()
{
    emit Signal_RunStop();
}

void Debugger::Reset()
{
    if (m_running) {
        qCritical("Debugger: Can't do this while running");
        return;
    }
    m_cpu->Reset();
    m_running = false;
    qInfo("Debugger: Reset");
    emit Signal_Reset();
}

void Debugger::AddToCache(std::uint16_t addr)
{
    if(m_cache.size() == CACHE_SIZE)
        m_cache.pop();
    m_cache.push(addr);
}

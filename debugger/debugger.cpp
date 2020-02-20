#include "debugger.h"

#include <QDebug>
#include <QMainWindow>
#include <QtConcurrent/QtConcurrent>
#include <QString>

#include <fstream>

Debugger::Debugger(const std::string& path)
{
    connect(&this->m_run_watcher, SIGNAL (finished()), this, SLOT (RunFinished()));

    // Setup memory
    std::ifstream in{ path, std::ios::binary };
    if(in.fail())
        throw std::runtime_error("Debugger: Couldn't find program!");
    m_memory = std::vector<char>{std::istreambuf_iterator<char>(in),std::istreambuf_iterator<char>() };

    // Setup CPU
    m_cpu->read = [this](std::uint16_t addr){ return m_memory[addr]; };
    m_cpu->write = [this](std::uint16_t addr, std::uint8_t value){ m_memory[addr] = value; };
    m_cpu->reset();
}

bool Debugger::Step()
{
    if(m_running) {
        qCritical("Debugger: Cant step while CPU is running!");
        return false;
    }
    m_cpu->step();
    return true;
}

bool Debugger::Run(std::function<void()> callback)
{
    if(m_running) {
        m_running = false;
        return false;
    }

    m_callback = callback;

    m_running = true;
    qInfo("Debugger: CPU Started");

    auto future = QtConcurrent::run([this]() {
        while(m_running)
            m_cpu->step();
    });

    this->m_run_watcher.setFuture(future);
    return true;
}

bool Debugger::RunWithBP(std::uint16_t addr, std::function<void()> callback)
{
    if(m_running) {
        qCritical("Debugger: CPU already running");
        return false;
    }

    m_callback = callback;

    auto future = QtConcurrent::run([this, addr]() {
        auto state = m_cpu->getState();
        while(state.regPC != addr) {
            m_cpu->step();
            state = m_cpu->getState();
        }
        qInfo("Debugger: Stopped at breakpoint");
    });

    this->m_run_watcher.setFuture(future);
    return true;
}

void Debugger::RunFinished()
{
    m_callback();
    qInfo("Debugger: CPU Stopped.");
}

bool Debugger::Reset()
{
    if(m_running) {
        qCritical("Debugger: Can't do this while running");
        return false;
    }
    m_cpu->reset();
    m_running = false;
    return true;
}

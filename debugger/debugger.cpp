#include "debugger.h"

#include <QDebug>
#include <QMainWindow>
#include <QtConcurrent/QtConcurrent>
#include <QString>

#include <fstream>

Debugger::Debugger(const std::string& path)
    : m_cpu(std::make_shared<llvmes::CPU>())
    , m_memory(0x10000)
    , m_running(false)
{
    connect(&this->m_run_watcher, SIGNAL (finished()), this, SLOT (RunFinished()));

    // Setup memory
    std::ifstream in{ path, std::ios::binary };
    if(in.fail())
        throw std::runtime_error("Debugger: Couldn't find program!");
    m_memory = std::vector<char>{std::istreambuf_iterator<char>(in),std::istreambuf_iterator<char>() };

    // Setup CPU
    m_cpu->Read = [this](std::uint16_t addr){ return m_memory[addr]; };
    m_cpu->Write = [this](std::uint16_t addr, std::uint8_t value){ m_memory[addr] = value; };
    m_cpu->Reset();
}

bool Debugger::Step()
{
    if(m_running) {
        qCritical("Debugger: Cant step while CPU is running!");
        return false;
    }
    m_cpu->Step();
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
            m_cpu->Step();
        QThread::msleep(200);
        qInfo("Debugger: CPU Stopped");
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
        std::uint16_t pc = m_cpu->regPC;
        while(pc != addr) {
            pc = m_cpu->regPC;
            m_cpu->Step();
        }
        QThread::msleep(200);
        qInfo("Debugger: Stopped at breakpoint");
    });

    this->m_run_watcher.setFuture(future);
    return true;
}

void Debugger::RunFinished()
{
    m_callback();
}

bool Debugger::Reset()
{
    if(m_running) {
        qCritical("Debugger: Can't do this while running");
        return false;
    }
    m_cpu->Reset();
    m_running = false;
    qInfo("Debugger: Reset");
    return true;
}

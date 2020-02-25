#pragma once
#include <QVBoxLayout>
#include <QWidget>

class DisassemblyView : public QWidget {
    Q_OBJECT
   public:
    explicit DisassemblyView(QWidget* parent = nullptr);
    void AddLine(std::uint16_t addr, const QString& disassembly);

   private:
    QVBoxLayout* m_vbox;
   signals:
};

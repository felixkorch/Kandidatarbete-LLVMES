#pragma once
#include <QVBoxLayout>
#include <QWidget>

class DisassemblyView : public QWidget {
    Q_OBJECT
   public:
    explicit DisassemblyView(QWidget* parent = nullptr);

    void AddLine(const QString& addr, const QString& disassembly);


   private:
    QVBoxLayout* m_vbox;
   signals:
};

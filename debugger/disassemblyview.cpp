#include "disassemblyview.h"

#include <llvmes/interpreter/cpu.h>

#include <QLabel>
#include <QString>

#include "debugger.h"

using namespace llvmes;

DisassemblyView::DisassemblyView(QWidget* parent) : QWidget(parent)
{
    m_vbox = new QVBoxLayout(this);
    m_vbox->setAlignment(Qt::AlignTop);
}

void DisassemblyView::AddLine(const QString& addr, const QString& disassembly)
{
    QWidget* line = new QWidget;
    QHBoxLayout* hbox = new QHBoxLayout(line);


    QLabel* address_label = new QLabel(addr);

    address_label->setTextInteractionFlags(Qt::TextSelectableByMouse);

    QLabel* disassembly_label = new QLabel(disassembly);
    disassembly_label->setSizePolicy(QSizePolicy::QSizePolicy::Maximum,
                                     QSizePolicy::Maximum);

    hbox->addWidget(address_label);
    hbox->addWidget(disassembly_label);

    m_vbox->addWidget(line);
}

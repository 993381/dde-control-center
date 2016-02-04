/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include <QtWidgets/QFrame>
#include <QtGui/QFont>

#include "constants.h"
#include "powermanagement.h"

PowerManagement::PowerManagement(QWidget *parent)
    : QFrame(parent),
      m_firstHSeparator(new DSeparatorHorizontal)
{

    m_batteryIsPresent = false;
    m_batteryPercentage = 0;

    m_powerManagerViewLabel = new QLabel;
    m_powerManagerViewLabel->setStyleSheet(QString("background-color: %1").arg(DCC::BgLightColor.name()));
    m_powerManagerLabel = new QLabel(tr("Power Management"));
    m_powerManagerLabel->sizeHint();
    m_shortSeparatorLine = new QLabel;
    m_shortSeparatorLine->setFixedSize(5, 2);
    m_shortSeparatorLine->setStyleSheet("background-color: rgb(79, 79, 79)");
    m_powerPercentageLabel = new QLabel(QString("%1%").arg(m_batteryPercentage));

    m_powerPercentageLabel->setStyleSheet("font-size: 10pt;");
    m_powerManagerLayout = new QHBoxLayout;
    m_powerManagerLayout->setMargin(0);
    m_powerManagerLayout->setSpacing(0);
    m_powerManagerLayout->addWidget(m_powerManagerLabel);
    m_powerManagerLayout->addWidget(m_shortSeparatorLine);
    m_powerManagerLayout->addWidget(m_powerPercentageLabel);
    m_powerManagerLayout->addStretch();
    m_powerManagerViewLabel->setLayout(m_powerManagerLayout);
    m_powerManagerViewLabel->setFixedWidth(170);

    m_topHeaderLine = new ModuleHeader(m_powerManagerViewLabel);

    topHeaderLayout = new QVBoxLayout;
    topHeaderLayout->setMargin(0);
    topHeaderLayout->setSpacing(0);
    topHeaderLayout->addWidget(m_topHeaderLine);
    topHeaderLayout->addWidget(m_firstHSeparator);
    setLayout(topHeaderLayout);

    initConnection();

}
PowerManagement::~PowerManagement() {
}

void PowerManagement::initConnection() {
    connect(m_topHeaderLine, SIGNAL(resetButtonClicked()), this, SLOT(reset()));
}
void PowerManagement::reset() {
    emit Reset();
}

void PowerManagement::setElectricQuantity(double electricQuantity) {
    m_batteryPercentage = electricQuantity;
    m_powerPercentageLabel->setText(QString("%1%").arg(m_batteryPercentage));
}
void PowerManagement::batteryReservedControl(bool batteryIsPresent) {
    m_batteryIsPresent = batteryIsPresent;
    if (!m_batteryIsPresent) {
        m_shortSeparatorLine->hide();
        m_powerPercentageLabel->hide();
    } else {
        m_shortSeparatorLine->show();
        m_powerPercentageLabel->show();
    }
}

/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "timewidget.h"

#include <libdui/dthememanager.h>
#include <libdui/libdui_global.h>

#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QEvent>
#include <QTimer>
#include <QFontDatabase>

DUI_USE_NAMESPACE

TimeWidget::TimeWidget(QWidget *parent) :
    QFrame(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_normalWidget = new NormalWidget;
    m_editWidget = new EditWidget;

    QVBoxLayout *centeralLayout = new QVBoxLayout;
    centeralLayout->addWidget(m_normalWidget);
    centeralLayout->addWidget(m_editWidget);
    centeralLayout->setSpacing(0);
    centeralLayout->setMargin(0);

    setLayout(centeralLayout);

    m_editWidget->hide();

    connect(m_normalWidget, &NormalWidget::onDoubleClick, this, &TimeWidget::switchToEditMode);
    connect(m_editWidget, &EditWidget::accept, this, &TimeWidget::applyTime);
    connect(m_editWidget, &EditWidget::accept, this, &TimeWidget::switchToNormalMode);
    connect(m_editWidget, &EditWidget::cancel, this, &TimeWidget::switchToNormalMode);
}

void TimeWidget::switchToEditMode()
{
    m_normalWidget->hide();
    m_editWidget->show();

    m_editWidget->updateTime();
}

void TimeWidget::switchToNormalMode()
{
    m_normalWidget->show();
    m_editWidget->hide();

    m_normalWidget->updateDateTime();
}


NormalWidget::NormalWidget(QWidget *parent) :
    QWidget(parent)
{
    int id = QFontDatabase::addApplicationFont(":/fonts/fonts/MavenProLight-200.otf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);
    QFont font(family);

    m_timeLabel = new QLabel;
    m_timeLabel->setAlignment(Qt::AlignCenter);
    m_timeLabel->setObjectName("TimeLabel");
    m_timeLabel->setFont(font);

    m_amOrPm = new QLabel;
    m_amOrPm->setAlignment(Qt::AlignBottom | Qt::AlignVCenter);
    m_amOrPm->setObjectName("TimeAmOrPm");

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addStretch();
    hLayout->addWidget(m_timeLabel);
    hLayout->addWidget(m_amOrPm);
    hLayout->addStretch();

    m_dateLabel = new QLabel;
    m_dateLabel->setAlignment(Qt::AlignCenter);
    m_dateLabel->setObjectName("DateLabel");

    m_tipsLabel = new QLabel;
    m_tipsLabel->setWordWrap(true);
    m_tipsLabel->setAlignment(Qt::AlignCenter);
    m_tipsLabel->setText(tr("Double-click this area to change your time"));
    m_tipsLabel->setObjectName("TipsLabel");
    m_tipsLabel->hide();

    QVBoxLayout *normalLayout = new QVBoxLayout;
    normalLayout->addLayout(hLayout);
    normalLayout->addWidget(m_dateLabel);
    normalLayout->addWidget(m_tipsLabel);
    normalLayout->setContentsMargins(12, 30, 12, 30);

    setFixedHeight(150);
    setLayout(normalLayout);

    // we need adjust time format by m_amOrPm visible property.
    QTimer::singleShot(1, this, SLOT(updateDateTime()));

    D_THEME_INIT_WIDGET(NormalWidget);
}

void NormalWidget::updateDateTime()
{

    const QDateTime datetime = QDateTime::currentDateTime();
    const int hour = datetime.time().hour() % (!m_is24HourFormat ? 12 : 24);
    const QString hourStr = QString("%1").arg(QString::number(hour), 2, '0');
    const QString minuteStr = QString("%1").arg(QString::number(datetime.time().minute()), 2, '0');

    m_timeLabel->setText(QString("%1:%2").arg(hourStr).arg(minuteStr));
    m_dateLabel->setText(datetime.toString(tr("dddd, dd MMMM yyyy")));

    m_amOrPm->setText(datetime.toString("A"));
}

void NormalWidget::enterEvent(QEvent *)
{
    if (!m_editEnable)
        return;

    m_dateLabel->hide();
    m_tipsLabel->show();
}

void NormalWidget::leaveEvent(QEvent *)
{
    updateDateTime();
    m_dateLabel->show();
    m_tipsLabel->hide();
}

void NormalWidget::mouseDoubleClickEvent(QMouseEvent *)
{
    if (m_editEnable)
        emit onDoubleClick();
}


EditWidget::EditWidget(QWidget *parent) :
    QWidget(parent)
{
    int id = QFontDatabase::addApplicationFont(":/fonts/fonts/MavenProLight-200.otf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);
    QFont font(family);

    m_spinHour = new TimeSpinBox;
    m_spinHour->setMaxNumber(23);
    m_spinMinute = new TimeSpinBox;
    m_spinMinute->setMaxNumber(59);
    QLabel *timeSplit = new QLabel(":");
    timeSplit->setObjectName("TimeSpliter");
    timeSplit->setFont(font);

    QHBoxLayout *timeLayout = new QHBoxLayout;
    timeLayout->addStretch();
    timeLayout->addWidget(m_spinHour);
    timeLayout->addWidget(timeSplit);
    timeLayout->addWidget(m_spinMinute);
    timeLayout->addStretch();
    timeLayout->setSpacing(8);

    m_setTimeButton = new DLinkButton(tr("Set"));
    m_setTimeButton->setFixedWidth(50);
    m_cancelTimeButton = new DLinkButton(tr("Cancel"));
    m_cancelTimeButton->setFixedWidth(50);
    QLabel *controlSplit = new QLabel("|");
    controlSplit->setObjectName("ControlSpliter");

    QHBoxLayout *controlLayout = new QHBoxLayout;
    controlLayout->addStretch();
    controlLayout->addWidget(m_setTimeButton);
    controlLayout->addWidget(controlSplit);
    controlLayout->addWidget(m_cancelTimeButton);
    controlLayout->addStretch();

    QVBoxLayout *centeralLayout = new QVBoxLayout;
    centeralLayout->addLayout(timeLayout);
    centeralLayout->addLayout(controlLayout);
    centeralLayout->setSpacing(10);

    setLayout(centeralLayout);

    connect(m_setTimeButton, &DLinkButton::clicked, [this] () -> void {
                emit accept(QDateTime(QDate::currentDate(), QTime(m_spinHour->text().toInt(), m_spinMinute->text().toInt())));
            });
    connect(m_cancelTimeButton, &DLinkButton::clicked, this, &EditWidget::cancel);

    D_THEME_INIT_WIDGET(EditWidget);
}

void EditWidget::updateTime()
{
    m_spinHour->setNumber(QTime::currentTime().hour());
    m_spinMinute->setNumber(QTime::currentTime().minute());
}


TimeSpinBox::TimeSpinBox(QWidget *parent) :
    QFrame(parent)
{
    int id = QFontDatabase::addApplicationFont(":/fonts/fonts/MavenProLight-200.otf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);
    QFont font(family);

    m_spinLabel = new QLabel;
    m_spinLabel->setFixedSize(80, 80);
    m_spinLabel->setAlignment(Qt::AlignCenter);
    m_spinLabel->setObjectName("SpinLabel");
    m_spinLabel->setFont(font);

    m_addBtn = new DImageButton;
    m_addBtn->setObjectName("SpinAddBtn");
    //m_addBtn->hide();

    m_subBtn = new DImageButton;
    m_subBtn->setObjectName("SpinSubBtn");
    //m_subBtn->hide();

    QVBoxLayout *centeralLayout = new QVBoxLayout;
    centeralLayout->addStretch();
    centeralLayout->addWidget(m_addBtn);
    centeralLayout->setAlignment(m_addBtn, Qt::AlignCenter);
    centeralLayout->addWidget(m_spinLabel);
    centeralLayout->addWidget(m_subBtn);
    centeralLayout->setAlignment(m_subBtn, Qt::AlignCenter);
    centeralLayout->addStretch();
    centeralLayout->setSpacing(0);

    setLayout(centeralLayout);

    connect(m_addBtn, &DImageButton::clicked, this, &TimeSpinBox::addNumber);
    connect(m_subBtn, &DImageButton::clicked, this, &TimeSpinBox::subNumber);

    D_THEME_INIT_WIDGET(TimeSpinBox);
}

void TimeSpinBox::setNumber(int num)
{
    m_spinLabel->setText(QString("%1").arg(QString::number(num), 2, QLatin1Char('0')));
}

void TimeSpinBox::wheelEvent(QWheelEvent *e)
{
    e->accept();
    e->delta() > 0 ? addNumber() : subNumber();
}

void TimeSpinBox::addNumber()
{
    const int num = m_spinLabel->text().toInt();

    if (num < m_maxNumber)
        setNumber(num + 1);
    else
        setNumber(m_minNumber);
}

void TimeSpinBox::subNumber()
{
    const int num = m_spinLabel->text().toInt();

    if (num > m_minNumber)
        setNumber(num - 1);
    else
        setNumber(m_maxNumber);
}

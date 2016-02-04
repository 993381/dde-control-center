/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "applictionitemwidget.h"
#include "constants.h"
#include "helper.h"

#include <QVBoxLayout>
#include <QPixmap>
#include <QMetaObject>

#include <libdui/dthememanager.h>

ApplictionItemWidget::ApplictionItemWidget(QWidget *parent)
    : QFrame(parent)
{
    D_THEME_INIT_WIDGET(ApplictionItemWidget, selected, hovered);

    m_dbusJobManagerInter = new DBusUpdateJobManager("com.deepin.lastore", "/com/deepin/lastore", QDBusConnection::systemBus(), this);

    m_appIcon = new QLabel;
    m_appName = new QLabel;
    m_appName->setStyleSheet("padding:2px 0;");
    m_appName->setObjectName("AppName");
    m_appVersion = new QLabel;
    m_appVersion->setStyleSheet("padding:2px 0;");
    m_appVersion->setObjectName("AppVersion");
    m_progress = new DCircleProgress;
    m_progress->setObjectName("AppProgress");
    m_progress->setStyleSheet(QString());
    m_progress->setFixedSize(25, 25);
    m_progress->setLineWidth(2);
    m_progress->setValue(0);
    m_progress->hide();
    m_updateBtn = new QPushButton;
    m_updateBtn->setText(tr("Update"));
    m_updateBtn->setObjectName("UpdateButton");
    m_updateBtn->hide();
    m_separator = new HSeparatorWidget;

    QVBoxLayout *infoLayout = new QVBoxLayout;
    infoLayout->addStretch();
    infoLayout->addWidget(m_appName);
    infoLayout->addSpacing(0);
    infoLayout->addWidget(m_appVersion);
    infoLayout->addStretch();
    infoLayout->setSpacing(0);
    infoLayout->setMargin(0);

    QHBoxLayout *mainLayout = new QHBoxLayout;
//    mainLayout->addSpacing(10);
    mainLayout->addLayout(infoLayout);
    mainLayout->addStretch();
    mainLayout->addWidget(m_progress);
    mainLayout->addWidget(m_updateBtn);
//    mainLayout->addSpacing(15);
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addLayout(mainLayout);
    rightLayout->addWidget(m_separator);
    rightLayout->setSpacing(0);
    rightLayout->setContentsMargins(10, 0, 12, 0);

    QHBoxLayout *mLayout = new QHBoxLayout;
    mLayout->addSpacing(13);
    mLayout->addWidget(m_appIcon);
    mLayout->addLayout(rightLayout);
    mLayout->setSpacing(0);
    mLayout->setMargin(0);

    setFixedHeight(50);
    setFixedWidth(DCC::ModuleContentWidth);
    setLayout(mLayout);

    connect(m_updateBtn, &QPushButton::clicked, this, &ApplictionItemWidget::toggleUpdateJob);
    connect(m_progress, &DCircleProgress::clicked, this, &ApplictionItemWidget::progressClicked);
}

void ApplictionItemWidget::setAppUpdateInfo(const AppUpdateInfo &info)
{
    m_updateInfo = info;

    const QString &iconPath = getIconPath(info);

    m_appIcon->setPixmap(QPixmap(iconPath).scaled(32, 32, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    m_appName->setText(info.m_name);
    m_appVersion->setText(info.m_avilableVersion);
}

void ApplictionItemWidget::disableUpdate()
{
    qDebug() << "disable update " << m_updateInfo.m_packageId << " = " << m_stat;

    if (m_stat != Ready)
        qWarning() << "disable update on no-ready job";

    m_updateBtn->hide();
    m_disableUpdate = true;
}

void ApplictionItemWidget::connectToJob(DBusUpdateJob *dbusJob)
{
    if (m_dbusJobInter)
        m_dbusJobInter->deleteLater();

    m_dbusJobInter = dbusJob;

    qDebug() << "connect to: " << m_dbusJobInter->packageId();

    // set state to runnning
    if (!m_jobRunning)
        toggleUpdateJob();

    m_dbusJobManagerInter->StartJob(m_dbusJobInter->id());
    connect(m_dbusJobInter, &DBusUpdateJob::ProgressChanged, this, &ApplictionItemWidget::updateJobProgress);
    connect(m_dbusJobInter, &DBusUpdateJob::StatusChanged, this, &ApplictionItemWidget::updateJobStatus);

    // update immeidately
    updateJobProgress();
    updateJobStatus();
}

void ApplictionItemWidget::enterEvent(QEvent *)
{
    if (!m_progress->isVisible() && !m_disableUpdate)
        m_updateBtn->show();
    m_hovered = true;
    emit hoverStateChanged();
}

void ApplictionItemWidget::leaveEvent(QEvent *)
{
    m_updateBtn->hide();
    m_hovered = false;
    emit hoverStateChanged();
}

void ApplictionItemWidget::toggleUpdateJob()
{
    if (m_disableUpdate)
        return;

    m_jobRunning = !m_jobRunning;
    qDebug() << "toggle running = " << m_jobRunning;

    m_updateBtn->setVisible(!m_jobRunning);
    m_progress->setVisible(m_jobRunning);

    if (m_jobRunning && !m_dbusJobInter)
        startJob();
}

void ApplictionItemWidget::startJob()
{
    qDebug() << m_updateInfo.m_packageId;
    QDBusPendingReply<QDBusObjectPath> reply = m_dbusJobManagerInter->UpdatePackage("", m_updateInfo.m_packageId);
    const QDBusObjectPath &jobPath = reply.value();

    DBusUpdateJob *newJob = new DBusUpdateJob("com.deepin.lastore", jobPath.path(), QDBusConnection::systemBus(), this);

    connectToJob(newJob);
}

void ApplictionItemWidget::updateJobProgress()
{
    if (!m_dbusJobInter || !m_dbusJobInter->isValid())
        return;

    if (m_stat == Fail)
        return;

    const double progress = m_dbusJobInter->progress();
    const int percent = int(100 * progress);

    m_progress->setValue(percent);
    m_progress->setText(QString("%1").arg(percent));

    qDebug() << "progress: " << progress << percent;
}

void ApplictionItemWidget::updateJobStatus()
{
    qDebug() << m_dbusJobInter->status();
    const QString &status = m_dbusJobInter->status();
//    const QString &id = m_dbusJobInter->id();

    if (status == "success")
    {
        // CleanJob 由后端处理
//        QMetaObject::invokeMethod(m_dbusJobManagerInter, "CleanJob", Qt::QueuedConnection, Q_ARG(QString, id));
        updateJobStat(Success);
        emit jobFinished();
        return;
    }

    // faild
    if (status == "failed")
        updateJobStat(Fail);
}

void ApplictionItemWidget::updateJobStat(Status stat)
{
    if (m_stat == stat)
        return;

    m_stat = stat;

    switch (m_stat)
    {
    case Ready:     m_progress->topLabel()->clear();        break;
    case Fail:      m_progress->topLabel()->setPixmap(QPixmap(":/images/images/start.png"));        break;
    default:        qDebug() << "Un handled case " << __FILE__ << __LINE__;
    }
}

void ApplictionItemWidget::progressClicked()
{
    qDebug() << "process clicked, stat = " << m_stat;

    switch (m_stat)
    {
    case Fail:      restartJob();        break;
    default:        qDebug() << "Un handled case " << __FILE__ << __LINE__;
    }
}

void ApplictionItemWidget::restartJob()
{
    qDebug() << "restart job: " << m_dbusJobInter->packageId();

    m_dbusJobManagerInter->StartJob(m_dbusJobInter->id());

    updateJobStat(Ready);
}

const QString ApplictionItemWidget::getIconPath(const AppUpdateInfo &info) const
{
    return Helper::searchAppIcon(QStringList() << info.m_icon << info.m_name << "application-x-desktop", 32);
}


/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     wubw <wubowen_cm@deepin.com>
 *
 * Maintainer: wubw <wubowen_cm@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "updatewidget.h"
#include "modules/update/updatemodel.h"
#include "modules/update/updatework.h"
#include "modules/update/updateitem.h"
#include "widgets/settingsgroup.h"
#include "updatesettings.h"
#include "updatehistorybutton.h"
#include "recenthistoryapplist.h"

#include <types/appupdateinfolist.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QStackedLayout>
#include <DSysInfo>

DCORE_USE_NAMESPACE

using namespace dcc::update;
using namespace dcc::widgets;
using namespace DCC_NAMESPACE;
using namespace DCC_NAMESPACE::update;

UpdateWidget::UpdateWidget(QWidget *parent)
    : QWidget(parent)
    , m_layout(new QVBoxLayout)
    , m_model(nullptr)
    , m_work(nullptr)
    , m_centerLayout(new QVBoxLayout)
    , m_label(new QLabel)
    , m_historyBtn(new UpdateHistoryButton)
    , m_updateState(UpdatesStatus::Default)
    , m_updateHistoryText(new QLabel)
    , m_applistGroup(new SettingsGroup)
    , m_recentHistoryApplist(new RecentHistoryApplist)
    , m_topSwitchWidgetBtn(new DButtonBox)
    , m_mainLayout(new QStackedLayout)
{
    //~ contents_path /update/Update
    DButtonBoxButton *btnUpdate = new DButtonBoxButton(QIcon::fromTheme("dcc_update_topupdate"), tr("Updates"));
    btnUpdate->setIconSize(QSize(24, 24));
    //~ contents_path /update/Update Settings
    DButtonBoxButton *btnSetting = new DButtonBoxButton(QIcon::fromTheme("dcc_update_topsettings"), tr("Update Settings"));
    btnSetting->setIconSize(QSize(24, 24));
    m_btnlist.append(btnUpdate);
    m_btnlist.append(btnSetting);
    m_topSwitchWidgetBtn->setButtonList(m_btnlist, true);
    m_btnlist.first()->setChecked(true);
    m_topSwitchWidgetBtn->setId(btnUpdate, 0);
    m_topSwitchWidgetBtn->setId(btnSetting, 1);
    m_topSwitchWidgetBtn->setMinimumSize(240, 36);

    m_updateHistoryText->setText(tr("Last Update"));

    connect(m_topSwitchWidgetBtn, &DButtonBox::buttonClicked, [this](QAbstractButton *value) {
        refreshWidget(static_cast<UpdateType>(m_topSwitchWidgetBtn->id(value)));
    });

    m_mainLayout->setMargin(0);
    m_layout->setAlignment(Qt::AlignTop);
    m_layout->setSpacing(0);
    m_layout->addSpacing(10);
    m_layout->addWidget(m_topSwitchWidgetBtn, 0, Qt::AlignHCenter);
    m_layout->addLayout(m_mainLayout, 0);

    QWidget *recentHistoryWidget = new QWidget;
    QVBoxLayout *bottomLayout = new QVBoxLayout;
    recentHistoryWidget->setLayout(bottomLayout);

    bottomLayout->setMargin(0);
    bottomLayout->setSpacing(0);
    bottomLayout->addWidget(m_label, 0, Qt::AlignCenter);
    bottomLayout->addWidget(m_historyBtn, 0, Qt::AlignCenter);
    bottomLayout->addWidget(m_updateHistoryText, 0, Qt::AlignCenter);
    bottomLayout->addWidget(m_recentHistoryApplist);

    m_layout->addWidget(recentHistoryWidget);

    m_historyBtn->setVisible(false);
    m_updateHistoryText->setVisible(false);
    m_recentHistoryApplist->setVisible(false);
    m_recentHistoryApplist->setContentWidget(m_applistGroup);

    m_layout->addWidget(m_recentHistoryApplist);

    setLayout(m_layout);
}

UpdateWidget::~UpdateWidget()
{

}

void UpdateWidget::initialize()
{
    connect(m_historyBtn, &UpdateHistoryButton::notifyBtnRelease, this, [ = ](bool state) {
        resetUpdateCheckState();

        if (state) {
            m_historyBtn->setLabelText(tr("Return"));
            m_updateHistoryText->setVisible(true);
            m_applistGroup->setVisible(true);
            m_recentHistoryApplist->setVisible(true);
            onAppendApplist(getTestApplistInfo());
        } else {
            m_applistGroup->setVisible(false);
        }
    });
}

void UpdateWidget::setModel(const UpdateModel *model, const UpdateWorker *work)
{
    m_model = const_cast<UpdateModel *>(model);
    m_work = const_cast<UpdateWorker *>(work);

    UpdateCtrlWidget *updateWidget = new UpdateCtrlWidget(m_model);
    updateWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(updateWidget, &UpdateCtrlWidget::requestDownloadUpdates, m_work, &UpdateWorker::downloadAndDistUpgrade);
    connect(updateWidget, &UpdateCtrlWidget::requestPauseDownload, m_work, &UpdateWorker::pauseDownload);
    connect(updateWidget, &UpdateCtrlWidget::requestResumeDownload, m_work, &UpdateWorker::resumeDownload);
    connect(updateWidget, &UpdateCtrlWidget::requestInstallUpdates, m_work, &UpdateWorker::distUpgrade);
    connect(updateWidget, &UpdateCtrlWidget::notifyUpdateState, this, &UpdateWidget::onNotifyUpdateState);
    updateWidget->setSystemVersion(m_systemVersion);

    UpdateSettings *updateSetting = new UpdateSettings(m_model);
    updateSetting->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(updateSetting, &UpdateSettings::requestSetAutoUpdate, m_work, &UpdateWorker::setAutoDownloadUpdates);
    connect(updateSetting, &UpdateSettings::requestShowMirrorsView, this, &UpdateWidget::pushMirrorsView);
    connect(updateSetting, &UpdateSettings::requestSetAutoCleanCache, m_work, &UpdateWorker::setAutoCleanCache);
    connect(updateSetting, &UpdateSettings::requestSetAutoCheckUpdates, m_work, &UpdateWorker::setAutoCheckUpdates);
#ifndef DISABLE_SYS_UPDATE_SOURCE_CHECK
    connect(updateSetting, &UpdateSettings::requestSetSourceCheck, m_work, &UpdateWorker::setSourceCheck);
#endif
    connect(updateSetting, &UpdateSettings::requestEnableSmartMirror, m_work, &UpdateWorker::setSmartMirror);

    QWidget *updateOutWidget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addStretch();
    layout->addWidget(updateWidget, Qt::AlignTop);
    updateWidget->setMinimumHeight(height());
    layout->addStretch();
    updateOutWidget->setLayout(layout);

    m_mainLayout->addWidget(updateOutWidget);
    m_mainLayout->addWidget(updateSetting);
}

void UpdateWidget::setSystemVersion(QString version)
{
    qDebug() << Q_FUNC_INFO << QString("%1 %2").arg(tr("Current Edition")).arg(version.toLatin1().data());

    if (m_systemVersion != version) {
        m_systemVersion = version;
    }

    if (DSysInfo::isCommunityEdition()) {
        m_label->setText("Deepin " + DSysInfo::deepinVersion());
    } else if (DSysInfo::deepinType() == DSysInfo::DeepinType::DeepinPersonal) {
        m_label->setText(QString("%1 %2").arg(tr("Current Edition")).arg(DSysInfo::productTypeString().toUpper() + " " + DSysInfo::deepinTypeDisplayName() + " " + DSysInfo::deepinVersion()));
    } else {
        m_label->setText(QString("%1 V%2").arg(tr("Current Edition")).arg(version.remove('"')));
    }
}

void UpdateWidget::resetUpdateCheckState(bool state)
{
    m_label->setVisible(false);
    m_historyBtn->setVisible(state);
    m_updateHistoryText->setVisible(false);
    m_historyBtn->setLabelText(tr("Update History"));
    m_recentHistoryApplist->setVisible(false);
}

void UpdateWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
}

//Used to test in "Recent update"
QList<AppUpdateInfo> UpdateWidget::getTestApplistInfo()
{
    QList<AppUpdateInfo> applist;

    AppUpdateInfo date5;
    date5.m_avilableVersion = "深度截图";
    date5.m_changelog = "深度截图";
    date5.m_currentVersion = "15.11.18";
    date5.m_icon = "/lastore/metadata/deepin-screenshot/meta/icons/deepin-screenshot.svg";
    date5.m_name = "Deepin";
    date5.m_packageId = "dde";

    applist.append(date5);

    AppUpdateInfo date;
    for (int i = 0; i < 10; i++) {
        date.m_avilableVersion = "雷鸟邮件";
        date.m_changelog = "雷鸟邮件";
        date.m_currentVersion = "15.11.19";
        date.m_icon = "/lastore/metadata/thunderbird/meta/icons/thunderbird.svg";
        date.m_name = "Deepin";
        date.m_packageId = "dde";
        applist.append(date);
    }

    return applist;
}

void UpdateWidget::refreshWidget(UpdateType type)
{
    displayUpdateContent(type);

    if (type == UpdateSettingMir) {
        QTimer::singleShot(0, this, [this] {
            Q_EMIT pushMirrorsView();
        });
    }
}

void UpdateWidget::showCheckUpdate()
{
    qDebug() << Q_FUNC_INFO << " current update status : " << m_model->status();

    m_work->checkForUpdates();

    m_mainLayout->setCurrentIndex(0);
    // prohibit dde-offline-upgrader from showing while this page is showing.
    QDBusConnection::sessionBus().registerService(OfflineUpgraderService);
}

void UpdateWidget::showUpdateSetting()
{
    qDebug() << Q_FUNC_INFO;
    resetUpdateCheckState(false);
    m_work->checkNetselect();
#ifndef DISABLE_SYS_UPDATE_MIRRORS
    m_work->refreshMirrors();
#endif
    m_mainLayout->setCurrentIndex(1);
}

void UpdateWidget::displayUpdateContent(UpdateType index)
{
    QLayoutItem *item;
    while ((item = m_centerLayout->layout()->takeAt(0)) != nullptr) {
        item->widget()->deleteLater();
        delete item;
        item = nullptr;
    }

    switch (static_cast<UpdateType>(index)) {
    case UpdateCheck:
        showCheckUpdate();
        m_btnlist.at(0)->setChecked(true);
        break;
    case UpdateSetting:
    case UpdateSettingMir:
        showUpdateSetting();
        m_btnlist.at(1)->setChecked(true);
        break;
    default:
        break;
    }
    setFocus();
}

void UpdateWidget::onNotifyUpdateState(int state)
{
    if (m_updateState == static_cast<UpdatesStatus>(state)) {
        return;
    } else {
        m_updateState = static_cast<UpdatesStatus>(state);
    }

    m_label->setVisible(false);
    m_historyBtn->setVisible(false);

    switch (m_updateState) {
    case Checking:
        m_label->setVisible(true);
        break;
    case Updated:
    case UpdateSucceeded:
    case NeedRestart:
        //更新历史,暂时屏蔽显示入口,待后期再觉得是否需要此功能
//        m_historyBtn->setVisible(true);
        break;
    case UpdatesAvailable:
    case Downloading:
    case DownloadPaused:
    case Downloaded:
    case Installing:
    case UpdateFailed:
    case NoNetwork:
    case NoSpace:
    case DeependenciesBrokenError:
        //now donothing
        break;
    default:
        break;
    }
}

void UpdateWidget::onAppendApplist(const QList<AppUpdateInfo> &infos)
{
    qDebug() << infos.count();
    QLayoutItem *item;
    while ((item = m_applistGroup->layout()->takeAt(0)) != nullptr) {
        item->widget()->deleteLater();
        delete item;
    }

    for (const AppUpdateInfo &info : infos) {
        UpdateItem *item = new UpdateItem();
        item->setAppInfo(info);

        m_applistGroup->appendItem(item);
    }
}

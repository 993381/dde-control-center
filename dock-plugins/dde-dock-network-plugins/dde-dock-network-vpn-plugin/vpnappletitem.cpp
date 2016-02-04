/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include <QHBoxLayout>
#include "vpnappletitem.h"
#include <dde-dock/dockconstants.h>

const int ITEM_LEFT_MARGIN = 5;
const int ITEM_RIGHT_MARGIN = 15;

VpnAppletItem::VpnAppletItem(const VpnData &data, DBusNetwork *dbusNetwork, QWidget *parent)
    : QFrame(parent), m_dbusNetwork(dbusNetwork), m_itemData(data)
{
    connect(dbusNetwork, &DBusNetwork::ActiveConnectionsChanged, this, &VpnAppletItem::updateConnectionState);

    QFrame *checkFrame = new QFrame;
    checkFrame->setFixedSize(Dock::APPLET_EFFICIENT_ICON_SIZE, Dock::APPLET_EFFICIENT_ICON_SIZE);

    m_checkIcon = new DImageButton(checkFrame);
    m_checkIcon->setNormalPic(":/images/images/connected.png");
    m_checkIcon->setHoverPic(":/images/images/disconnect.png");
    m_checkIcon->setPressPic(":/images/images/disconnect.png");
    m_checkIcon->setFixedSize(Dock::APPLET_EFFICIENT_ICON_SIZE, Dock::APPLET_EFFICIENT_ICON_SIZE);
    m_checkIcon->setVisible(false);
    connect(m_checkIcon, &DImageButton::clicked, [=]{
        qDebug() << "[VpnPlugin] Disconnect: " << getItemUuid();
        ASYN_CALL(m_dbusNetwork->DeactivateConnection(data.uuid), {Q_UNUSED(args)}, this)
    });

    m_loadingIcon = new DLoadingIndicator(checkFrame);
    m_loadingIcon->setFixedSize(Dock::APPLET_EFFICIENT_ICON_SIZE, Dock::APPLET_EFFICIENT_ICON_SIZE);
    m_loadingIcon->setImageSource(QPixmap(":/images/images/waiting.png"));
    m_loadingIcon->setLoading(true);
    m_loadingIcon->setVisible(false);

    m_title = new QPushButton(data.id);
    m_title->setObjectName("VpnItemTitle");
    connect(m_title, &QPushButton::clicked, [=] {
        qDebug() << "[VpnPlugin] Try to active vpn: " << getItemUuid();
        ASYN_CALL(m_dbusNetwork->ActivateConnection(data.uuid, QDBusObjectPath("/")), {Q_UNUSED(args)}, this)
    });

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(ITEM_LEFT_MARGIN, 0, ITEM_RIGHT_MARGIN, 0);
    layout->setSpacing(0);
    layout->addWidget(checkFrame);
    layout->addWidget(m_title);

    updateConnectionState();
}

QString VpnAppletItem::getItemUuid() const
{
    return m_itemData.uuid;
}

VpnAppletItem::VpnData VpnAppletItem::getItemData() const
{
    return m_itemData;
}

void VpnAppletItem::updateConnectionState()
{
    ConnectionState state = getActiveConnectionsStateByUuid(m_itemData.uuid, m_dbusNetwork);
    switch (state) {
    case ActiveConnectionStateActivating:
    case ActiveConnectionStateDeactivating:
        m_loadingIcon->show();
        m_loadingIcon->setLoading(true);
        m_checkIcon->setVisible(false);
        break;
    case ActiveConnectionStateActivated:
        m_loadingIcon->setVisible(false);
        m_loadingIcon->setLoading(false);
        m_checkIcon->show();
        break;
    default:
        m_loadingIcon->setVisible(false);
        m_loadingIcon->setLoading(false);
        m_checkIcon->setVisible(false);
    }
}

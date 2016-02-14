/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef WIRELESSAPPLETITEM_H
#define WIRELESSAPPLETITEM_H

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include "dbus/dbusnetwork.h"
#include "libdui/dloadingindicator.h"
#include "libdui/dimagebutton.h"
#include "../network-data/networkdata.h"

DUI_USE_NAMESPACE
using namespace NetworkPlugin;

class WirelessAppletItem : public QFrame
{
    Q_OBJECT
public:
    struct ApData {
        QString apPath;
        QString ssid;
        int strength = -1;
        bool secured = false;
        bool securedInEap = true;
    };

    explicit WirelessAppletItem(const ApData &data, const QString &devicePath, DBusNetwork *dbusNetwork, QWidget *parent = 0);

    QString getApPath() const;
    ApData getApData() const;

public slots:
    void onActiveApChanged(const QString &ap);
    void onActiveConnectionsChanged();

signals:
    void strengthChanged(int strength);

private:
    QPixmap getPixmapByStrength();
    void updateConnectionState();
    void onAccessPointPropertiesChanged(const QString &devicePath, const QString &info);

private:
    DBusNetwork *m_dbusNetwork;
    QString m_devicePath;
    QString m_availableUuid;    //fixme,one ssid may point to multiple uuid
    ApData m_apData;

    QPushButton *m_title;
    DImageButton *m_checkIcon;
    DLoadingIndicator *m_loadingIcon;
    QLabel *m_strengthIcon;
};

#endif // WIRELESSAPPLETITEM_H

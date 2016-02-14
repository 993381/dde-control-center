/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef VPNPLUGIN_H
#define VPNPLUGIN_H

#include <QObject>
#include <QSettings>

#include <dde-dock/dockconstants.h>
#include <dde-dock/dockplugininterface.h>
#include <dde-dock/dockpluginproxyinterface.h>

#include "dbus/dbusnetwork.h"
#include "../network-data/networkdata.h"

class QLabel;
class VpnApplet;
class VpnPlugin : public QObject, public DockPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.Dock.PluginInterface" FILE "dde-dock-network-vpn-plugin.json")
    Q_INTERFACES(DockPluginInterface)

public:
    VpnPlugin();
    ~VpnPlugin() Q_DECL_OVERRIDE;

    void init(DockPluginProxyInterface *proxy) Q_DECL_OVERRIDE;

    QString getPluginName() Q_DECL_OVERRIDE;

    QStringList ids() Q_DECL_OVERRIDE;
    QString getName(QString id) Q_DECL_OVERRIDE;
    QString getTitle(QString id) Q_DECL_OVERRIDE;
    QString getCommand(QString id) Q_DECL_OVERRIDE;
    QString getMenuContent(QString id) Q_DECL_OVERRIDE;
    QPixmap getIcon(QString) Q_DECL_OVERRIDE;
    QWidget *getItem(QString id) Q_DECL_OVERRIDE;
    QWidget *getApplet(QString id) Q_DECL_OVERRIDE;
    bool configurable(const QString &id) Q_DECL_OVERRIDE;
    bool enabled(const QString &id) Q_DECL_OVERRIDE;
    void setEnabled(const QString &id, bool enabled) Q_DECL_OVERRIDE;
    void changeMode(Dock::DockMode newMode, Dock::DockMode oldMode) Q_DECL_OVERRIDE;
    void invokeMenuItem(QString, QString, bool) Q_DECL_OVERRIDE;

private:
    QLabel *m_vpnItem;
    VpnApplet *m_applet;
    QSettings *m_settings;
    DockPluginProxyInterface *m_proxy;
    Dock::DockMode m_mode = Dock::EfficientMode;
    com::deepin::daemon::DBusNetwork *m_dbusNetwork;

    void initSettings();
    void updateIcon();
    void addNewItem(const QString &id);
    void removeItem(const QString &id);
    void onEnabledChanged(const QString &id);
    void onConnectionsChanged();
    QString settingEnabledKey(const QString &id);
};

#endif // VPNPLUGIN_H

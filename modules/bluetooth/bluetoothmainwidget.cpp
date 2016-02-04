/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include <QDebug>
#include <QResizeEvent>
#include <QJsonDocument>

#include <libdui/dconstants.h>
#include <libdui/dseparatorhorizontal.h>

#include "moduleheader.h"
#include "constants.h"

#include "deviceitemwidget.h"
#include "bluetoothmainwidget.h"
#include "adapterwidget.h"

DUI_USE_NAMESPACE

#define ASYN_CALL(Fun, Code, captured...) { \
    QDBusPendingCallWatcher * watcher = new QDBusPendingCallWatcher(Fun, this); \
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [watcher, captured]{ \
        const QVariantList & args = watcher->reply().arguments(); \
        Code \
        watcher->deleteLater(); \
    }); }

BluetoothMainWidget::BluetoothMainWidget(QWidget *parent) :
    QFrame(parent),
    m_mainLayout(new QVBoxLayout(this))
{
    initUI();
    intiBackend();
}

void BluetoothMainWidget::addAdapter(AdapterInfo *info)
{
    if (!info)
        return;

    info->widget = new AdapterWidget(info);
    m_adapterList->addWidget(info->widget);
}

void BluetoothMainWidget::removeAdapter(const AdapterInfo *info)
{
    if(!info)
        return;

    int index = m_adapterList->indexOf(info->widget);
    if(index < 0)
        return;

    for(DeviceInfo *device_info : m_pathToDeviceInfoMap.values()) {
        if(device_info->adapterInfo == info) {
            m_pathToDeviceInfoMap.remove(info->path + device_info->path);
        }
    }

    m_pathToAdapterInfoMap.remove(info->path);
    m_adapterList->removeWidget(index);
}

void BluetoothMainWidget::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);

    m_adapterList->setFixedHeight(e->size().height() - m_adapterList->y());
}

BluetoothMainWidget::AdapterInfo* BluetoothMainWidget::newAdapterInfoByMap(const QVariantMap &map)
{
    if (m_pathToAdapterInfoMap.contains(map["Path"].toString()))
        return nullptr;

    AdapterInfo *info = new AdapterInfo;

    info->path = map["Path"].toString();
    info->bluetoothDbus = m_bluetoothDbus;

    m_pathToAdapterInfoMap[info->path] = info;

    updateAdapterInfoByMap(info, map);

    ASYN_CALL(m_bluetoothDbus->GetDevices(QDBusObjectPath(info->path)), {
                  if(!info->widget)
                    return;

                  QJsonDocument json_doc = QJsonDocument::fromJson(args[0].toByteArray());
                  for(const QJsonValue &value : json_doc.array()){
                      const QVariantMap &tmp_map = value.toObject().toVariantMap();
                      DeviceInfo *device_info = newDeviceInfoByMap(tmp_map);

                      device_info->item = newDeviceListItem(device_info);
                      if (device_info->paired)
                        info->widget->addTrustedDevice(device_info);
                      else
                        info->widget->addDevice(device_info);
                  }
              }, this, info);

    return info;
}

BluetoothMainWidget::DeviceInfo *BluetoothMainWidget::newDeviceInfoByMap(const QVariantMap &map)
{
    DeviceInfo *info = new DeviceInfo;

    info->path = map["Path"].toString();

    m_pathToDeviceInfoMap[map["AdapterPath"].toString() + info->path] = info;

    updateDeviceInfoByMap(info, map);

    return info;
}

DeviceItemWidget *BluetoothMainWidget::newDeviceListItem(DeviceInfo *device_info) const
{
    DeviceItemWidget *item = new DeviceItemWidget(device_info);
    item->setFixedSize(DCC::ModuleContentWidth, DUI::EXPAND_HEADER_HEIGHT);

    return item;
}

void BluetoothMainWidget::updateAdapterInfoByMap(BluetoothMainWidget::AdapterInfo *info, const QVariantMap &map)
{
    if(!info)
        return;

    info->name = map["Alias"].toString();
    info->powered = map["Powered"].toBool();
    info->discovering = map["Discovering"].toBool();
    info->discoverable = map["Discoverable"].toBool();
    info->discoverableTimeout = map["DiscoverableTimeout"].toUInt();
}

void BluetoothMainWidget::updateDeviceInfoByMap(BluetoothMainWidget::DeviceInfo *info, const QVariantMap &map)
{
    if(!info)
        return;

//    qDebug() << "update " << map["Paired"].toBool()  << "to " << info->trusted;

    AdapterWidget *adapter = nullptr;
    if (info->adapterInfo && info->adapterInfo->widget)
        adapter = info->adapterInfo->widget;

    if (adapter && map["Paired"].toBool())
    {
        qDebug() << "move ";
        adapter->removeDevice(info, false);
        adapter->addTrustedDevice(info);
    }

    info->name = map["Alias"].toString();
    info->trusted = map["Trusted"].toBool();
    info->paired = map["Paired"].toBool();
    info->state = map["State"].toInt();
    info->icon = map["Icon"].toString();
}

QVariantMap getMapByJson(const QString &json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(json.toUtf8());
    return json_doc.object().toVariantMap();
}

void BluetoothMainWidget::intiBackend()
{
    m_bluetoothDbus = new DBusBluetooth(this);
//    m_bluetoothDbus->ClearUnpairedDevice();

    if(m_bluetoothDbus->state() > 0) {
        ASYN_CALL(m_bluetoothDbus->GetAdapters(), {
                      QJsonDocument json_doc = QJsonDocument::fromJson(args[0].toByteArray());
                      for(const QJsonValue &value : json_doc.array()){
                          const QVariantMap &map = value.toObject().toVariantMap();
                          addAdapter(newAdapterInfoByMap(map));
                      }
                  }, this);
    }

    connect(m_bluetoothDbus, &DBusBluetooth::AdapterAdded, this, [this](const QString &str) {
        addAdapter(newAdapterInfoByMap(getMapByJson(str)));
    });

    connect(m_bluetoothDbus, &DBusBluetooth::AdapterRemoved, this, [this](const QString &str) {
        removeAdapter(m_pathToAdapterInfoMap.value(getMapByJson(str)["Path"].toString(), nullptr));
    });

    connect(m_bluetoothDbus, &DBusBluetooth::DeviceAdded, this, [this](const QString &str){
        const QVariantMap &map = getMapByJson(str);

        AdapterInfo *info = m_pathToAdapterInfoMap.value(map["AdapterPath"].toString(), nullptr);
        if(info){
            DeviceInfo *device_info = newDeviceInfoByMap(map);
            device_info->item = newDeviceListItem(device_info);
            info->widget->addDevice(device_info);
        }
    });

    connect(m_bluetoothDbus, &DBusBluetooth::DeviceRemoved, this, [this](const QString &str){
        const QVariantMap &map = getMapByJson(str);
        AdapterInfo *info = m_pathToAdapterInfoMap.value(map["AdapterPath"].toString(), nullptr);
        if(info){
            DeviceInfo *device_info = m_pathToDeviceInfoMap.value(info->path + map["Path"].toString(), nullptr);
            if(device_info){
//                if (device_info->trusted)
                    info->widget->removeTrustedDevice(device_info);
//                else
                    info->widget->removeDevice(device_info, true);
                m_pathToDeviceInfoMap.remove(info->path + device_info->path);
            }
        }
    });

    connect(m_bluetoothDbus, &DBusBluetooth::AdapterPropertiesChanged, this, [this](const QString &str){
        const QVariantMap &map = getMapByJson(str);
        AdapterInfo *info = m_pathToAdapterInfoMap.value(map["Path"].toString(), nullptr);
        updateAdapterInfoByMap(info, map);
        if(info)
            info->widget->updateUI();
    });

    connect(m_bluetoothDbus, &DBusBluetooth::DevicePropertiesChanged, this, [this](const QString &str){
        const QVariantMap &map = getMapByJson(str);
        DeviceInfo *info = m_pathToDeviceInfoMap.value(map["AdapterPath"].toString() + map["Path"].toString(), nullptr);
        updateDeviceInfoByMap(info, map);
        if(info)
            info->item->updateUI();
    });

    connect(m_bluetoothDbus, &DBusBluetooth::RequestConfirmation, this, [this] (const QDBusObjectPath &in0, const QString &in1) {
        qDebug() << "RequestConfirmation" << in0.path() << in1;

        // TODO: display confirm dialog.
        m_bluetoothDbus->Confirm(in0, true);
    });

    connect(m_bluetoothDbus, &DBusBluetooth::RequestPinCode, this, [this] (const QDBusObjectPath &in0) {
        qDebug() << "RequestPinCode" << in0.path();
    });

    connect(m_bluetoothDbus, &DBusBluetooth::DisplayPinCode, this, [this] (const QDBusObjectPath &in0, const QString &in1) {
        qDebug() << "DisplayPinCode" << in0.path() << in1;
    });
}

void BluetoothMainWidget::initUI()
{
    ModuleHeader *header = new ModuleHeader(tr("Bluetooth"), false);

    m_adapterList = new DListWidget;

    m_mainLayout->setSpacing(0);
    m_mainLayout->setMargin(0);

    m_mainLayout->addWidget(header);
    m_mainLayout->addWidget(new DSeparatorHorizontal);
    m_mainLayout->addWidget(m_adapterList);
    m_mainLayout->addStretch(1);
}


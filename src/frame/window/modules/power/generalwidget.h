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
#pragma once
#include "interface/namespace.h"
#include "widgets/dccslider.h"
#include "widgets/powerdisplaywidget.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
namespace dcc {
namespace widgets {
class SwitchWidget;
class TitledSliderItem;
class NormalLabel;
class TitleValueItem;
}
namespace power {
class PowerModel;
}
}
namespace DCC_NAMESPACE {
namespace power {
class GeneralWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GeneralWidget(QWidget *parent = nullptr, bool bIsBattery = false);
    virtual ~GeneralWidget();
    void setModel(const dcc::power::PowerModel *model);
private:
    //初始化使用到的 slider 控件
    void initSlider();
private:
    QVBoxLayout *m_layout;
    //add Energy Saving Mode
    QVBoxLayout *m_layEnergySavingMode;
    dcc::widgets::SwitchWidget *m_swLowPowerAutoIntoSaveEnergyMode;
    //---------------------------------------------
    //add battery info
    dcc::widgets::SwitchWidget *m_showBatteryCapacity;
    dcc::widgets::TitleValueItem *m_batteryCapacity;
    //---------------------------------------------
    dcc::widgets::SwitchWidget *m_lowBatteryMode;
    dcc::widgets::SwitchWidget *m_autoIntoSaveEnergyMode;
    dcc::widgets::SwitchWidget *m_wakeComputerNeedPassword;
    dcc::widgets::SwitchWidget *m_wakeDisplayNeedPassword;
    dcc::widgets::TitledSliderItem *m_monitorSleepOnPower = nullptr;
    dcc::widgets::TitledSliderItem *m_sldLowerBrightness = nullptr;
    QLabel *m_titleWidget;
    dcc::widgets::SwitchWidget *m_powerShowTimeToFull;
    dcc::widgets::PowerDisplayWidget *m_ShowTimeToFullTips;
Q_SIGNALS:
    void requestSetLowBatteryMode(const bool &state);//同节能模式
    void requestSetPowerSavingModeAutoWhenQuantifyLow(const bool &state);//低电量自动切换节能模式
    void requestSetPowerSavingModeAuto(const bool &state);//自动切换节能模式
    void requestSetWakeComputer(const bool &state);//待机恢复输入密码
    void requestSetWakeDisplay(const bool &state);//唤醒显示器输入密码
    void requestSetPowerSaveMode(const bool &state);//节能模式
    void requestSetPowerSavingModeLowerBrightnessThreshold(const int &level);//节能模式亮度降低
public Q_SLOTS:
    void setPowerDisplay(const bool &state);
    void onGSettingsChanged(const QString &key);
};
}// namespace datetime
}// namespace DCC_NAMESPACE

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
#include "timezonelist.h"
#include "modules/datetime/timezoneitem.h"
#include "modules/datetime/timezone_dialog/timezone.h"
#include "modules/datetime/timezone_dialog/timezonechooser.h"
#include "widgets/settingsgroup.h"
#include "widgets/settingsitem.h"
#include "widgets/settingshead.h"
#include "window/utils.h"

using namespace dcc;
using namespace dcc::datetime;
using namespace dcc::widgets;
using namespace DCC_NAMESPACE::datetime;

TimezoneList::TimezoneList(QWidget *parent)
    : QWidget(parent)
    , m_centralLayout(new QVBoxLayout)
    , m_addTimezoneButton(new DFloatingButton(DStyle::SP_IncreaseElement, this))
    , m_headGroup(new SettingsGroup)
    , m_headItem(new SettingsHead)
    , m_contentList(new TimezoneContentList(this))
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_headItem->setEditEnable(true);
    m_headItem->setVisible(true);
    //~ contents_path /datetime/Timezone List
    m_headItem->setTitle(tr("Timezone List"));
    m_headItem->setFixedHeight(34);
    m_headGroup->appendItem(m_headItem);

    m_centralLayout->setMargin(0);
    m_centralLayout->setSpacing(0);
    m_centralLayout->addWidget(m_headGroup);
    m_centralLayout->addWidget(m_contentList);

    m_contentList->setSpace(List_Interval);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setMargin(0);
    btnLayout->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
    btnLayout->addWidget(m_addTimezoneButton);
    m_centralLayout->addLayout(btnLayout);
    setLayout(m_centralLayout);

    //~ contents_path /datetime/Timezone List
    m_addTimezoneButton->setToolTip(tr("Add Timezone"));

    connect(m_addTimezoneButton, &QPushButton::clicked, this, [this]{
        m_contentList->onEditClicked(false);
        m_headItem->toCancel();
        requestAddTimeZone();
    });
    connect(m_headItem, &SettingsHead::editChanged, m_contentList, &TimezoneContentList::onEditClicked);
    connect(m_contentList, &TimezoneContentList::requestRemoveUserTimeZone, this, [this]() {
        //refresh right button to edit
        m_headItem->toCancel();
    });

    connect(m_contentList, &TimezoneContentList::notifyItemCount, this, [this](int value) {
        m_headGroup->setVisible(value > 1);
    });
}

TimezoneList::~TimezoneList()
{

}

TimezoneContentList *TimezoneList::getTimezoneContentListPtr()
{
    return m_contentList;
}

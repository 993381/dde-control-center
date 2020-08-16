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

#include "loadingitem.h"

#include "widgets/labels/normallabel.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QIcon>
#include <DSysInfo>

DCORE_USE_NAMESPACE
using namespace dcc::widgets;
using namespace DCC_NAMESPACE;
using namespace DCC_NAMESPACE::update;

LoadingItem::LoadingItem(QFrame *parent)
    : QWidget(parent)
    , m_messageLabel(new NormalLabel)
    , m_progress(new QProgressBar(this))
    , m_currLangSelector(new LangSelector("com.deepin.daemon.LangSelector",
                                                "/com/deepin/daemon/LangSelector",
                                                QDBusConnection::sessionBus(), this))
{
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(10);

    m_progress->setRange(0, 100);
    m_progress->setFixedWidth(200);
    m_progress->setFixedHeight(7);
    m_progress->setTextVisible(false);

    QVBoxLayout *imgLayout = new QVBoxLayout;
    imgLayout->setAlignment(Qt::AlignCenter);
    m_image = new QImage;
    m_labelImage = new QLabel;
    m_labelImage->setPixmap(QPixmap::fromImage(*m_image));
    m_labelImage->setMinimumSize(128, 128);
    imgLayout->addWidget(m_labelImage, 0, Qt::AlignTop);

    QHBoxLayout *txtLayout = new QHBoxLayout;
    txtLayout->setAlignment(Qt::AlignCenter);
    m_labelText = new QLabel;
    txtLayout->addWidget(m_labelText);

    layout->addStretch();
    layout->addLayout(imgLayout);
    layout->addLayout(txtLayout);
    layout->addWidget(m_progress, 0, Qt::AlignHCenter);
    layout->addWidget(m_messageLabel, 0, Qt::AlignHCenter);
    layout->addStretch();

    setLayout(layout);
}

void LoadingItem::setProgressValue(int value)
{
    m_progress->setValue(value);
}

void LoadingItem::setProgressBarVisible(bool visible)
{
    m_progress->setVisible(visible);
}

void LoadingItem::setMessage(const QString &message)
{
    m_messageLabel->setText(message);
    m_messageLabel->setWordWrap(true);
}

void LoadingItem::setVersionVisible(bool state)
{
    m_labelText->setVisible(state);
}

void LoadingItem::setSystemVersion(QString version)
{
    if ("zh_CN.UTF-8" != m_currLangSelector->currentLocale() && DSysInfo::DeepinPersonal == DSysInfo::deepinType()) {
        m_labelText->setText(DSysInfo::productTypeString().toUpper() + " " + DSysInfo::deepinVersion().left(2) + " " + "Home");
    } else if (DSysInfo::DeepinDesktop == DSysInfo::deepinType()) {
        m_labelText->setText(DSysInfo::productTypeString() + " " + DSysInfo::deepinVersion().left(2));
    } else {
        m_labelText->setText(DSysInfo::productTypeString() + " " + DSysInfo::deepinVersion().left(2) + " " + DSysInfo::deepinTypeDisplayName());
    }
}

void LoadingItem::setImage(QImage *image)
{
    m_labelImage->setPixmap(QPixmap::fromImage(*image));
}

void LoadingItem::setImageVisible(bool state)
{
    m_labelImage->setVisible(state);
}

QPixmap LoadingItem::getPixmap(const QString name, const QSize size)
{
    const QIcon &icon = QIcon(name);
    const qreal ratio = devicePixelRatioF();
    QPixmap pixmap = icon.pixmap(size * ratio).scaled(size * ratio, Qt::KeepAspectRatio, Qt::FastTransformation);
    pixmap.setDevicePixelRatio(ratio);
    return pixmap;
}

//image or text only use one
void LoadingItem::setImageOrTextVisible(bool state)
{
    qDebug() << Q_FUNC_INFO << state;

    setVersionVisible(state);
    setImageVisible(true);

    QString path = "";
    if (state) {
        //path = ":/update/updatev20/dcc_all_updated.svg";
        path = ":/update/themes/common/icons/success.svg";
    } else {
        path = ":/update/updatev20/dcc_checking_update.svg";
    }

    m_labelImage->setPixmap(getPixmap(path, QSize(128, 128)));
}

//image and text all use or not use
void LoadingItem::setImageAndTextVisible(bool state)
{
    setVersionVisible(state);
    setImageVisible(state);
}

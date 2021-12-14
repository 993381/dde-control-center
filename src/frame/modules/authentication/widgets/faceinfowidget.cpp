/*
 * Copyright (C) 2016 ~ 2021 Deepin Technology Co., Ltd.
 *
 * Author:     duanhongyu <duanhongyu@uniontech.com>

 * Maintainer: duanhongyu <duanhongyu@uniontech.com>
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

#include "faceinfowidget.h"

#include <DApplicationHelper>
#include <DPlatformTheme>

#include <QBoxLayout>
#include <QTimer>
#include <QDebug>
#include <QPainter>
#include <QDBusUnixFileDescriptor>
#include <QPainterPath>

#define Faceimg_SIZE 248

using namespace dcc;
using namespace dcc::authentication;

FaceInfoWidget::FaceInfoWidget(QWidget *parent)
    : QLabel (parent)
    , m_faceLable(new QLabel(this))
    , m_startTimer(new QTimer(this))
    , m_themeColor(DGuiApplicationHelper::instance()->systemTheme()->activeColor())
    , m_persent(0)
    , m_rotateAngle(0)
{
    initWidget();

    connect(m_startTimer, &QTimer::timeout, this, &FaceInfoWidget::onUpdateProgressbar);
    m_startTimer->start(100);
}

FaceInfoWidget::~FaceInfoWidget()
{
    if (m_startTimer)
        m_startTimer->stop();
    m_faceLable = nullptr;
}

void FaceInfoWidget::initWidget()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    setFixedSize(QSize(258, 258));
    mainLayout->setAlignment(Qt::AlignHCenter);

    mainLayout->addWidget(m_faceLable, 0, Qt::AlignHCenter);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    setLayout(mainLayout);
}

void FaceInfoWidget::createConnection(const int fd)
{
    DA_read_frames(fd, static_cast<void *>(m_faceLable), recvCamara);
}

void FaceInfoWidget::onUpdateProgressbar()
{
    m_persent >= 100 ? m_persent = 0 : m_persent += 2;
    update();
}

void FaceInfoWidget::recvCamara(void *const context, const DA_img *const img)
{
    QLabel *label_ptr = static_cast<QLabel *>(context);

    QPixmap pix(Faceimg_SIZE, Faceimg_SIZE);
    pix.fill(Qt::transparent);
    QPainter painter(&pix);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QPainterPath path;
    path.addEllipse(0, 0, Faceimg_SIZE, Faceimg_SIZE);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, Faceimg_SIZE, Faceimg_SIZE, QPixmap::fromImage(QImage((uchar *)(img->data), img->width,
                                                                                   img->height, QImage::Format_RGB888)));
    if (label_ptr) {
        label_ptr->setPixmap(pix);
    }
}


void FaceInfoWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.save();

    m_rotateAngle = 360 * m_persent / 100;
    int side = qMin(width(), height());
    QRectF outRect(0, 0, side, side);
    QRectF inRect(4, 4, side-8, side-8);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(QColor("#e6e6e6")));
    painter.drawEllipse(outRect);
    painter.setBrush(QBrush(m_themeColor));
    // startAngle和spanAngle必须以1/16度指定，即整圆等于5760（16 * 360）
    painter.drawPie(outRect, (90 - m_rotateAngle)*16, 40 * 16);

    //画遮罩
    painter.setBrush(palette().window().color());
    painter.drawEllipse(inRect);
    painter.restore();

    QWidget::paintEvent(event);
}

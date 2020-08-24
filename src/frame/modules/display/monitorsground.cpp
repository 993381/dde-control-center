/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
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

#include "monitorsground.h"
#include "monitorproxywidget.h"
#include "displaymodel.h"
#include "monitor.h"

#include <QPainter>
#include <QDebug>

using namespace dcc::display;

const int MARGIN_W = 20;
const int MARGIN_H = 10;
const int VIEW_WIDTH = 400;
const int VIEW_HEIGHT = 200;

MonitorsGround::MonitorsGround(QWidget *parent)
    : QFrame(parent)
    , m_refershTimer(new QTimer(this))
{
    m_refershTimer->setInterval(100);
    m_refershTimer->setSingleShot(true);

    connect(m_refershTimer, &QTimer::timeout, this, &MonitorsGround::resetMonitorsView);

    setFixedSize(VIEW_WIDTH + MARGIN_W * 2, VIEW_HEIGHT + MARGIN_H * 2);
}

MonitorsGround::~MonitorsGround()
{
    qDeleteAll(m_monitors.keys());
}

void MonitorsGround::setDisplayModel(DisplayModel *model, Monitor *moni)
{
    m_model = model;
    m_viewPortWidth = model->screenWidth();
    m_viewPortHeight = model->screenHeight();

    auto initMW = [ this ](Monitor * mon) {

        MonitorProxyWidget *pw = new MonitorProxyWidget(mon, m_model, this);
        m_monitors[pw] = mon;

        connect(pw, &MonitorProxyWidget::requestApplyMove, this, &MonitorsGround::monitorMoved);
        connect(pw, &MonitorProxyWidget::requestMonitorPress, this, &MonitorsGround::requestMonitorPress);
        connect(pw, &MonitorProxyWidget::requestMonitorRelease, this, &MonitorsGround::requestMonitorRelease);
        connect(mon, &Monitor::geometryChanged, m_refershTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
        connect(m_model, &DisplayModel::primaryScreenChanged, pw, static_cast<void (MonitorProxyWidget::*)()>(&MonitorProxyWidget::update), Qt::QueuedConnection);
    };

    if (!moni) {
        Monitor *primary = nullptr;
        for (auto mon : model->monitorList()) {
            if (mon->isPrimary()) {
                primary = mon;
                continue;
            }
            initMW(mon);
        }
        if (primary) {
            initMW(primary);
        }
    } else {
        initMW(moni);
    }

    QTimer::singleShot(1, this, &MonitorsGround::resetMonitorsView);
}

void MonitorsGround::resetMonitorsView()
{
    qDebug() << Q_FUNC_INFO;

    reloadViewPortSize();
    if (m_model->isMerge()) {
        adjustAll();
        return;
    } else {
        for (auto pw : m_monitors.keys())
            adjust(pw);
    }

    // recheck settings
    if (!isScreenPerfect()) {
        monitorMoved(m_monitors.firstKey());
    }
}

void MonitorsGround::monitorMoved(MonitorProxyWidget *pw)
{
    qDebug() << Q_FUNC_INFO << pw->name();

    const double scale = screenScale();
    const double offsetX = VIEW_WIDTH / 2 - (m_viewPortWidth * scale) / 2 + MARGIN_W;
    const double offsetY = VIEW_HEIGHT / 2 - (m_viewPortHeight * scale) / 2 + MARGIN_H;

    pw->setMovedX(static_cast<int>((pw->pos().x() - offsetX) / scale));
    pw->setMovedY(static_cast<int>((pw->pos().y() - offsetY) / scale));

    // ensure screens is 贴合但不相交
    ensureWidgetPerfect(pw);

    // clear global offset
    int minX = INT_MAX;
    int minY = INT_MAX;
    for (auto w : m_monitors.keys()) {
        minX = std::min(minX, w->x());
        minY = std::min(minY, w->y());
    }
    for (auto w : m_monitors.keys()) {
        w->setMovedX(w->x() - minX);
        w->setMovedY(w->y() - minY);
    }

    applySettings();
    qApp->processEvents();
    QTimer::singleShot(1, this, &MonitorsGround::resetMonitorsView);
}

void MonitorsGround::adjust(MonitorProxyWidget *pw)
{
    bool bSingle = false;
    int enabledCount = 0;
    for (auto* value : m_monitors)
    {
        if (value->enable()) {
            enabledCount++;
            m_monitors.key(value)->setVisible(true);
        }
    }
    if (1 == enabledCount)
        bSingle = true;

    qDebug() << "adjust" << pw->name();

    const double scale = screenScale();

    const double offsetX = VIEW_WIDTH / 2 - (m_viewPortWidth * scale) / 2 + MARGIN_W;
    const double offsetY = VIEW_HEIGHT / 2 - (m_viewPortHeight * scale) / 2 + MARGIN_H;

    const double w = scale * pw->w();
    const double h = scale * pw->h();
    const double x = scale * pw->x();
    const double y = scale * pw->y();

    if (bSingle) {
        const double wSingle = 0.15 * pw->w();
        const double hSingle = 0.15 * pw->h();
        pw->setGeometry(static_cast<int>((width() - wSingle)/2), static_cast<int>((height() - hSingle)/2), static_cast<int>(wSingle), static_cast<int>(hSingle));
        this->setEnabled(false);//单屏时不允许鼠标拖动 不然以前的机制会导致窗体重算引发方大
    } else {
        this->setEnabled(true);
        pw->setGeometry(static_cast<int>(x + offsetX), static_cast<int>(y + offsetY), static_cast<int>(w), static_cast<int>(h));
    }
    pw->update();
}

void MonitorsGround::adjustAll()
{
    const double scale = screenScale();
    int offset = 0;
    const double offsetX = VIEW_WIDTH / 2 - (m_viewPortWidth * scale) / 2 + MARGIN_W;
    const double offsetY = VIEW_HEIGHT / 2 - (m_viewPortHeight * scale) / 2 + MARGIN_H;
    MonitorProxyWidget *primarywdt = nullptr;
    for (auto pw : m_monitors.keys()) {
        if (pw->name() == m_model->primary()) {
            primarywdt = pw;
            continue;
        }
        const double w = scale * pw->w() * 0.5;
        const double h = scale * pw->h() * 0.5;
        const double x = scale * pw->x();
        const double y = scale * pw->y();

        pw->setGeometry(static_cast<int>(x + offsetX + w * 0.5 - offset), static_cast<int>(y + offsetY + h * 0.5 - offset), static_cast<int>(w), static_cast<int>(h));
        offset += 10;
    }
    if (primarywdt) {
        const double w = scale * primarywdt->w() * 0.5;
        const double h = scale * primarywdt->h() * 0.5;
        const double x = scale * primarywdt->x();
        const double y = scale * primarywdt->y();

        primarywdt->setGeometry(static_cast<int>(x + offsetX + w * 0.5 - offset), static_cast<int>(y + offsetY + h * 0.5 - offset), static_cast<int>(w), static_cast<int>(h));
    }
}

void MonitorsGround::ensureWidgetPerfect(MonitorProxyWidget *pw)
{
    qDebug() << Q_FUNC_INFO << pw->name();

    // TODO: only support 2 screens
    if (m_monitors.size() != 2)
        return;

    MonitorProxyWidget *other = nullptr;
    for (auto w : m_monitors.keys()) {
        if (w != pw) {
            other = w;
            break;
        }
    }
    Q_ASSERT(other);

    const QPoint bestOffset = bestMoveOffset(pw, other);
    const int x = pw->x() - bestOffset.x();
    const int y = pw->y() - bestOffset.y();

    pw->setMovedX(x);
    pw->setMovedY(y);
}

void MonitorsGround::reloadViewPortSize()
{
    int w = 0;
    for (auto pw : m_monitors.keys())
        w = std::max(w, pw->x() + pw->w());

    int h = 0;
    for (auto pw : m_monitors.keys())
        h = std::max(h, pw->y() + pw->h());

    m_viewPortWidth = w;
    m_viewPortHeight = h;
}

void MonitorsGround::applySettings()
{
    for (auto it(m_monitors.cbegin()); it != m_monitors.cend(); ++it)
        Q_EMIT requestApplySettings(it.value(), it.key()->x(), it.key()->y());
}

bool MonitorsGround::isScreenPerfect() const
{
    // only support 2 screens
    if (m_monitors.size() != 2)
        return true;

    MonitorProxyWidget *p0 = m_monitors.firstKey();
    MonitorProxyWidget *p1 = m_monitors.lastKey();

    const QRect r0(p0->x(), p0->y(), p0->w(), p0->h());
    const QRect r1(p1->x(), p1->y(), p1->w(), p1->h());

    if (r0 == r1)
        return true;
    if (r0.intersects(r1))
        return false;

    return bestMoveOffset(p0, p1).isNull();
}

double MonitorsGround::screenScale() const
{
    const double scaleW = double(width()) / m_viewPortWidth;
    const double scaleH = double(height()) / m_viewPortHeight;

    return std::min(scaleW, scaleH);
}

const QPoint MonitorsGround::bestMoveOffset(MonitorProxyWidget *pw0, MonitorProxyWidget *pw1) const
{
    const QPoint selfTopLeft = QPoint(pw0->x(), pw0->y());
    QList<QPoint> selfPoints;
    selfPoints.append(selfTopLeft);
    selfPoints.append(QPoint(pw0->x() + pw0->w(), pw0->y()));
    selfPoints.append(QPoint(pw0->x(), pw0->y() + pw0->h()));
    selfPoints.append(QPoint(pw0->x() + pw0->w(), pw0->y() + pw0->h()));

    const QPoint otherTopLeft = QPoint(pw1->x(), pw1->y());
    const QRect r1(pw1->x(), pw1->y(), pw1->w(), pw1->h());
    QList<QPoint> otherPoints;
    otherPoints.append(otherTopLeft);
    otherPoints.append(QPoint(pw1->x() + pw1->w(), pw1->y()));
    otherPoints.append(QPoint(pw1->x(), pw1->y() + pw1->h()));
    otherPoints.append(QPoint(pw1->x() + pw1->w(), pw1->y() + pw1->h()));

    // TODO: check screen rect contains another screen and size not equal
    QPoint bestOffset;
    int min = INT_MAX;
    for (auto p1 : selfPoints) {
        for (auto p2 : otherPoints) {
            const int m = (p2 - p1).manhattanLength();
            if (m >= min)
                continue;

            const QPoint offset = p1 - p2;

            // test intersect
            const QRect r0(pw0->x() - offset.x(), pw0->y() - offset.y(), pw0->w(), pw0->h());
            if (r0.intersects(r1))
                continue;

            min = m;
            bestOffset = offset;
        }
    }

    return bestOffset;
}

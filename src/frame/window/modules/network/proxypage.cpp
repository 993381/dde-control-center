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

#include "proxypage.h"
#include "widgets/buttontuple.h"
#include "widgets/translucentframe.h"
#include "widgets/settingsgroup.h"
#include "widgets/settingsheaderitem.h"
#include "widgets/lineeditwidget.h"
#include "widgets/settingsitem.h"
#include <networkmodel.h>

#include <DPalette>
#include <DApplicationHelper>

#include <QVBoxLayout>
#include <QDebug>

DWIDGET_USE_NAMESPACE

using namespace dcc;
using namespace dcc::widgets;
using namespace dde::network;

namespace DCC_NAMESPACE {
namespace network {
const QStringList ProxyMethodList = { "none", "manual", "auto" };

ProxyPage::ProxyPage(QWidget *parent)
    : QWidget(parent)
    , m_manualWidget(new ContentWidget)
    , m_autoWidget(new TranslucentFrame)
    , m_buttonTuple(new ButtonTuple(ButtonTuple::Save))
    , m_proxyTabs(new DButtonBox)
{
    m_buttonTuple->leftButton()->setText(tr("Cancel"));
    m_buttonTuple->rightButton()->setText(tr("Save"));

    DButtonBoxButton *tabNoneBtn = new DButtonBoxButton(tr("None"));
    DButtonBoxButton *tabManualBtn = new DButtonBoxButton(tr("Manual"));
    DButtonBoxButton *tabAutoBtn = new DButtonBoxButton(tr("Auto"));
    m_tablist.append(tabNoneBtn);
    m_tablist.append(tabManualBtn);
    m_tablist.append(tabAutoBtn);
    m_proxyTabs->setButtonList(m_tablist, true);
    m_proxyTabs->setId(tabNoneBtn, 0);
    m_proxyTabs->setId(tabManualBtn, 1);
    m_proxyTabs->setId(tabAutoBtn, 2);
    tabManualBtn->setChecked(true);

    m_httpAddr = new LineEditWidget;
    m_httpAddr->setPlaceholderText(tr("Optional"));
    m_httpAddr->setTitle(tr("HTTP Proxy"));
    m_httpPort = new LineEditWidget;
    m_httpPort->setPlaceholderText(tr("Optional"));
    m_httpPort->setTitle(tr("Port"));

    m_httpsAddr = new LineEditWidget;
    m_httpsAddr->setPlaceholderText(tr("Optional"));
    m_httpsAddr->setTitle(tr("HTTPS Proxy"));
    m_httpsPort = new LineEditWidget;
    m_httpsPort->setPlaceholderText(tr("Optional"));
    m_httpsPort->setTitle(tr("Port"));

    m_ftpAddr = new LineEditWidget;
    m_ftpAddr->setPlaceholderText(tr("Optional"));
    m_ftpAddr->setTitle(tr("FTP Proxy"));
    m_ftpPort = new LineEditWidget;
    m_ftpPort->setPlaceholderText(tr("Optional"));
    m_ftpPort->setTitle(tr("Port"));

    m_socksAddr = new LineEditWidget;
    m_socksAddr->setPlaceholderText(tr("Optional"));
    m_socksAddr->setTitle(tr("SOCKS Proxy"));
    m_socksPort = new LineEditWidget;
    m_socksPort->setPlaceholderText(tr("Optional"));
    m_socksPort->setTitle(tr("Port"));

    m_ignoreList = new DTextEdit;
    QLabel *ignoreTips = new QLabel;
    ignoreTips->setWordWrap(true);
    ignoreTips->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ignoreTips->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    ignoreTips->setText(tr("Ignore the proxy configurations for the above hosts and domains"));

    m_autoUrl = new LineEditWidget;
    m_autoUrl->setPlaceholderText(tr("Optional"));
    m_autoUrl->setTitle(tr("Configuration URL"));

    m_httpGroup = new SettingsGroup;
    m_httpGroup->appendItem(m_httpAddr);
    m_httpGroup->appendItem(m_httpPort);

    m_httpsGroup = new SettingsGroup;
    m_httpsGroup->appendItem(m_httpsAddr);
    m_httpsGroup->appendItem(m_httpsPort);

    m_ftpGroup = new SettingsGroup;
    m_ftpGroup->appendItem(m_ftpAddr);
    m_ftpGroup->appendItem(m_ftpPort);

    m_socksGroup = new SettingsGroup;
    m_socksGroup->appendItem(m_socksAddr);
    m_socksGroup->appendItem(m_socksPort);

    SettingsGroup *autoGroup = new SettingsGroup;
    autoGroup->appendItem(m_autoUrl);

    QVBoxLayout *manualLayout = new QVBoxLayout;
    manualLayout->addWidget(m_httpGroup);
    manualLayout->addWidget(m_httpsGroup);
    manualLayout->addWidget(m_ftpGroup);
    manualLayout->addWidget(m_socksGroup);
    manualLayout->addWidget(m_ignoreList);
    manualLayout->addWidget(ignoreTips);
    manualLayout->setMargin(0);
    manualLayout->setSpacing(10);

    QVBoxLayout *autoLayout = new QVBoxLayout;
    autoLayout->addWidget(autoGroup);
    autoLayout->setSpacing(10);
    autoLayout->setMargin(0);
    m_autoWidget->setLayout(autoLayout);

    QWidget *manualWidget = new TranslucentFrame;
    manualWidget->setLayout(manualLayout);
    m_manualWidget->setContent(manualWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->addWidget(m_proxyTabs);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(m_manualWidget);
    mainLayout->addWidget(m_autoWidget);
    mainLayout->addSpacing(10);
    mainLayout->addStretch();
    mainLayout->addWidget(m_buttonTuple);
    mainLayout->setSpacing(0);

    setWindowTitle(tr("System Proxy"));
    setLayout(mainLayout);

//    connect(m_buttonTuple->leftButton(), &QPushButton::clicked, this, &ProxyPage::back);
//    connect(m_buttonTuple->rightButton(), &QPushButton::clicked, this, &ProxyPage::back, Qt::QueuedConnection);
    connect(m_buttonTuple->rightButton(), &QPushButton::clicked, this, &ProxyPage::applySettings);
    connect(m_proxyTabs, &DButtonBox::buttonClicked, this, [this](QAbstractButton *value) {
        onProxyToggled(m_proxyTabs->id(value));
    });
//    connect(m_proxyType, &DSegmentedControl::currentChanged, [=](const int index) { Q_EMIT requestSetProxyMethod(ProxyMethodList[index]); });
//    connect(m_ignoreList, &QPlainTextEdit::textChanged, [=] { Q_EMIT requestSetIgnoreHosts(m_ignoreList->toPlainText()); });
//    connect(m_httpAddr->textEdit(), &QLineEdit::editingFinished, [=] { applyProxy("http"); });
//    connect(m_httpPort->textEdit(), &QLineEdit::editingFinished, [=] { applyProxy("http"); });
//    connect(m_httpsAddr->textEdit(), &QLineEdit::editingFinished, [=] { applyProxy("https"); });
//    connect(m_httpsPort->textEdit(), &QLineEdit::editingFinished, [=] { applyProxy("https"); });
//    connect(m_ftpAddr->textEdit(), &QLineEdit::editingFinished, [=] { applyProxy("ftp"); });
//    connect(m_ftpPort->textEdit(), &QLineEdit::editingFinished, [=] { applyProxy("ftp"); });
//    connect(m_socksAddr->textEdit(), &QLineEdit::editingFinished, [=] { applyProxy("socks"); });
//    connect(m_socksPort->textEdit(), &QLineEdit::editingFinished, [=] { applyProxy("socks"); });
//    connect(m_autoUrl->textEdit(), &QLineEdit::editingFinished, [=] { Q_EMIT requestSetAutoProxy(m_autoUrl->text()); });

    QTimer::singleShot(1, this, [ = ] {
        Q_EMIT requestQueryProxyData();
    });
}

void ProxyPage::setModel(NetworkModel *model)
{
    m_model = model;

    connect(model, &NetworkModel::proxyMethodChanged, this, &ProxyPage::onProxyMethodChanged);
    connect(model, &NetworkModel::proxyIgnoreHostsChanged, this, &ProxyPage::onIgnoreHostsChanged);
    connect(model, &NetworkModel::proxyChanged, this, &ProxyPage::onProxyChanged);
    connect(model, &NetworkModel::autoProxyChanged, m_autoUrl, &LineEditWidget::setText);

    onProxyChanged("http", model->proxy("http"));
    onProxyChanged("https", model->proxy("https"));
    onProxyChanged("ftp", model->proxy("ftp"));
    onProxyChanged("socks", model->proxy("socks"));
    onProxyMethodChanged(model->proxyMethod());
    onIgnoreHostsChanged(model->ignoreHosts());
    m_autoUrl->setText(model->autoProxy());
}

void ProxyPage::onProxyMethodChanged(const QString &proxyMethod)
{
    const int index = ProxyMethodList.indexOf(proxyMethod);
    if ((index > 2) || (index < 0))
        return;

    m_tablist[index]->setChecked(true);
    onProxyToggled(index);
}

void ProxyPage::onProxyToggled(const int index)
{
    // refersh ui
    m_manualWidget->setVisible(index == 1);
    m_autoWidget->setVisible(index == 2);

    setFocus();
}

void ProxyPage::applySettings() const
{
    bool ok = true;
    if (!m_httpPort->text().isEmpty()) {
        m_httpPort->text().toUInt(&ok);
        if (!ok) {
            m_httpPort->setIsErr(true);
            m_httpPort->dTextEdit()->showAlertMessage(tr("Invalid port"), m_httpGroup, 2000);
            return;
        }
    }

    if (!m_httpsPort->text().isEmpty()) {
        m_httpsPort->text().toUInt(&ok);
        if (!ok) {
            m_httpsPort->setIsErr(true);
            m_httpsPort->dTextEdit()->showAlertMessage(tr("Invalid port"), m_httpsGroup, 2000);
            return;
        }
    }

    if (!m_ftpPort->text().isEmpty()) {
        m_ftpPort->text().toUInt(&ok);
        if (!ok) {
            m_ftpPort->setIsErr(true);
            m_ftpPort->dTextEdit()->showAlertMessage(tr("Invalid port"), m_ftpGroup, 2000);
            return;
        }
    }

    if (!m_socksPort->text().isEmpty()) {
        m_socksPort->text().toUInt(&ok);
        if (!ok) {
            m_socksPort->setIsErr(true);
            m_socksPort->dTextEdit()->showAlertMessage(tr("Invalid port"), m_socksGroup, 2000);
            return;
        }
    }

    Q_EMIT requestSetProxy("http", m_httpAddr->text(), m_httpPort->text());
    Q_EMIT requestSetProxy("https", m_httpsAddr->text(), m_httpsPort->text());
    Q_EMIT requestSetProxy("ftp", m_ftpAddr->text(), m_ftpPort->text());
    Q_EMIT requestSetProxy("socks", m_socksAddr->text(), m_socksPort->text());

    Q_EMIT requestSetIgnoreHosts(m_ignoreList->toPlainText());

    Q_EMIT requestSetAutoProxy(m_autoUrl->text());

    Q_EMIT requestSetProxyMethod(ProxyMethodList[m_proxyTabs->checkedId()]);
}

void ProxyPage::onIgnoreHostsChanged(const QString &hosts)
{
    const QTextCursor cursor = m_ignoreList->textCursor();

    m_ignoreList->blockSignals(true);
    m_ignoreList->setPlainText(hosts);
    m_ignoreList->setTextCursor(cursor);
    m_ignoreList->blockSignals(false);
}

void ProxyPage::applyProxy(const QString &type)
{
    if (type == "http") {
        Q_EMIT requestSetProxy("http", m_httpAddr->text(), m_httpPort->text());
    } else if (type == "https") {
        Q_EMIT requestSetProxy("https", m_httpsAddr->text(), m_httpsPort->text());
    } else if (type == "ftp") {
        Q_EMIT requestSetProxy("ftp", m_ftpAddr->text(), m_ftpPort->text());
    } else if (type == "socks") {
        Q_EMIT requestSetProxy("socks", m_socksAddr->text(), m_socksPort->text());
    }
}

void ProxyPage::onProxyChanged(const QString &type, const ProxyConfig &config)
{
    if (type == "http") {
        m_httpAddr->setText(config.url);
        m_httpPort->setText(QString::number(config.port));
    } else if (type == "https") {
        m_httpsAddr->setText(config.url);
        m_httpsPort->setText(QString::number(config.port));
    } else if (type == "ftp") {
        m_ftpAddr->setText(config.url);
        m_ftpPort->setText(QString::number(config.port));
    } else if (type == "socks") {
        m_socksAddr->setText(config.url);
        m_socksPort->setText(QString::number(config.port));
    }
}
}
}

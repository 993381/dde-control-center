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

#include "modifypasswordpage.h"
#include "settingsgroup.h"
#include "translucentframe.h"

#include <QVBoxLayout>
#include <QDebug>
#include <QPushButton>

using namespace dcc::widgets;
using namespace dcc::accounts;

ModifyPasswordPage::ModifyPasswordPage(User *user, QWidget *parent)
    : ContentWidget(parent),

      m_userInter(user),

      m_oldpwdEdit(new LineEditWidget),
      m_pwdEdit(new LineEditWidget),
      m_pwdEditRepeat(new LineEditWidget),

      m_buttonTuple(new ButtonTuple)
{
    m_oldpwdEdit->textEdit()->setEchoMode(QLineEdit::Password);
    m_oldpwdEdit->setTitle(tr("Current Password"));
    m_pwdEdit->textEdit()->setEchoMode(QLineEdit::Password);
    m_pwdEdit->setTitle(tr("New Password"));
    m_pwdEditRepeat->textEdit()->setEchoMode(QLineEdit::Password);
    m_pwdEditRepeat->setTitle(tr("Repeat Password"));

    m_oldpwdEdit->setPlaceholderText(tr("Required"));
    m_pwdEdit->setPlaceholderText(tr("Required"));
    m_pwdEditRepeat->setPlaceholderText(tr("Required"));

    QPushButton *cancel = m_buttonTuple->leftButton();
    QPushButton *accept = m_buttonTuple->rightButton();
    cancel->setText(tr("Cancel"));
    cancel->setAccessibleName("Modify_Cancel");
    accept->setText(tr("Accept"));
    accept->setAccessibleName("Modify_Accept");

    SettingsGroup *pwdGroup = new SettingsGroup;
    pwdGroup->appendItem(m_oldpwdEdit);
    pwdGroup->appendItem(m_pwdEdit);
    pwdGroup->appendItem(m_pwdEditRepeat);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addSpacing(10);
    mainLayout->addWidget(pwdGroup);
    mainLayout->addWidget(m_buttonTuple);
    mainLayout->setSpacing(10);
    mainLayout->setMargin(0);

    TranslucentFrame *mainWidget = new TranslucentFrame;
    mainWidget->setLayout(mainLayout);

    setContent(mainWidget);

    connect(accept, &QPushButton::clicked, this, &ModifyPasswordPage::passwordSubmit);
    connect(cancel, &QPushButton::clicked, this, &ModifyPasswordPage::back);
    connect(m_pwdEditRepeat->textEdit(), &QLineEdit::editingFinished, this, &ModifyPasswordPage::checkPwd);
    connect(m_pwdEdit->textEdit(), &QLineEdit::textEdited, m_pwdEdit, &LineEditWidget::hideAlertMessage);
    connect(user, &User::passwordModifyFinished, this, &ModifyPasswordPage::onPasswordChangeFinished);
    connect(user, &User::fullnameChanged, this, &ModifyPasswordPage::updateTitle);
    connect(this, &ModifyPasswordPage::disappear, m_pwdEdit, &LineEditWidget::hideAlertMessage);

    updateTitle();
}

void ModifyPasswordPage::passwordSubmit()
{
    const QString pwdOld = m_oldpwdEdit->textEdit()->text();
    const QString pwd0 = m_pwdEdit->textEdit()->text();
    const QString pwd1 = m_pwdEditRepeat->textEdit()->text();

    checkPwd();

    if (m_oldpwdEdit->text() == m_pwdEdit->text()) {
        m_pwdEdit->showAlertMessage(tr("New password should differ from the current one"));
        return;
    }

    if (m_pwdEdit->text().isEmpty() || m_pwdEditRepeat->text().isEmpty() || m_oldpwdEdit->text().isEmpty())
        return;

    if (pwd0 != pwd1)
        return;

    emit requestChangePassword(m_userInter, pwdOld, pwd0);
}

void ModifyPasswordPage::checkPwd()
{
    m_oldpwdEdit->setIsErr(m_oldpwdEdit->text().isEmpty());
    m_pwdEdit->setIsErr(m_pwdEdit->text().isEmpty() || m_oldpwdEdit->text() == m_pwdEdit->text());
    m_pwdEditRepeat->setIsErr(m_pwdEditRepeat->text() != m_pwdEdit->text());
}

void ModifyPasswordPage::onPasswordChangeFinished(const int exitCode)
{
    if (exitCode == 0)
    {
        emit back();
        return;
    } else {
        qWarning() << Q_FUNC_INFO << "exit =" << exitCode;

        m_oldpwdEdit->setIsErr();
    }
}

void ModifyPasswordPage::updateTitle()
{
    const QString &fullname = m_userInter->fullname();
    setTitle(tr("Password") + " - " + (fullname.isEmpty() ? m_userInter->name() : fullname));
}

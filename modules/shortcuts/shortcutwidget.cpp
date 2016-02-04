/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include <QDebug>
#include <QMouseEvent>
#include <QMessageBox>

#include <libdui/dthememanager.h>
#include <libdui/dconstants.h>

#include "imagenamebutton.h"

#include "shortcutwidget.h"

ShortcutWidget::ShortcutWidget(ShortcutDbus *dbus, int id, const QString &title, const QString &shortcut, QWidget *parent):
    QFrame(parent),
    m_hlayout(new QHBoxLayout),
    m_vlayout(new QVBoxLayout),
    m_id(id),
    m_title(new QLabel(title)),
    m_shortcut(new ShortcutEdit(dbus)),
    m_animation(new QPropertyAnimation(this))
{
    m_title->setObjectName("ShortcutTitle");
    m_shortcut->setObjectName("ShortcutValue");

    D_THEME_INIT_WIDGET(ShortcutWidget);

    m_shortcut->setShortcut(shortcut);

    m_hlayout->setMargin(0);
    m_hlayout->addSpacing(HEADER_LEFT_MARGIN);
    m_hlayout->addWidget(m_title, 0, Qt::AlignVCenter|Qt::AlignLeft);
    m_hlayout->addWidget(m_shortcut, 0, Qt::AlignVCenter|Qt::AlignRight);
    m_hlayout->addSpacing(HEADER_RIGHT_MARGIN);

    m_vlayout->setMargin(0);
    m_vlayout->addLayout(m_hlayout);

    setLayout(m_vlayout);

    m_me = this;

    m_removeButton = new ImageNameButton("list_remove", this);
    m_removeButton->move(-m_removeButton->width(), m_removeButton->y());

    connect(m_shortcut, &ShortcutEdit::shortcutKeyFinished, this, &ShortcutWidget::shortcutKeyFinished);

    connect(m_removeButton, &DImageButton::clicked, [=]{
        emit removeShortcut();
    });

    connect(m_animation, &QPropertyAnimation::valueChanged, [=](const QVariant &value){
        /// 0 is normal spacing index for m_hlayout
        delete m_hlayout->takeAt(0);
        m_hlayout->insertSpacing(0, HEADER_LEFT_MARGIN+value.toRect().right()/1.5);
    });

    m_animation->setTargetObject(m_removeButton);
    m_animation->setPropertyName("geometry");

    setFixedHeight(RADIO_ITEM_HEIGHT);
}

void ShortcutWidget::setId(int id)
{
    m_id = id;
}

void ShortcutWidget::setTitle(const QString &title)
{
    if(title == m_title->text())
        return;

    emit keyWordChanged(m_title->text(), title);

    m_title->setText(title);
}

void ShortcutWidget::setShortcut(const QString &shortcut)
{
    if (m_shortcuText == shortcut)
        return;

    m_shortcut->setShortcut(shortcut);

    emit keyWordChanged(m_shortcuText, shortcut);

    m_shortcuText = shortcut;
}

void ShortcutWidget::showRemoveButton() const
{
    m_animation->setStartValue(QRect(-IMAGE_BUTTON_WIDTH, 0, IMAGE_BUTTON_WIDTH, 30));
    m_animation->setEndValue(QRect(HEADER_LEFT_MARGIN, 0, IMAGE_BUTTON_WIDTH, 30));
    m_animation->start();
    m_shortcut->setEnableEdit(false);
}

void ShortcutWidget::hideRemoveButton() const
{
    m_animation->setEndValue(QRect(-IMAGE_BUTTON_WIDTH, 0, IMAGE_BUTTON_WIDTH, 30));
    m_animation->setStartValue(QRect(HEADER_LEFT_MARGIN, 0, IMAGE_BUTTON_WIDTH, 30));
    m_animation->start();
    m_shortcut->setEnableEdit(true);
}

void ShortcutWidget::setEnableEdit(bool enable)
{
    m_shortcut->setEnableEdit(enable);
}

int ShortcutWidget::id() const
{
    return m_id;
}

QString ShortcutWidget::title() const
{
    return m_title->text();
}

QString ShortcutWidget::shortcut() const
{
    return m_shortcut->shortcut();
}

QStringList ShortcutWidget::keyWords() const
{
    return QStringList()<<m_title->text()<<m_shortcut->shortcut();
}

void ShortcutWidget::setData(const QVariant &datas)
{
    QVariantList list = datas.toList();
    if(list.count()==3&&list[0].type()==QVariant::Int
            &&list[1].type()==QVariant::String&&list[2].type()==QVariant::String){
        setId(list[0].toInt());
        setTitle(list[1].toString());
        setShortcut(list[2].toString());
    }
}

QVariant ShortcutWidget::getData()
{
    return QVariantList()<<m_id<<m_title->text()<<m_shortcut->shortcut();
}

QWidget *ShortcutWidget::widget() const
{
    return m_me;
}

void ShortcutWidget::shortcutKeyFinished(const QString &str)
{
    int split_pos = str.indexOf("_");
    if(split_pos<0||split_pos>=str.count()-1)
        return;

    QString flag = str.mid(0, split_pos);
    QString key = str.mid(split_pos+1);

    if(key.toLower() != shortcut().toLower())
        emit shortcutChanged(flag, key);
}



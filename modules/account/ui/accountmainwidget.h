/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QFrame>
#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QStackedWidget>
#include "moduleheader.h"
#include "user_list/userlistpanel.h"
#include "user_create/createuserpanel.h"
#include "libdui/dconstants.h"
#include "libdui/dtextbutton.h"
#include "libdui/dthememanager.h"
#include "libdui/dseparatorhorizontal.h"
#include "generalremovebutton.h"
#include "generaladdbutton.h"
#include "dynamiclabel.h"

class AccountMainWidget : public QFrame
{
    Q_OBJECT
public:
    enum PanelState {
        StateCreating,
        StateDeleting,
        StateSetting,
        StateNormal
    };

    explicit AccountMainWidget(QWidget *parent = 0);
    void preDestroy();

signals:
    void stateChanged(PanelState state);
    void requestDelete(bool flag);
    void cancelDelete();
    void hideForSetting();
    void showForNormal();

private:
    void initHeader();
    void initListPanel();
    void initCreatePanel();
    void initHeaderStackWidget();
    void initDBusAccount();

    void setPanelState(PanelState state);

    void onAddButtonClicked();
    void onAddButtonMouseEntered();
    void onDeleteButtonStateChanged();
    void onDeleteButtonMouseEntered();
private:
    GeneralAddButton *m_addUserButton = NULL;
    GeneralRemoveButton *m_deleteUserButton = NULL;
    QStackedWidget *m_headerStackWidget = NULL;
    CreateUserPanel *m_createPanel = NULL;
    DynamicLabel *m_buttonToolTip = NULL;
    QStackedWidget *m_stackWidget = NULL;
    QScrollArea *m_listScrollArea = NULL;
    UserListPanel *m_listPanel = NULL;
    QVBoxLayout *m_mainLayout = NULL;
    ModuleHeader *m_header = NULL;
    DBusAccount *m_account = NULL;

    PanelState m_state = StateNormal;
};

#endif // MAINWIDGET_H

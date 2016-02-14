/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include <QDebug>
#include <QApplication>
#include <QFrame>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QList>
#include <QVariant>

#include <libdui/dslider.h>
#include <libdui/dseparatorhorizontal.h>
#include <libdui/dlineedit.h>
#include <libdui/dheaderline.h>
#include <libdui/dswitchbutton.h>
#include <libdui/dimagebutton.h>
#include <libdui/dthememanager.h>
#include <libdui/dtextbutton.h>
#include <libdui/dbuttonlist.h>
#include <libdui/darrowlineexpand.h>
#include <libdui/dsearchedit.h>
#include <libdui/anchors.h>
#include <libdui/denhancedwidget.h>

#include "moduleheader.h"
#include "normallabel.h"
#include "addrmdoneline.h"
#include "genericlistitem.h"

#include "keyboard.h"
#include "dbus/dbusinputdevices.h"
#include "dbus/dbuslangselector.h"
#include "firstletterclassify.h"

DUI_USE_NAMESPACE

Keyboard::Keyboard() :
    QObject(),
    m_frame(new QFrame),
    m_letterClassifyList(nullptr),
    m_settings(new QSettings("deepin", "dde-control-center-kayboard", this)),
    m_dbusKeyboard(nullptr)
{
    Q_UNUSED(QT_TRANSLATE_NOOP("ModuleName", "Keyboard and Language"));

    Q_INIT_RESOURCE(widgets_theme_dark);
    Q_INIT_RESOURCE(widgets_theme_light);

    setAutoDelete(false);
    initBackend();

    if (m_dbusKeyboard)
        initUI();
}

Keyboard::~Keyboard()
{
    qDebug() << "~Keyboard and Language";
    m_frame->deleteLater();
}

void Keyboard::initBackend()
{
    DBusInputDevices * dbusInputDevices = new DBusInputDevices(this);
    foreach (InputDevice device, dbusInputDevices->infos()) {
        if (device.deviceType == "keyboard") {
            m_dbusKeyboard = new DBusKeyboard(this);
            break;
        }
    }
}

void Keyboard::updateKeyboardLayout(SearchList *button_list, AddRmDoneLine *line, bool showRemoveButton)
{
    QStringList user_layout_list = m_dbusKeyboard->userLayoutList();
    QString current_layout = m_dbusKeyboard->currentLayout();

    foreach (const QString &str, user_layout_list) {
        QDBusPendingReply<QString> tmp = m_dbusKeyboard->GetLayoutDesc(str);
        tmp.waitForFinished();
        QString title = tmp.value();

        m_mapUserLayoutInfo[title] = str;

        GenericListItem *item = new GenericListItem(showRemoveButton && str!=current_layout);
        connect(item, &GenericListItem::removeButtonClicked, [=]{
            m_dbusKeyboard->DeleteUserLayout(str);

            const QChar letterFirst = title[0];
            if(letterFirst.isUpper() || letterFirst.isLower()){
                onAddLayoutItem(str, title, QStringList() << QString(letterFirst));
            }else{
                QDBusInterface dbus_pinyin( "com.deepin.api.Pinyin", "/com/deepin/api/Pinyin",
                                                  "com.deepin.api.Pinyin" );
                QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(dbus_pinyin.asyncCall("Query", QString(title[0])), this);
                connect(watcher, &QDBusPendingCallWatcher::finished, watcher, [this, watcher, title, str]{
                    QDBusPendingReply<QStringList> reply = *watcher;
                    if(!reply.isError()){
                        onAddLayoutItem(str, title, reply.value());
                    }
                    watcher->deleteLater();
                });
            }
        });
        connect(line, &AddRmDoneLine::removeClicked, item, &GenericListItem::showRemoveButton);
        connect(line, &AddRmDoneLine::doneClicked, item, &GenericListItem::hideRemoveButton);
        item->setTitle(title);
        m_mapUserLayoutIndex[str] = button_list->addItem(item);

        if(str == current_layout){
            button_list->setCheckable(true);
            button_list->setCheckedItem(button_list->count()-1);
            button_list->setCheckable(!showRemoveButton);
        }
    }

    button_list->setWidgetResizable(true);
    button_list->setFixedSize(310, qMin(button_list->count()*EXPAND_HEADER_HEIGHT, 200));
}

void Keyboard::initUI()
{
    m_frame->setFixedWidth(310);
    m_frame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    m_mainLayout = new QVBoxLayout(m_frame);
    m_mainLayout->setMargin(10);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    /// Header
    ModuleHeader * header = new ModuleHeader(tr("Keyboard and Language"));
    m_mainLayout->addWidget(header);
    m_mainLayout->addWidget(new DSeparatorHorizontal);
    m_mainLayout->addSpacing(10);
    connect(header, &ModuleHeader::resetButtonClicked, m_dbusKeyboard, &DBusKeyboard::Reset);

    /// Basic Settings
    QGridLayout * basicSettingsLayout = new QGridLayout;
    basicSettingsLayout->setColumnStretch(0, 1);
    basicSettingsLayout->setColumnMinimumWidth(1, 180);
    basicSettingsLayout->setSpacing(20);
    basicSettingsLayout->setContentsMargins(0, 0, 14, 0);

    NormalLabel * repeatDelayTitle = new NormalLabel(tr("Repeat Delay"));
    DSlider * repeatDelaySlider = new DSlider(Qt::Horizontal);
    repeatDelaySlider->setRange(20, 600);
    repeatDelaySlider->setValue(m_dbusKeyboard->repeatDelay());
    basicSettingsLayout->addWidget(repeatDelayTitle, 0, 0, Qt::AlignRight);
    basicSettingsLayout->addWidget(repeatDelaySlider, 0, 1);

    QTimer *repeatDelayTimer = new QTimer(this);
    repeatDelayTimer->setInterval(100);
    repeatDelayTimer->setSingleShot(true);

    connect(repeatDelaySlider, &DSlider::valueChanged, repeatDelayTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(repeatDelayTimer, &QTimer::timeout, [=] {
        m_dbusKeyboard->setRepeatDelay(repeatDelaySlider->value());
    });

    connect(m_dbusKeyboard, &DBusKeyboard::RepeatDelayChanged,
            repeatDelaySlider, [repeatDelaySlider, this]{
        repeatDelaySlider->setValue(m_dbusKeyboard->repeatDelay());
    });

    NormalLabel * repeatSpeedTitle = new NormalLabel(tr("Repeat Rate"));
    DSlider * repeatSpeedSlider = new DSlider(Qt::Horizontal);
    repeatSpeedSlider->setRange(200, 1000);
    repeatSpeedSlider->setValue(1000 - (m_dbusKeyboard->repeatInterval() * 10 - 200));
    basicSettingsLayout->addWidget(repeatSpeedTitle, 1, 0, Qt::AlignRight);
    basicSettingsLayout->addWidget(repeatSpeedSlider, 1, 1);

    QTimer *repeatSpeedTimer = new QTimer(this);
    repeatSpeedTimer->setInterval(100);
    repeatSpeedTimer->setSingleShot(true);

    connect(repeatSpeedSlider, &DSlider::valueChanged, repeatSpeedTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(repeatSpeedTimer, &QTimer::timeout, [=] {
        m_dbusKeyboard->setRepeatInterval(((1000 - repeatSpeedSlider->value()) + 200) / 10);
    });

    connect(m_dbusKeyboard, &DBusKeyboard::RepeatIntervalChanged,
            repeatSpeedSlider, [repeatSpeedSlider, this]{
        repeatSpeedSlider->setValue(1000 - (m_dbusKeyboard->repeatInterval() * 10 - 200));
    });

    NormalLabel * cursorBlinkIntervalTitle = new NormalLabel(tr("Cursor Blink Rate"));
    DSlider * cursorBlinkIntervalSlider = new DSlider(Qt::Horizontal);
    cursorBlinkIntervalSlider->setRange(100, 2500);
    cursorBlinkIntervalSlider->setValue(2500 - (m_dbusKeyboard->cursorBlink() - 100));
    qApp->setCursorFlashTime(m_dbusKeyboard->cursorBlink());
    basicSettingsLayout->addWidget(cursorBlinkIntervalTitle, 2, 0, Qt::AlignRight);
    basicSettingsLayout->addWidget(cursorBlinkIntervalSlider, 2, 1);

    QTimer *cursorBlinkIntervalTimer = new QTimer(this);
    cursorBlinkIntervalTimer->setInterval(100);
    cursorBlinkIntervalTimer->setSingleShot(true);

    connect(cursorBlinkIntervalSlider, &DSlider::valueChanged, cursorBlinkIntervalTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(cursorBlinkIntervalTimer, &QTimer::timeout, [=] {
        m_dbusKeyboard->setCursorBlink((2500 - cursorBlinkIntervalSlider->value()) + 100);
    });
    connect(m_dbusKeyboard, &DBusKeyboard::CursorBlinkChanged,
            cursorBlinkIntervalSlider, [cursorBlinkIntervalSlider, this]{
        cursorBlinkIntervalSlider->setValue(2500 - (m_dbusKeyboard->cursorBlink() - 100));
        qApp->setCursorFlashTime(m_dbusKeyboard->cursorBlink());
    });

    NormalLabel * testAreaTitle = new NormalLabel(tr("Test Area"));
    DLineEdit * testAreaEdit = new DLineEdit;
    connect(header, &ModuleHeader::resetButtonClicked, testAreaEdit, &DLineEdit::clear);

    basicSettingsLayout->addWidget(testAreaTitle, 3, 0, Qt::AlignRight);
    basicSettingsLayout->addWidget(testAreaEdit, 3, 1);

    m_mainLayout->addLayout(basicSettingsLayout);
    m_mainLayout->addSpacing(10);
    m_mainLayout->addWidget(new DSeparatorHorizontal);

    DHeaderLine * capsLockLine = new DHeaderLine;
    DSwitchButton * capsLockSwitch = new DSwitchButton(capsLockLine);
    capsLockSwitch->setChecked(m_dbusKeyboard->capslockToggle());
    capsLockLine->setTitle(tr("Caps Lock prompt"));
    capsLockLine->setContent(capsLockSwitch);

    connect(capsLockSwitch, &DSwitchButton::checkedChanged, [this, capsLockSwitch]{
        m_dbusKeyboard->setCapslockToggle(capsLockSwitch->checked());
    });

    connect(m_dbusKeyboard, &DBusKeyboard::CapslockToggleChanged,
            capsLockSwitch, [capsLockSwitch, this]{
        capsLockSwitch->setChecked(m_dbusKeyboard->capslockToggle());
    });

    m_languageTips = new QLabel;
    m_languageTips->setStyleSheet("color:#666;");
    m_languageTips->setText(m_settings->value("languageTips").toString());

    DArrowLineExpand *language_expand = new DArrowLineExpand;
    language_expand->setTitle(tr("Language"));
    language_expand->headerLine()->leftLayout()->addSpacing(10);
    language_expand->headerLine()->leftLayout()->addWidget(m_languageTips);

    AddRmDoneLine *keyboardLayoutLine = new AddRmDoneLine;
    keyboardLayoutLine->setTitle(tr("Keyboard Layout"));
    keyboardLayoutLine->setRmButtonToolTip(tr("Remove Keyboard Layout"));
    keyboardLayoutLine->setAddButtonToolTip(tr("Add Keyboard Layout"));

    SearchList *user_layout_list = new SearchList;
    user_layout_list->setItemSize(290, EXPAND_HEADER_HEIGHT);
    user_layout_list->setCheckable(true);
    user_layout_list->setEnableVerticalScroll(true);

    updateKeyboardLayout(user_layout_list, keyboardLayoutLine);
    keyboardLayoutLine->setRemoveHidden(user_layout_list->count()<2);

    connect(user_layout_list, &SearchList::countChanged,
            [user_layout_list, keyboardLayoutLine]{
        if(user_layout_list->isVisible()&&keyboardLayoutLine->doneButton()->isHidden())
            keyboardLayoutLine->setRemoveHidden(user_layout_list->count()<2);
    });

    connect(keyboardLayoutLine, &AddRmDoneLine::addClicked,
            [keyboardLayoutLine, this, user_layout_list, language_expand]{
        keyboardLayoutLine->setAddHidden(true);
        keyboardLayoutLine->setRemoveHidden(true);
        keyboardLayoutLine->setDoneHidden(false);

        user_layout_list->hide();
        m_letterClassifyList->show();
        language_expand->setExpand(false);
    });
    connect(keyboardLayoutLine, &AddRmDoneLine::removeClicked,
            [keyboardLayoutLine, user_layout_list]{
        keyboardLayoutLine->setAddHidden(true);
        keyboardLayoutLine->setRemoveHidden(true);
        keyboardLayoutLine->setDoneHidden(false);

        user_layout_list->setCheckable(false);
    });

    auto onDoneClicked = [this, keyboardLayoutLine, user_layout_list]{
        keyboardLayoutLine->setDoneHidden(true);
        keyboardLayoutLine->setAddHidden(false);
        keyboardLayoutLine->setRemoveHidden(user_layout_list->count()<2);

        m_letterClassifyList->hide();
        user_layout_list->show();
        m_letterClassifyList->removeItems(m_selectLayoutList);
        m_selectLayoutList.clear();

        user_layout_list->setCheckable(true);
    };

    connect(keyboardLayoutLine, &AddRmDoneLine::doneClicked, onDoneClicked);

    connect(language_expand, &DArrowLineExpand::expandChange,
            [onDoneClicked, keyboardLayoutLine](bool arg){
        if(arg&&keyboardLayoutLine->doneButton()->isVisible()){
            onDoneClicked();
        }
    });

    connect(user_layout_list, &SearchList::checkedItemChanged, [this, user_layout_list](int index){
        if(index<0)
            return;

        QString str = m_mapUserLayoutInfo[user_layout_list->getItemData(index).toString()];

        if(m_dbusKeyboard->currentLayout() != str)
            m_dbusKeyboard->setCurrentLayout(str);
    });
    connect(m_dbusKeyboard, &DBusKeyboard::CurrentLayoutChanged, [this, user_layout_list]{
        QString str = m_dbusKeyboard->currentLayout();
        int index = m_mapUserLayoutIndex[str];
        if(index>=0&&index<user_layout_list->count())
            user_layout_list->setCheckedItem(index);
    });

    connect(m_dbusKeyboard, &DBusKeyboard::UserLayoutListChanged,
            [this, user_layout_list, keyboardLayoutLine]{
        user_layout_list->clear();
        updateKeyboardLayout(user_layout_list, keyboardLayoutLine,
                             user_layout_list->isVisible()
                             &&keyboardLayoutLine->doneButton()->isVisible());
    });

    QFrame *lang_list_frame = new QFrame;
    lang_list_frame->setFixedWidth(310);

    QVBoxLayout *lang_frame_layout = new QVBoxLayout;
    lang_frame_layout->setMargin(0);

    DSearchEdit *lang_search = new DSearchEdit;
    lang_search->setPlaceHolder(tr("Search"));
    lang_search->setFixedWidth(290);
    lang_search->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_langItemList = new SearchList;
    m_langItemList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_langItemList->setCheckable(true);
    m_langItemList->setFixedWidth(310);
    m_langItemList->setItemSize(290, EXPAND_HEADER_HEIGHT);
    m_langItemList->setEnableVerticalScroll(true);

    DEnhancedWidget *extend_mainWidget = new DEnhancedWidget(m_frame, m_frame);
    connect(extend_mainWidget, &DEnhancedWidget::heightChanged,
            [user_layout_list, lang_list_frame, lang_search, this]{
        m_langItemList->setFixedHeight(m_frame->height() - user_layout_list->geometry().bottom() - 80);
        lang_list_frame->setFixedHeight(m_langItemList->height() + 50);
    });
    DEnhancedWidget *extend_user_layoutList = new DEnhancedWidget(user_layout_list, user_layout_list);
    connect(extend_user_layoutList, &DEnhancedWidget::heightChanged,
            [user_layout_list, lang_list_frame, lang_search, this]{
        m_langItemList->setFixedHeight(m_frame->height() - user_layout_list->geometry().bottom() - 80);
        lang_list_frame->setFixedHeight(m_langItemList->height() + 50);
    });

    lang_frame_layout->addSpacing(10);
    lang_frame_layout->addWidget(lang_search, 0, Qt::AlignTop|Qt::AlignHCenter);
    lang_frame_layout->addWidget(m_langItemList, 50);
    lang_frame_layout->addStretch(1);

    lang_list_frame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    lang_list_frame->setLayout(lang_frame_layout);
    language_expand->setContent(lang_list_frame);

    m_dbusLangSelector = new DbusLangSelector(this);

    lang_list_frame->setMinimumHeight(lang_search->height()+m_langItemList->height());

    connect(lang_search, &DSearchEdit::textChanged, [lang_search, this]{
        m_langItemList->setKeyWord(lang_search->text());
    });
    connect(m_langItemList, &SearchList::checkedItemChanged, [this](int index){
        if(index<0)
            return;

        QString str = m_mapUserLayoutInfo[m_langItemList->getItem(index)->keyWords()[0]];
        if(m_dbusLangSelector->currentLocale() != str){
            m_dbusLangSelector->SetLocale(str);
            m_languageTips->setText(m_mapUserLayoutInfo.key(str));
        }
    });
    connect(m_dbusLangSelector, &DbusLangSelector::CurrentLocaleChanged, [this]{
        QString str = m_dbusLangSelector->currentLocale();
        int index = m_mapUserLayoutIndex[str];
        if(index>=0&&index<m_langItemList->count())
            m_langItemList->setCheckedItem(index);
    });

    m_mainLayout->addWidget(capsLockLine);
    m_mainLayout->addWidget(new DSeparatorHorizontal);
    m_mainLayout->addWidget(keyboardLayoutLine);
    m_mainLayout->addWidget(new DSeparatorHorizontal);
    m_mainLayout->addWidget(user_layout_list);
    m_mainLayout->addWidget(new DSeparatorHorizontal);
    m_mainLayout->addWidget(language_expand);
    m_mainLayout->addStretch(1);

    m_letterClassifyList = new FirstLetterClassify(m_frame);
    m_letterClassifyList->hide();
    m_letterClassifyList->setFixedWidth(310);
    m_mainLayout->insertWidget(10, m_letterClassifyList);
    QTimer::singleShot(400, this, SLOT(loadLater()));
}

void Keyboard::run()
{
    QPointer<Keyboard> self = this;

    QDBusPendingReply<KeyboardLayoutList> list = m_dbusKeyboard->LayoutList();
    list.waitForFinished();
    KeyboardLayoutList tmp_map = list.value();

    QDBusInterface dbus_pinyin( "com.deepin.api.Pinyin", "/com/deepin/api/Pinyin",
                                      "com.deepin.api.Pinyin" );

    foreach (const QString &str, tmp_map.keys()) {
        if(self.isNull())
            return;

        if(m_mapUserLayoutInfo.contains(tmp_map[str]))
            continue;

        QString title = tmp_map[str];
        QChar letterFirst = title[0];
        QStringList letterFirstList;
        if(letterFirst.isLower() || letterFirst.isUpper()){
            letterFirstList << QString(letterFirst);
        }else{
            QDBusMessage message = dbus_pinyin.call("Query", QString(letterFirst));
            letterFirstList = message.arguments()[0].toStringList();
        }

        if(!letterFirstList.isEmpty()){
            emit addLayoutItem(str, title, letterFirstList);
        }
    }
}

void Keyboard::loadLater()
{
    QList<LocaleInfo> langList;
    QByteArray readBytes = m_settings->value("LangList").toByteArray();
    QDataStream readStream(&readBytes, QIODevice::ReadOnly);
    readStream >> langList;
    if (!langList.isEmpty())
    {
        loadLanguageList(langList);
    } else {
        QDBusPendingCallWatcher *lang_list = new QDBusPendingCallWatcher(m_dbusLangSelector->GetLocaleList(), this);
        connect(lang_list, &QDBusPendingCallWatcher::finished, [=] {
            QDBusPendingReply<LocaleList> reply = *lang_list;

            const QList<LocaleInfo> infos = reply.value();
            loadLanguageList(infos);

            // save langList for optimize load speed
            QByteArray writeBytes;
            QDataStream writeStream(&writeBytes, QIODevice::WriteOnly);
            writeStream << infos;

            m_settings->setValue("LangList", writeBytes);
        });
    }

    // load classify
    connect(this, &Keyboard::addLayoutItem, this, &Keyboard::onAddLayoutItem, Qt::QueuedConnection);
    QThreadPool::globalInstance()->start(this);
}

void Keyboard::onAddLayoutItem(const QString &id, const QString &title, const QStringList &letterFirstList)
{
    KeyboardLayoutDelegate *item = new KeyboardLayoutDelegate(title);
    item->setKeyWords(letterFirstList);
    connect(item, &KeyboardLayoutDelegate::checkedChanged, [id, item, this](bool checked){
       if(checked){
           m_dbusKeyboard->AddUserLayout(id);
           m_selectLayoutList << item;
       }else{
           m_dbusKeyboard->DeleteUserLayout(id);
           m_selectLayoutList.removeOne(item);
       }
    });
    m_letterClassifyList->addItem(item, letterFirstList.first().at(0));

    for (int i = 1; i < letterFirstList.count(); ++i) {
        QChar ch = letterFirstList[i][0];
        KeyboardLayoutDelegate *tmp = new KeyboardLayoutDelegate(title);
        tmp->setKeyWords(letterFirstList);
        connect(tmp, &KeyboardLayoutDelegate::checkedChanged, item, &KeyboardLayoutDelegate::setChecked);
        m_letterClassifyList->addItem(tmp, ch);
    }
}

void Keyboard::loadLanguageList(const QList<LocaleInfo> &infos)
{
    const QString theme = DThemeManager::instance()->theme();
    const QString current_lang = m_dbusLangSelector->currentLocale();

    for (const LocaleInfo &info : infos)
    {
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        GenericListItem *item = new GenericListItem;

        item->setKeyWords(QStringList()<<info.name);
        if(theme == "dark"){
            item->setImageNormal(":/lang_images/normal/"+info.id+".png");
        }else{
            item->setImageNormal(":/lang_images/dark/"+info.id+".png");
        }
        item->setImageHover(":/lang_images/hover/"+info.id+".png");
        item->setImagePress(":/lang_images/hover/"+info.id+".png");
        item->setImageChecked(":/lang_images/active/"+info.id+".png");

        m_langItemList->addItem(item);
        if(info.id == current_lang){
            m_langItemList->setCheckedItem(m_langItemList->count()-1);
        }

        m_mapUserLayoutInfo[info.name] = info.id;
        m_mapUserLayoutIndex[info.id] = m_langItemList->count()-1;
    }


    m_langItemList->beginSearch();

//        if (meDeleted && dbusLangSelector && dbusLangSelector->isValid())
        m_languageTips->setText(m_mapUserLayoutInfo.key(current_lang));
        m_settings->setValue("languageTips", m_mapUserLayoutInfo.key(current_lang) );

}

QFrame* Keyboard::getContent()
{
    return m_frame;
}

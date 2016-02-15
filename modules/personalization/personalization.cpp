/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "personalization.h"
#include "dbusworker.h"
#include <libdui/dbuttongrid.h>
#include <libdui/dseparatorhorizontal.h>
#include <libdui/dlabel.h>
#include <libdui/dboxwidget.h>
#include <QVBoxLayout>
#include <QTimer>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QThread>
#include <QFormLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QScreen>

#include "mousearea.h"

DUI_USE_NAMESPACE

Personalization::Personalization():m_margins(0, 5, 0, 5)
{
    Q_UNUSED(QT_TRANSLATE_NOOP("ModuleName", "Personalization"));

    qRegisterMetaType<ImageInfoList>("ImageInfoList");
    qRegisterMetaType<QJsonObject>("QJsonObject");
    qRegisterMetaType<JosnMapObjs>("qRegisterMetaType");
    initUI();
    initControllers();
    emit dataRequested();
}

void Personalization::initUI(){
    m_frame = new QFrame();
    m_expandGroup = new DExpandGroup(this);
    m_headerLine = new DHeaderLine();
    m_headerLine->setStyleSheet("font-size: 16px;color:white");
    m_headerLine->setFixedHeight(50);
    m_headerLine->setTitle(tr("Personalization"));

    DSeparatorHorizontal* horizontalSeparator = new DSeparatorHorizontal();
    initThemeExpand();
    initWindowExpand();
    initIconExpand();
    initCursorExpand();
    initWallPaperExpand();
    initFontExpand();

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->addWidget(m_headerLine);
    mainLayout->addWidget(horizontalSeparator);
    mainLayout->addWidget(m_themeExpand);
    mainLayout->addWidget(m_windowExpand);
    mainLayout->addWidget(m_iconExpand);
    mainLayout->addWidget(m_cursorExpand);
    mainLayout->addWidget(m_wallpaperExpand);
    mainLayout->addWidget(m_fontExpand);
    mainLayout->addStretch(1);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    m_frame->setLayout(mainLayout);
    m_expandGroup->addExpand(m_themeExpand);
    m_expandGroup->addExpand(m_windowExpand);
    m_expandGroup->addExpand(m_iconExpand);
    m_expandGroup->addExpand(m_cursorExpand);
    m_expandGroup->addExpand(m_wallpaperExpand);
    m_expandGroup->addExpand(m_fontExpand);
}


void Personalization::initControllers(){
    m_dbusWorker = new DBusWorker;
    m_dbusWorker->moveToThread(&m_workerThread);
    connect(&m_workerThread, &QThread::finished, m_dbusWorker, &QObject::deleteLater);
    connect(this, &Personalization::dataRequested, m_dbusWorker, &DBusWorker::doWork);
    connect(m_dbusWorker, &DBusWorker::themeKeysChanged, this, &Personalization::updateThemeKeys);
    connect(m_dbusWorker, &DBusWorker::windowKeysChanged, this, &Personalization::updateWindowKeys);
    connect(m_dbusWorker, &DBusWorker::iconKeysChanged, this, &Personalization::updateIconKeys);
    connect(m_dbusWorker, &DBusWorker::cursorKeysChanged, this, &Personalization::updateCursorKeys);
    connect(m_dbusWorker, &DBusWorker::backgroundKeysChanged, this, &Personalization::updateBackgroundKeys);

    connect(m_dbusWorker, &DBusWorker::themeObjsChanged, this, &Personalization::updateThemeObjs);
    connect(m_dbusWorker, &DBusWorker::themeDetailsChanged, this, &Personalization::updateThemeButtons);
    connect(m_dbusWorker, &DBusWorker::windowDetailsChanged, this, &Personalization::updateWindowButtons);
    connect(m_dbusWorker, &DBusWorker::iconDetailsChanged, this, &Personalization::updateIconButtons);
    connect(m_dbusWorker, &DBusWorker::cursorDetailsChanged, this, &Personalization::updateCursorButtons);
    connect(m_dbusWorker, &DBusWorker::backgroundDetailsChanged, this, &Personalization::updateWallpaperButtons);
    connect(m_dbusWorker, &DBusWorker::standardFontDetailsChanged, this, &Personalization::updateStandardFontCombox);
    connect(m_dbusWorker, &DBusWorker::monospaceFontDetailsChanged, this, &Personalization::updateMonospaceFontCombox);

    connect(m_dbusWorker, &DBusWorker::currentThemeChanged, this, &Personalization::updateCurrentTheme);
    connect(m_dbusWorker, &DBusWorker::currentThemeHightlighted, this, &Personalization::highlightCurrentTheme);
    connect(m_dbusWorker, &DBusWorker::fontSizeChanged, this, &Personalization::setFontLabel);
    connect(m_dbusWorker, &DBusWorker::dataFinished, this, &Personalization::handleDataFinished);
    m_workerThread.start();
}

void Personalization::initConnect(){
    connect(m_slider, &DSlider::valueChanged, this, &Personalization::setFontSize);
    connect(m_themeButtonGrid, &DButtonGrid::buttonCheckedIndexChanged, this, &Personalization::setThemeByIndex);
    connect(m_themeButtonGrid, SIGNAL(requestRefreshed(QString)), this, SLOT(handleDataDeleteRefrehed(QString)));
    connect(m_windowButtonGrid, &DButtonGrid::buttonCheckedIndexChanged, this, &Personalization::setWindowByIndex);
    connect(m_iconButtonGrid, &DButtonGrid::buttonCheckedIndexChanged, this, &Personalization::setIconByIndex);
    connect(m_cursorButtonGrid, &DButtonGrid::buttonCheckedIndexChanged, this, &Personalization::setCursorByIndex);
    connect(m_wallpaperButtonGrid, &DButtonGrid::buttonCheckedIndexChanged, this, &Personalization::setBackgroundByIndex);
    connect(m_wallpaperButtonGrid, SIGNAL(requestRefreshed(QString)), this, SLOT(handleDataDeleteRefrehed(QString)));
    connect(m_standardFontCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(setStandardFontByIndex(int)));
    connect(m_monospaceFontCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(setMonospaceFontByIndex(int)));

}

void Personalization::initThemeExpand(){
    m_themeExpand = new DArrowLineExpand(m_frame);
    m_themeExpand->setTitle(tr("Theme"));
    m_themeButtonGrid = new DButtonGrid(1, 2);
    m_themeButtonGrid->setItemSize(m_itemWidth, m_itemHeight + 20);

    m_themeContentFrame = new QFrame;
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_themeButtonGrid, 0, Qt::AlignCenter);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(m_margins);
    m_themeContentFrame->setLayout(mainLayout);

    m_buttonGrids.append(m_themeButtonGrid);
    m_contentFrames.append(m_themeContentFrame);
}

DArrowLineExpand* Personalization::getThemeExpand(){
    return m_themeExpand;
}

void Personalization::initWindowExpand(){
    m_windowExpand = new DArrowLineExpand(m_frame);
    m_windowExpand->setTitle(tr("Window"));

    m_windowButtonGrid = new DButtonGrid(1, 2);
    m_windowButtonGrid->setItemSize(m_itemWidth, m_itemHeight + 20);

    m_windowContentFrame = new QFrame;

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_windowButtonGrid, 0, Qt::AlignCenter);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(m_margins);
    m_windowContentFrame->setLayout(mainLayout);

    m_buttonGrids.append(m_windowButtonGrid);
    m_contentFrames.append(m_windowContentFrame);

}

DArrowLineExpand* Personalization::getWindowExpand(){
    return m_windowExpand;
}

void Personalization::initIconExpand(){
    m_iconExpand = new DArrowLineExpand(m_frame);
    m_iconExpand->setTitle(tr("Icon"));

    m_iconButtonGrid = new DButtonGrid(1, 2);
    m_iconButtonGrid->setItemSize(m_itemWidth, m_itemHeight + 20);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_iconButtonGrid, 0, Qt::AlignCenter);
    mainLayout->addStretch();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(m_margins);

    DVBoxWidget *widget = new DVBoxWidget;
    widget->layout()->addLayout(mainLayout);
    widget->setFixedHeight(m_itemHeight + 20);

    m_iconContentArea = new QScrollArea;
    m_iconContentArea->setStyleSheet("background-color:transparent;");
    m_iconContentArea->setFrameStyle(QFrame::NoFrame);
    m_iconContentArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_iconContentArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_iconContentArea->setWidget(widget);

    m_buttonGrids.append(m_iconButtonGrid);
    m_contentFrames.append(m_iconContentArea);

}

DArrowLineExpand* Personalization::getIconExpand(){
    return m_iconExpand;
}

void Personalization::initCursorExpand(){
    m_cursorExpand = new DArrowLineExpand(m_frame);
    m_cursorExpand->setTitle(tr("Cursor"));

    m_cursorButtonGrid = new DButtonGrid(1, 2);
    m_cursorButtonGrid->setItemSize(m_itemWidth, m_itemHeight + 20);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_cursorButtonGrid, 0, Qt::AlignCenter);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(m_margins);

    DVBoxWidget *widget = new DVBoxWidget;
    widget->layout()->addLayout(mainLayout);
    widget->setFixedHeight(m_itemHeight + 20);

    m_cursorContentArea = new QScrollArea;
    m_cursorContentArea->setStyleSheet("background-color:transparent;");
    m_cursorContentArea->setFrameStyle(QFrame::NoFrame);
    m_cursorContentArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_cursorContentArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_cursorContentArea->setWidget(widget);

    m_buttonGrids.append(m_cursorButtonGrid);
    m_contentFrames.append(m_cursorContentArea);
}

DArrowLineExpand* Personalization::getCursorExpand(){
    return m_cursorExpand;
}

void Personalization::initWallPaperExpand(){
    m_wallpaperExpand = new DArrowLineExpand(m_frame);
    m_wallpaperExpand->setTitle(tr("Wallpaper"));

    m_wallpaperButtonGrid = new DButtonGrid(1, 2);
    m_wallpaperButtonGrid->setItemSize(m_itemWidth, m_itemHeight);

    m_wallpaperContentFrame = new QFrame;
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_wallpaperButtonGrid, 0, Qt::AlignCenter);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(m_margins);
    m_wallpaperContentFrame->setLayout(mainLayout);

    m_buttonGrids.append(m_wallpaperButtonGrid);
    m_contentFrames.append(m_wallpaperContentFrame);
}

DArrowLineExpand* Personalization::getWallPaperExpand(){
    return m_cursorExpand;
}


void Personalization::initFontExpand(){
    m_fontExpand = new DArrowLineExpand(m_frame);
    m_fontExpand->setTitle(tr("Fonts"));

    DLabel* standartLabel = new DLabel(tr("Standard"));
    DLabel* monospaceLabel = new DLabel(tr("Monospaced"));
    DLabel* sizeLabel = new DLabel(tr("Size"));
    m_standardFontCombox = new DFontComboBox;
    m_standardFontCombox->setFixedSize(200, BUTTON_HEIGHT);
    m_monospaceFontCombox = new DFontComboBox;
    m_monospaceFontCombox->setFixedSize(200, BUTTON_HEIGHT);

    QFrame* silidFrame = new QFrame;

    QHBoxLayout* sliderLayout = new QHBoxLayout;
    m_slider = new DSlider(Qt::Horizontal);
    m_slider->setRange(9, 16);
    m_slider->setHandleType(DSlider::SharpHandler);
    m_slider->setFixedHeight(BUTTON_HEIGHT);
    m_slider->setPageStep(1);

    QString tip = tr("Font") + QString::number(m_slider->value());
    m_fontTipLabel = new DLabel(tip);
    m_fontTipLabel->setFixedWidth(50);
    sliderLayout->addWidget(m_slider);
    sliderLayout->addWidget(m_fontTipLabel);
    sliderLayout->setContentsMargins(0, 0, 0, 0);
    silidFrame->setLayout(sliderLayout);


    QFormLayout* fontLayout = new QFormLayout;
    fontLayout->addRow(standartLabel, m_standardFontCombox);
    fontLayout->addRow(monospaceLabel,m_monospaceFontCombox);
    fontLayout->addRow(sizeLabel, silidFrame);
    fontLayout->setSpacing(15);
    fontLayout->setContentsMargins(10, 10, 10, 10);
    fontLayout->setLabelAlignment(Qt::AlignHCenter | Qt::AlignRight);

    m_fontContentFrame = new QFrame;
    //QVBoxLayout *mainLayout = new QVBoxLayout;
    //mainLayout->addLayout(fontLayout);
    //mainLayout->setSpacing(0);
    //mainLayout->setContentsMargins(m_margins);
    m_fontContentFrame->setLayout(fontLayout);
    m_contentFrames.append(m_fontContentFrame);

    m_fontContentFrame->setFixedHeight(110);
    m_fontExpand->setContent(m_fontContentFrame);

    m_themeExpand->setExpand(true);
}

DArrowLineExpand* Personalization::getFontExpand(){
    return m_fontExpand;
}

void Personalization::updateThemeKeys(const QStringList &themeKeys){
    m_themeKeys = themeKeys;
}

void Personalization::updateWindowKeys(const QStringList &windowKeys){
    m_windowKeys = windowKeys;
}

void Personalization::updateIconKeys(const QStringList &iconKeys){
    m_iconKeys = iconKeys;
}

void Personalization::updateCursorKeys(const QStringList &cursorKeys){
    m_cursorKeys = cursorKeys;
}

void Personalization::updateBackgroundKeys(const QStringList &backgroundKeys){
    m_backgroundKeys = backgroundKeys;
}


void Personalization::updateThemeObjs(const JosnMapObjs &themeObjs){
    m_themeObjs = themeObjs;
}

void Personalization::updateThemeButtons(const ImageInfoList &imageInfos){
    m_themeImageInfos = imageInfos;
    m_themeButtonGrid->clear();
    m_themeButtonGrid->addImageButtons(imageInfos);

    int w = m_themeButtonGrid->width() + m_margins.left() + m_margins.right();
    int h = m_themeButtonGrid->height() + m_margins.top() + m_margins.bottom();

    m_themeContentFrame->setFixedSize(w, h);
    m_themeExpand->setContent(m_themeContentFrame);
}

void Personalization::updateWindowButtons(const ImageInfoList &imageInfos){
    m_windowImageInfos = imageInfos;
    m_windowButtonGrid->addImageButtons(imageInfos);

    int w = m_windowButtonGrid->width() + m_margins.left() + m_margins.right();
    int h = m_windowButtonGrid->height() + m_margins.top() + m_margins.bottom();
    m_windowContentFrame->setFixedSize(w, h);
    m_windowExpand->setContent(m_windowContentFrame);
}

void Personalization::updateIconButtons(const ImageInfoList &imageInfos){
    m_iconImageInfos = imageInfos;
    m_iconButtonGrid->addImageButtons(imageInfos);

    int w = m_iconButtonGrid->width() + m_margins.left() + m_margins.right();
    int h = m_iconButtonGrid->height() + m_margins.top() + m_margins.bottom();

    const int maxH = qApp->desktop()->height()
                     - m_headerLine->height()     // header
                     - m_iconExpand->height() * 6 // expand
                     - 2;                         // separator
    if (h > maxH)
        h = maxH;

    m_iconContentArea->setFixedSize(w, h);
    m_iconExpand->setContent(m_iconContentArea);
}

void Personalization::updateCursorButtons(const ImageInfoList &imageInfos){
    m_cursorImageInfos = imageInfos;
    m_cursorButtonGrid->addImageButtons(imageInfos);

    int w = m_cursorButtonGrid->width() + m_margins.left() + m_margins.right();
    int h = m_cursorButtonGrid->height() + m_margins.top() + m_margins.bottom();

    const int maxH = qApp->desktop()->height()
                     - m_headerLine->height()       // header
                     - m_cursorExpand->height() * 6 // expand
                     - 2;                           // separator
    if (h > maxH)
        h = maxH;

    m_cursorContentArea->setFixedSize(w, h);
    m_cursorExpand->setContent(m_cursorContentArea);
}

void Personalization::updateWallpaperButtons(const ImageInfoList &imageInfos){
    m_wallpaperImageInfos = imageInfos;
    m_wallpaperButtonGrid->clear();
    m_wallpaperButtonGrid->addImageButtons(imageInfos, false);
    int w = m_wallpaperButtonGrid->width() + m_margins.left() + m_margins.right();
    int h = m_wallpaperButtonGrid->height() + m_margins.top() + m_margins.bottom();
    m_maxExpandContentHeight = m_frame->height() \
            - m_headerLine->height()\
            - 2\
            - 32 * 6;
    if (h > m_maxExpandContentHeight){
        m_wallpaperButtonGrid->setFixedHeight(m_maxExpandContentHeight - m_margins.top() - m_margins.bottom());
        m_wallpaperContentFrame->setFixedSize(w, m_maxExpandContentHeight);
    }else{
        m_wallpaperContentFrame->setFixedSize(w, h);
    }

    m_wallpaperExpand->setContent(m_wallpaperContentFrame);
}

void Personalization::updateStandardFontCombox(const QStringList &standardFonts){
    m_standardFonts.clear();
    m_standardFontCombox->clear();
    m_standardFonts = standardFonts;
    foreach (QString family, standardFonts) {
        m_standardFontCombox->addFontItem(family);
    }
}


void Personalization::updateMonospaceFontCombox(const QStringList &monospaceFonts){
    m_monospaceFonts.clear();
    m_monospaceFontCombox->clear();
    m_monospaceFonts = monospaceFonts;
    foreach (QString family, monospaceFonts) {
        m_monospaceFontCombox->addFontItem(family);
    }
}

void Personalization::handleDataFinished(){
    foreach (DBaseExpand* expand, m_expandGroup->expands()) {
        if (expand != m_themeExpand)
            expand->setExpand(false);
    }

    int space = qApp->desktop()->screenGeometry().height() - m_headerLine->height();
    for (int i=0; i< m_expandGroup->expands().count(); ++i) {
        space -= EXPAND_HEADER_HEIGHT + 2;
    }

    foreach (DButtonGrid* buttonGrid, m_buttonGrids) {
        int index =  m_buttonGrids.indexOf(buttonGrid);
        if (m_contentFrames.length() > index && m_expandGroup->expands().length() > index){
            QFrame* contentFrame = m_contentFrames.at(index);
//            DBaseExpand* expand = m_expandGroup->expands().at(index);
            if (contentFrame->height() >= space){
                buttonGrid->setFixedHeight(space);
                contentFrame->setFixedHeight(space + m_margins.top() + m_margins.bottom());
//                expand->updateContentHeight();
            }
        }
    }
    foreach (DBaseExpand* expand, m_expandGroup->expands()) {
        if (expand != m_themeExpand)
            expand->setExpand(false);
    }
    initConnect();
}

int Personalization::getValidKeyIndex(const ImageInfoList &infoList, const QString &key) const{
    for(int i=0; i< infoList.count(); i++){
        if (key == infoList.at(i).value("key")){
            return i;
        }
    }
    return -1;
}

void Personalization::updateCurrentTheme(QString themeKey){
    m_currentTheme = themeKey;
    if (m_themeObjs.contains(themeKey)){
       const QJsonObject& obj =  m_themeObjs.value(themeKey);
       if (m_themeKeys.contains(themeKey)){
            int index = getValidKeyIndex(m_themeImageInfos, themeKey);
            if (index >= 0){
                m_themeButtonGrid->checkButtonByIndex(index);
            }else{
                qCritical() << "There is no theme named:" << themeKey;
            }
       }
       QString gtkKey("Gtk");
       if (obj.contains(gtkKey)){
            QString id = obj.value(gtkKey).toObject().value("Id").toString();
            int index = getValidKeyIndex(m_windowImageInfos, id);
            if (index >= 0){
                m_windowButtonGrid->checkButtonByIndex(index);
            }else{
                qCritical() << "There is no window named:" << gtkKey;
            }
       }

       QString iconKey("Icon");
       if (obj.contains(iconKey)){
            QString id = obj.value(iconKey).toObject().value("Id").toString();
            int index = getValidKeyIndex(m_iconImageInfos, id);
            if (index >= 0){
                m_iconButtonGrid->checkButtonByIndex(index);
            }else{
                qCritical() << "There is no icon named:" << id;
            }
       }

       QString cursorKey("Cursor");
       if (obj.contains(cursorKey)){
            QString id = obj.value(cursorKey).toObject().value("Id").toString();
            int index = getValidKeyIndex(m_cursorImageInfos, id);
            if(index >= 0){
                m_cursorButtonGrid->checkButtonByIndex(index);
            }else{
                qCritical() << "There is no cursor named:" << id;
            }
       }

       QString backgroundKey("Background");
       if (obj.contains(backgroundKey)){
           QString URI = obj.value(backgroundKey).toObject().value("Id").toString();
           int index = getValidKeyIndex(m_wallpaperImageInfos, URI);
           if (index >= 0){
                m_wallpaperButtonGrid->checkButtonByIndex(index);
           }else{
                qCritical() << "There is no background named:" << URI;
           }
       }

       QString standardFont = obj.value("StandardFont").toObject().value("Id").toString();
       int sIndex = m_standardFonts.indexOf(standardFont);

       if(sIndex >= 0) {
           ///因为DFontComboBox的bug，初始化setCurrentIndex为0时会导致标题显示为空，必须手动先设置为其他值，再设为0方可解决
           m_standardFontCombox->setCurrentIndex((sIndex + 1) % m_standardFontCombox->count());
           m_standardFontCombox->setCurrentIndex(sIndex);
       } else {
           m_standardFonts.append(standardFont);
           m_standardFontCombox->addFontItem(standardFont);
           m_standardFontCombox->setCurrentIndex(m_standardFontCombox->count() - 1);
       }

       QString monospaceFont = obj.value("MonospaceFont").toObject().value("Id").toString();
       int mIndex = m_monospaceFonts.indexOf(monospaceFont);

       if(mIndex >= 0) {
           ///因为DFontComboBox的bug，初始化setCurrentIndex为0时会导致标题显示为空，必须手动先设置为其他值，再设为0方可解决
           m_monospaceFontCombox->setCurrentIndex((mIndex + 1) % m_monospaceFontCombox->count());
           m_monospaceFontCombox->setCurrentIndex(mIndex);
       } else {
           m_monospaceFonts.append(monospaceFont);
           m_monospaceFontCombox->addFontItem(monospaceFont);
           m_monospaceFontCombox->setCurrentIndex(m_monospaceFontCombox->count() - 1);
       }
    }
}

void Personalization::highlightCurrentTheme(QString themeKey)
{
    m_currentTheme = themeKey;
    if (m_themeObjs.contains(themeKey)){
//       const QJsonObject& obj =  m_themeObjs.value(themeKey);
       if (m_themeKeys.contains(themeKey)){
            int index = getValidKeyIndex(m_themeImageInfos, themeKey);
            if (index >= 0){
                m_themeButtonGrid->checkButtonByIndex(index);
            }else{
                qCritical() << "There is no theme named:" << themeKey;
            }
       }
    }
}

void Personalization::setThemeByIndex(int index){
    if (m_themeKeys.length() > index){
        QString key = m_themeImageInfos.at(index).value("key");
        if(m_dbusWorker->getCurrentTheme() != key)
            m_dbusWorker->setTheme(m_dbusWorker->staticTypeKeys.value("TypeDTheme"), key);
    }else{
        qCritical() << "set theme Error" <<  m_themeKeys << index;
    }
}

void Personalization::setWindowByIndex(int index){
    if (m_windowKeys.length() > index){
        QString key = m_windowImageInfos.at(index).value("key");
        m_dbusWorker->setTheme(m_dbusWorker->staticTypeKeys.value("TypeGtkTheme"), key);
    }else{
        qCritical() << "set window Error" <<  m_windowKeys << index;
    }
}

void Personalization::setIconByIndex(int index){
    if (m_iconKeys.length() > index){
        QString key = m_iconImageInfos.at(index).value("key");
        m_dbusWorker->setTheme(m_dbusWorker->staticTypeKeys.value("TypeIconTheme"), key);
    }else{
        qCritical() << "set icon Error" <<  m_iconKeys << index;
    }
}

void Personalization::setCursorByIndex(int index){
    if (m_cursorKeys.length() > index){
        QString key = m_cursorImageInfos.at(index).value("key");
        m_dbusWorker->setTheme(m_dbusWorker->staticTypeKeys.value("TypeCursorTheme"), key);
    }else{
        qCritical() << "set cursor Error" <<  m_cursorKeys << index;
    }
}

void Personalization::setBackgroundByIndex(int index){
    if (m_backgroundKeys.length() > index){
        QString key = m_wallpaperImageInfos.at(index).value("key");
        m_dbusWorker->setTheme(m_dbusWorker->staticTypeKeys.value("TypeBackground"), key);
    }else{
        qCritical() << "set background Error" <<  m_backgroundKeys << index;
    }
}

void Personalization::setStandardFontByIndex(int index){
    if (m_standardFonts.length() > index){
        QString key = m_standardFonts.at(index);
        m_dbusWorker->setTheme(m_dbusWorker->staticTypeKeys.value("TypeStandardFont"), key);
    }else{
        qCritical() << "set standard Error" <<  m_standardFonts << index;
    }
}

void Personalization::setMonospaceFontByIndex(int index){
    if (m_monospaceFonts.length() > index){
        QString key = m_monospaceFonts.at(index);
        m_dbusWorker->setTheme(m_dbusWorker->staticTypeKeys.value("TypeMonospaceFont"), key);
    }else{
        qCritical() << "set monospace Error" <<  m_monospaceFonts << index;
    }
}

void Personalization::setFontSize(int fontSize){
    m_dbusWorker->setTheme(m_dbusWorker->staticTypeKeys.value("TypeFontSize"), QString::number(fontSize));
    setFontLabel(fontSize);
}

void Personalization::setFontLabel(int fontSize){
    QString style = m_fontTipLabel->styleSheet();
    QString fontsizeStyle = QString("font-size:%1px;").arg(QString::number(fontSize));
    QRegExp rx("font-size\\s*:\\s*\\d+px\\s*;");
    style.replace(rx, fontsizeStyle);
    QString tip = tr("Font") + QString::number(fontSize);
    m_fontTipLabel->setText(tip);
    m_fontTipLabel->setStyleSheet(style);
    m_slider->setValue(fontSize);
}

QFrame* Personalization::getContent()
{
    return m_frame;
}

void Personalization::handleDataDeleteRefrehed(QString id){
    qDebug() << id;
    if (sender() == m_wallpaperButtonGrid){
        QString type = m_dbusWorker->staticTypeKeys.value("TypeBackground");
        m_dbusWorker->deleteItem(type, id);
    }else if (sender() == m_themeButtonGrid){
        QString type = m_dbusWorker->staticTypeKeys.value("TypeDTheme");
        qDebug() << type << id;
        m_dbusWorker->deleteItem(type, id);
    }
}


Personalization::~Personalization()
{
    qDebug() << "~Personalization()";
    m_workerThread.quit();
    m_workerThread.wait();
    m_frame->deleteLater();
}

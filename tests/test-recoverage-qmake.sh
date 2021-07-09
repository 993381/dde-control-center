#!/bin/bash
BUILD_DIR=build
REPORT_DIR=report
cd ../
rm -rf $BUILD_DIR
mkdir $BUILD_DIR
cd $BUILD_DIR
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j16
make check

cd tests/

cd dccwidgets/
./dccwidgets-unittest --gtest_output=xml:dde_test_report_dccwidgets.xml
# dccwidgets
mv asan.log* asan_dccwidgets.log

cd ..
cd dde-control-center/
./mouse-unittest --gtest_output=xml:dde_test_report_mouse.xml
./systeminfo-unittest --gtest_output=xml:dde_test_report_systeminfo.xml
#./bluetooth-unittest --gtest_output=xml:dde_test.xml
#./datetime-unittest --gtest_output=xml:dde_test.xml
./defapp-unittest --gtest_output=xml:dde_test_report_defapp.xml
./notification-unittest --gtest_output=xml:dde_test_report_notification.xml
./keyboard-unittest --gtest_output=xml:dde_test_report_keyboard.xml

#dde-control-center
#bluetooth和datetime未运行单元测试程序，暂时屏蔽
#mv asan_bluetooth.log* asan_bluetooth.log
mv asan_mouse.log* asan_mouse.log
mv asan_systeminfo.log* asan_systeminfo.log
mv asan_defapp.log* asan_defapp.log
#mv asan_datetime.log* .asan_datetime.log
mv asan_notification.log* asan_notification.log
mv asan_keyboard.log* asan_keyboard.log
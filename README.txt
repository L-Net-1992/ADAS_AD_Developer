1:修改node模块分类:aiccstackedwidget.hpp
2:修改NodeEditor样式:aiccstackedwidget.hpp
3:查看可执行程序引用哪些库文件:readelf -a AutoPlatform | grep -i runpath
4:win10下Qt发生错误
:-1: error: LNK1104: 无法打开文件“libboost_container-vc142-mt-x64-1_76.lib”
:-1: error: LNK1104: 无法打开文件“libboost_json-vc142-mt-x64-1_76.lib”

在Qt Creator中Projects->Build中Batch Edit...按钮->在窗口中写入
-DBoost_JSON_LIBRARY_RELEASE:FILEPATH=D:/Boost/boost_1_76_0/lib64-msvc-14.2/libboost_json-vc142-mt-x64-1_76.lib
-DBoost_CONTAINER_LIBRARY_RELEASE:FILEPATH=D:/Boost/boost_1_76_0/lib64-msvc-14.2/libboost_container-vc142-mt-x64-1_76.lib

FlowScene增加了以下3句才能重置线的正确位置
connection->connectionGeometry().setSelected(true);
nodeOut.resetReactionToConnection();
connection->connectionGeometry().setSelected(false);

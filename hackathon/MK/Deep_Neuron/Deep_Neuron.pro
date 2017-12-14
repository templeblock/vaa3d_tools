TEMPLATE    = lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
CAFFEPATH = C:/caffe
CUDAPATH  = "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v8.0"
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH += $$VAA3DPATH/common_lib/include
INCLUDEPATH	+= $$CAFFEPATH/include
INCLUDEPATH	+= $$CAFFEPATH/src/caffe
INCLUDEPATH	+= $$CUDAPATH/include/
INCLUDEPATH += ../../zhi/deep_learning/prediction
INCLUDEPATH += ../../../released_plugins/v3d_plugins/terastitcher/include
INCLUDEPATH += ../../../released_plugins/v3d_plugins/mean_shift_center

win32 {
    LIBS += -L$$CAFFEPATH\\scripts\\build\\lib\\Release
    LIBS += -lcaffe

    # cuda
    INCLUDEPATH += $$CUDAPATH\\include
    LIBS += -L$$CUDAPATH\\lib\\x64
    LIBS += -lcudart -lcublas -lcurand

    # opencv
    INCLUDEPATH += C:\\Users\\hsienchik\\.caffe\\dependencies\\libraries_v120_x64_py27_1.1.0\\libraries\\include\\opencv
    LIBS += -LC:\\Users\\hsienchik\\.caffe\\dependencies\\libraries_v120_x64_py27_1.1.0\\libraries\\x64\\vc12\\lib
    LIBS += -lopencv_core310 -lopencv_imgproc310 -lopencv_highgui310

    # other dependencies
    INCLUDEPATH += C:\\Users\\hsienchik\\.caffe\\dependencies\\libraries_v120_x64_py27_1.1.0\\libraries\\include
    INCLUDEPATH += C:\\Users\\hsienchik\\.caffe\\dependencies\\libraries_v120_x64_py27_1.1.0\\libraries\\include\\boost-1_61
    INCLUDEPATH += $$CAFFEPATH\\scripts\\build
    INCLUDEPATH += $$CAFFEPATH\\scripts\\build\\include
    LIBS += -L$$CAFFEPATH\\scripts\\build\\lib\\Release
    LIBS += -LC:\\Users\\hsienchik\\.caffe\\dependencies\\libraries_v120_x64_py27_1.1.0\\libraries\\lib
    LIBS += -lglog -lgflags -lprotobuf -llmdb -lleveldb -lboost_system-vc120-mt-1_61 -lboost_thread-vc120-mt-1_61
}

unix {
    LIBS += -L/local2/zhi/caffe/build/lib
    LIBS += -lcaffe

    # cuda
    INCLUDEPATH += /usr/local/cuda/include
    LIBS += -L/usr/local/cuda/lib64
    LIBS += -lcudart -lcublas -lcurand
    
    # opencv
    INCLUDEPATH += /local2/zhi/opencv-2.4.12/include/opencv
    LIBS += -L/local2/zhi/opencv-2.4.12/release/lib
    LIBS += -lopencv_core -lopencv_imgproc -lopencv_highgui

    # other dependencies
    INCLUDEPATH += /local2/zhi/boost_1_63_0
    LIBS += -L/local2/zhi/boost_1_63_0/stage/lib
    LIBS += -L/usr/lib64/atlas
    LIBS += -L/local2/cuda/cuda-8.0/lib64
    LIBS += -L/local2/zhi/caffe/.build_release/lib
    LIBS += -L/usr/local/lib
    LIBS += -lglog -lgflags -lprotobuf -lboost_system -lboost_thread -llmdb -lleveldb -lstdc++ -lcblas -latlas
}


macx{
    LIBS += -L$$VAA3DPATH/common_lib/lib_mac64 -lv3dtiff
    LIBS += -L$$VAA3DPATH/jba/c++ -lv3dnewmat
}

unix:!macx {
    QMAKE_CXXFLAGS += -fopenmp
    LIBS += -fopenmp
    LIBS += -L$$VAA3DPATH/jba/c++ -lv3dnewmat

}


HEADERS	+= Deep_Neuron_plugin.h 
HEADERS += ../../zhi/deep_learning/prediction/classification.h
HEADERS += ../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h
HEADERS += $$VAA3DPATH/basic_c_fun/basic_surf_objs.h
HEADERS += building_tester.h

SOURCES	+= Deep_Neuron_plugin.cpp 
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += ../../zhi/deep_learning/prediction/classification.cpp
SOURCES += ../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES += building_tester.cpp

TARGET	= $$qtLibraryTarget(Deep_Neuron)
DESTDIR	= ../../../../v3d_external/bin/plugins/Deep_Neuron/

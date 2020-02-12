TEMPLATE = subdirs

SUBDIRS += \
    FaceDetector \
    FacemarkLBF \
    InceptionV3 \
    MaskRCNN \
    MobileNetSSD \
    TextDetectorEAST \
    YoloV3


create.commands = $(MKDIR) $$PWD/Build/code_analysis
QMAKE_EXTRA_TARGETS += create

equals(TEMPLATE, subdirs): prepareRecursiveTarget(vera++)
QMAKE_EXTRA_TARGETS += vera++

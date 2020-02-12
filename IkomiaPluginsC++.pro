TEMPLATE = subdirs

SUBDIRS += \
    FaceDetector \
    FacemarkLBF \
    InceptionV3 \
    MaskRCNN \
    MobileNetSSD \
    TextDetectorEAST


create.commands = $(MKDIR) $$PWD/Build/code_analysis
QMAKE_EXTRA_TARGETS += create

equals(TEMPLATE, subdirs): prepareRecursiveTarget(vera++)
QMAKE_EXTRA_TARGETS += vera++

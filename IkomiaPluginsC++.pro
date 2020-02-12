TEMPLATE = subdirs

SUBDIRS += \
    FaceDetector


create.commands = $(MKDIR) $$PWD/Build/code_analysis
QMAKE_EXTRA_TARGETS += create

equals(TEMPLATE, subdirs): prepareRecursiveTarget(vera++)
QMAKE_EXTRA_TARGETS += vera++

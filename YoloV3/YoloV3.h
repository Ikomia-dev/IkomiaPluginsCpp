#ifndef YOLOV3_H
#define YOLOV3_H

#include "YoloV3Global.h"
#include "Utils.hpp"
#include "Process/OpenCV/dnn/COcvDnnProcess.h"
#include "Widget/OpenCV/dnn/COcvWidgetDnnCore.h"
#include "CPluginProcessInterface.hpp"

//------------------------//
//----- CYoloV3Param -----//
//------------------------//
class YOLOV3SHARED_EXPORT CYoloV3Param: public COcvDnnProcessParam
{
    public:

        CYoloV3Param() : COcvDnnProcessParam()
        {
            m_framework = Framework::DARKNET;
        }

        void        setParamMap(const UMapString& paramMap) override
        {
            COcvDnnProcessParam::setParamMap(paramMap);
            m_confidence = std::stod(paramMap.at("confidence"));
            m_nmsThreshold = std::stod(paramMap.at("nmsThreshold"));
        }

        UMapString  getParamMap() const override
        {
            auto paramMap = COcvDnnProcessParam::getParamMap();
            paramMap.insert(std::make_pair("confidence", std::to_string(m_confidence)));
            paramMap.insert(std::make_pair("nmsThreshold", std::to_string(m_nmsThreshold)));
            return paramMap;
        }

    public:

        double m_confidence = 0.5;
        double m_nmsThreshold = 0.4;
};

//-------------------//
//----- CYoloV3 -----//
//-------------------//
class YOLOV3SHARED_EXPORT CYoloV3: public COcvDnnProcess
{
    public:

        CYoloV3();
        CYoloV3(const std::string& name, const std::shared_ptr<CYoloV3Param>& pParam);

        size_t      getProgressSteps() const override;
        int         getNetworkInputSize() const override;
        double      getNetworkInputScaleFactor() const override;
        cv::Scalar  getNetworkInputMean() const override;

        void        run() override;

    private:

        void        manageOutput(cv::Mat &dnnOutput);
};

//--------------------------//
//----- CYoloV3Factory -----//
//--------------------------//
class YOLOV3SHARED_EXPORT CYoloV3Factory : public CProcessFactory
{
    public:

        CYoloV3Factory()
        {
            m_info.m_name = QObject::tr("YoloV3").toStdString();
            m_info.m_shortDescription = QObject::tr("Object detection using YOLO V3 neural network").toStdString();
            m_info.m_description = QObject::tr("We present some updates to YOLO! We made a bunch of little design changes to make it better. "
                                               "We also trained this new network that's pretty swell. It's a little bigger than last time but more accurate. "
                                               "It's still fast though, don't worry. At 320x320 YOLOv3 runs in 22 ms at 28.2 mAP, as accurate as SSD but three times faster. "
                                               "When we look at the old .5 IOU mAP detection metric YOLOv3 is quite good. It achieves 57.9 mAP@50 in 51 ms on a Titan X, "
                                               "compared to 57.5 mAP@50 in 198 ms by RetinaNet, similar performance but 3.8x faster.").toStdString();
            m_info.m_path = QObject::tr("Plugins/C++/Object/Detection").toStdString();
            m_info.m_version = "0.2.0";
            m_info.m_iconPath = "Icon/icon.png";
            m_info.m_authors = "Joseph Redmon, Ali Farhadi";
            m_info.m_article = "YOLOv3: An Incremental Improvement";
            m_info.m_year = 2018;
            m_info.m_license = "YOLO License (public)";
            m_info.m_repo = "https://github.com/pjreddie/darknet";
            m_info.m_keywords = "deep,learning,detection,yolo,darknet";
        }

        virtual ProtocolTaskPtr create(const ProtocolTaskParamPtr& pParam) override
        {
            auto paramPtr = std::dynamic_pointer_cast<CYoloV3Param>(pParam);
            if(paramPtr != nullptr)
                return std::make_shared<CYoloV3>(m_info.m_name, paramPtr);
            else
                return create();
        }
        virtual ProtocolTaskPtr create() override
        {
            auto paramPtr = std::make_shared<CYoloV3Param>();
            assert(paramPtr != nullptr);
            return std::make_shared<CYoloV3>(m_info.m_name, paramPtr);
        }
};

//-------------------------//
//----- CYoloV3Widget -----//
//-------------------------//
class YOLOV3SHARED_EXPORT CYoloV3Widget: public COcvWidgetDnnCore
{
    public:

        CYoloV3Widget(QWidget *parent = Q_NULLPTR): COcvWidgetDnnCore(parent)
        {
            init();
        }
        CYoloV3Widget(ProtocolTaskParamPtr pParam, QWidget *parent = Q_NULLPTR): COcvWidgetDnnCore(pParam, parent)
        {
            m_pParam = std::dynamic_pointer_cast<CYoloV3Param>(pParam);
            init();
        }

    private:

        void init() override
        {
            if(m_pParam == nullptr)
                m_pParam = std::make_shared<CYoloV3Param>();

            auto pParam = std::dynamic_pointer_cast<CYoloV3Param>(m_pParam);
            assert(pParam);

            auto pSpinConfidence = addDoubleSpin(tr("Confidence"), pParam->m_confidence, 0.0, 1.0, 0.1, 2);
            auto pSpinNmsThreshold = addDoubleSpin(tr("NMS threshold"), pParam->m_nmsThreshold, 0.0, 1.0, 0.1, 2);

            //Connections
            connect(pSpinConfidence, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](double val)
            {
                auto pParam = std::dynamic_pointer_cast<CYoloV3Param>(m_pParam);
                assert(pParam);
                pParam->m_confidence = val;
            });
            connect(pSpinNmsThreshold, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](double val)
            {
                auto pParam = std::dynamic_pointer_cast<CYoloV3Param>(m_pParam);
                assert(pParam);
                pParam->m_nmsThreshold = val;
            });
            connect(m_pApplyBtn, &QPushButton::clicked, [&]
            {
                emit doApplyProcess(m_pParam);
            });
        }
};

//--------------------------------//
//----- CYoloV3WidgetFactory -----//
//--------------------------------//
class YOLOV3SHARED_EXPORT CYoloV3WidgetFactory : public CWidgetFactory
{
    public:

        CYoloV3WidgetFactory()
        {
            m_name = QObject::tr("YoloV3").toStdString();
        }

        virtual ProtocolTaskWidgetPtr   create(ProtocolTaskParamPtr pParam)
        {
            return std::make_shared<CYoloV3Widget>(pParam);
        }
};

//-----------------------------------//
//----- Global plugin interface -----//
//-----------------------------------//
class YOLOV3SHARED_EXPORT CYoloV3Interface : public QObject, public CPluginProcessInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "ikomia.plugin.process")
    Q_INTERFACES(CPluginProcessInterface)

    public:

        virtual std::shared_ptr<CProcessFactory> getProcessFactory()
        {
            return std::make_shared<CYoloV3Factory>();
        }

        virtual std::shared_ptr<CWidgetFactory> getWidgetFactory()
        {
            return std::make_shared<CYoloV3WidgetFactory>();
        }
};

#endif // YOLOV3_H

#include "YoloV3.h"
#include "Graphics/CGraphicsLayer.h"

CYoloV3::CYoloV3() : COcvDnnProcess()
{
    m_pParam = std::make_shared<CYoloV3Param>();
    addOutput(std::make_shared<CGraphicsProcessOutput>());
    addOutput(std::make_shared<CMeasureProcessIO>());
}

CYoloV3::CYoloV3(const std::string &name, const std::shared_ptr<CYoloV3Param> &pParam): COcvDnnProcess(name)
{
    m_pParam = std::make_shared<CYoloV3Param>(*pParam);
    addOutput(std::make_shared<CGraphicsProcessOutput>());
    addOutput(std::make_shared<CMeasureProcessIO>());
}

size_t CYoloV3::getProgressSteps() const
{
    return 3;
}

int CYoloV3::getNetworkInputSize() const
{
    return 416;
}

double CYoloV3::getNetworkInputScaleFactor() const
{
    return 1.0 / 255.0;
}

cv::Scalar CYoloV3::getNetworkInputMean() const
{
    return cv::Scalar();
}

void CYoloV3::run()
{
    beginTaskRun();
    auto pInput = std::dynamic_pointer_cast<CImageProcessIO>(getInput(0));
    auto pParam = std::dynamic_pointer_cast<CYoloV3Param>(m_pParam);

    if(pInput == nullptr || pParam == nullptr)
        throw CException(CoreExCode::INVALID_PARAMETER, "Invalid parameters", __func__, __FILE__, __LINE__);

    if(pInput->isDataAvailable() == false)
        throw CException(CoreExCode::INVALID_PARAMETER, "Empty image", __func__, __FILE__, __LINE__);

    //Force model files path
    std::string pluginDir = Utils::Plugin::getCppPath() + "/" + Utils::File::conformName(QString::fromStdString(m_name)).toStdString();
    pParam->m_structureFile = pluginDir + "/Model/yolov3.cfg";
    pParam->m_modelFile = pluginDir + "/Model/yolov3.weights";
    pParam->m_labelsFile = pluginDir + "/Model/coco_names.txt";

    CMat imgSrc;
    CMat imgOrigin = pInput->getImage();
    std::vector<cv::Mat> netOutputs;

    //Detection networks need color image as input
    if(imgOrigin.channels() < 3)
        cv::cvtColor(imgOrigin, imgSrc, cv::COLOR_GRAY2RGB);
    else
        imgSrc = imgOrigin;

    emit m_signalHandler->doProgress();

    try
    {
        if(m_net.empty() || pParam->m_bUpdate)
        {
            m_net = readDnn();
            if(m_net.empty())
                throw CException(CoreExCode::INVALID_PARAMETER, "Failed to load network", __func__, __FILE__, __LINE__);

            pParam->m_bUpdate = false;
        }

        int size = getNetworkInputSize();
        double scaleFactor = getNetworkInputScaleFactor();
        cv::Scalar mean = getNetworkInputMean();
        auto inputBlob = cv::dnn::blobFromImage(imgSrc, scaleFactor, cv::Size(size,size), mean, false, false);
        m_net.setInput(inputBlob);

        auto netOutNames = getOutputsNames();
        m_net.forward(netOutputs, netOutNames);
    }
    catch(cv::Exception& e)
    {
        throw CException(CoreExCode::INVALID_PARAMETER, e.what(), __func__, __FILE__, __LINE__);
    }

    endTaskRun();

    if(m_classNames.empty())
        readClassNames();

    emit m_signalHandler->doProgress();
    manageOutput(netOutputs[0]);
    emit m_signalHandler->doProgress();
}

void CYoloV3::manageOutput(cv::Mat &dnnOutput)
{
    forwardInputImage();

    auto pParam = std::dynamic_pointer_cast<CYoloV3Param>(m_pParam);
    auto pInput = std::dynamic_pointer_cast<CImageProcessIO>(getInput(0));
    CMat imgSrc = pInput->getImage();

    //Graphics output
    auto pGraphicsOutput = std::dynamic_pointer_cast<CGraphicsProcessOutput>(getOutput(1));
    pGraphicsOutput->setNewLayer(getName());
    pGraphicsOutput->setImageIndex(0);

    //Measures output
    auto pMeasureOutput = std::dynamic_pointer_cast<CMeasureProcessIO>(getOutput(2));
    pMeasureOutput->clearData();

    for(int i=0; i<dnnOutput.rows; ++i)
    {
        const int probabilityIndex = 5;
        const int probabilitySize = dnnOutput.cols - probabilityIndex;
        const float* pProbArray = &dnnOutput.at<float>(i, probabilityIndex);
        size_t objectClass = std::max_element(pProbArray, pProbArray + probabilitySize) - pProbArray;
        float confidence = dnnOutput.at<float>(i, (int)objectClass + probabilityIndex);

        if (confidence > pParam->m_confidence)
        {
            float xCenter = dnnOutput.at<float>(i, 0) * imgSrc.cols;
            float yCenter = dnnOutput.at<float>(i, 1) * imgSrc.rows;
            float width = dnnOutput.at<float>(i, 2) * imgSrc.cols;
            float height = dnnOutput.at<float>(i, 3) * imgSrc.rows;
            float left = xCenter - width/2;
            float top = yCenter - height/2;

            //Create rectangle graphics of bbox
            auto graphicsBox = pGraphicsOutput->addRectangle(left, top, width, height);

            //Retrieve class label
            std::string className = objectClass < m_classNames.size() ? m_classNames[objectClass] : "unknown " + std::to_string(objectClass);
            std::string label = className + " : " + std::to_string(confidence);
            pGraphicsOutput->addText(label, xCenter+5-width/2, yCenter+5-height/2);

            //Store values to be shown in results table
            std::vector<CObjectMeasure> results;
            results.emplace_back(CObjectMeasure(CMeasure(CMeasure::CUSTOM, QObject::tr("Confidence").toStdString()), confidence, graphicsBox->getId(), className));
            results.emplace_back(CObjectMeasure(CMeasure::Id::BBOX, {left, top, width, height}, graphicsBox->getId(), className));
            pMeasureOutput->addObjectMeasures(results);
        }
    }
}

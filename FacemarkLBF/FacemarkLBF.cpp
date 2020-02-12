#include "FacemarkLBF.h"
#include "Graphics/CGraphicsLayer.h"

CFacemarkLBF::CFacemarkLBF() : CImageProcess2d()
{
    addOutput(std::make_shared<CFeatureProcessIO<cv::Point2f>>());
    addOutput(std::make_shared<CGraphicsProcessOutput>());
}

CFacemarkLBF::CFacemarkLBF(const std::string name, const std::shared_ptr<CFacemarkLBFParam> &pParam) : CImageProcess2d(name)
{
    addOutput(std::make_shared<CFeatureProcessIO<cv::Point2f>>());
    addOutput(std::make_shared<CGraphicsProcessOutput>());
    m_pParam = std::make_shared<CFacemarkLBFParam>(*pParam);
}

size_t CFacemarkLBF::getProgressSteps() const
{
    return 3;
}

void CFacemarkLBF::run()
{
    beginTaskRun();
    auto pInput = std::dynamic_pointer_cast<CImageProcessIO>(getInput(0));
    auto pParam = std::dynamic_pointer_cast<CFacemarkLBFParam>(m_pParam);

    if(pInput == nullptr || pParam == nullptr)
        throw CException(CoreExCode::INVALID_PARAMETER, "Invalid parameters", __func__, __FILE__, __LINE__);

    if(pInput->isDataAvailable() == false)
        throw CException(CoreExCode::INVALID_PARAMETER, "Empty image", __func__, __FILE__, __LINE__);

    emit m_signalHandler->doSetTotalSteps(3);

    CMat imgSrc = pInput->getImage();
    emit m_signalHandler->doProgress();

    try
    {
        manageInputGraphics(imgSrc);

        // Load landmark detector
        if(m_pFacemark == nullptr)
        {
            // Create an instance of Facemark
            m_pFacemark = cv::face::FacemarkLBF::create();
            std::string modelFile = Utils::Plugin::getCppPath() + "/" + Utils::File::conformName(QString::fromStdString(m_name)).toStdString() + "/Model/lbfmodel.yaml";
            m_pFacemark->loadModel(modelFile);
        }
        // Variable for landmarks.
        // Landmarks for one face is a vector of points
        // There can be more than one face in the image. Hence, we
        // use a vector of vector of points.
        std::vector< std::vector<cv::Point2f> > landmarks;
        // Run landmark detector
        bool success = m_pFacemark->fit(imgSrc,m_faces,landmarks);
        if(success)
            manageOutput(landmarks);
    }
    catch(cv::Exception& e)
    {
        throw CException(CoreExCode::INVALID_PARAMETER, e.what(), __func__, __FILE__, __LINE__);
    }

    endTaskRun();
    emit m_signalHandler->doProgress();
    forwardInputImage();
    emit m_signalHandler->doProgress();
}

void CFacemarkLBF::drawPolyline(std::shared_ptr<CGraphicsProcessOutput> &pOutput, std::vector<cv::Point2f> &landmarks, size_t start, size_t end, bool bIsClosed)
{
    if(bIsClosed)
    {
        GraphicsPolygonProperty prop = *m_graphicsContextPtr->getPolygonPropertyPtr();

        auto pPolygon = new CGraphicsPolygon(prop, pOutput->getLayer());
        for(size_t i = start; i <= end; i++)
        {
            auto pt = landmarks[i];
            pPolygon->addPoint(QPointF(pt.x, pt.y));
        }
        pOutput->addItem(pPolygon);
    }
    else
    {
        GraphicsPolylineProperty prop = *m_graphicsContextPtr->getPolylinePropertyPtr();

        auto pPolyline = new CGraphicsPolyline(prop, pOutput->getLayer());
        for(size_t i = start; i <= end; i++)
        {
            auto pt = landmarks[i];
            pPolyline->addPoint(QPointF(pt.x, pt.y));
        }
        pOutput->addItem(pPolyline);
    }
}

void CFacemarkLBF::drawLandmarksPoint(std::shared_ptr<CGraphicsProcessOutput> &pOutput, std::vector<cv::Point2f> &landmarks)
{
    GraphicsPointProperty prop = *m_graphicsContextPtr->getPointPropertyPtr();

    auto currentItems = pOutput->getItems();
    for(size_t i = 0; i < landmarks.size(); i++)
    {
        auto pt = landmarks[i];
        auto pos = QPointF(pt.x, pt.y);
        auto pPoint = new CGraphicsPoint(pos, prop, pOutput->getLayer());
        currentItems.append(pPoint);
    }
    pOutput->setItems(currentItems);
}

void CFacemarkLBF::drawLandmarksFace(std::shared_ptr<CGraphicsProcessOutput> &pOutput, std::vector<cv::Point2f> &landmarks)
{
    // Draw face for the 68-point model.
    if (landmarks.size() == 68)
    {
        drawPolyline(pOutput, landmarks, 0, 16);           // Jaw line
        drawPolyline(pOutput, landmarks, 17, 21);          // Left eyebrow
        drawPolyline(pOutput, landmarks, 22, 26);          // Right eyebrow
        drawPolyline(pOutput, landmarks, 27, 30);          // Nose bridge
        drawPolyline(pOutput, landmarks, 30, 35, true);    // Lower nose
        drawPolyline(pOutput, landmarks, 36, 41, true);    // Left eye
        drawPolyline(pOutput, landmarks, 42, 47, true);    // Right Eye
        drawPolyline(pOutput, landmarks, 48, 59, true);    // Outer lip
        drawPolyline(pOutput, landmarks, 60, 67, true);    // Inner lip
    }
    else
    {
        // If the number of points is not 68, we do not know which
        // points correspond to which facial features. So, we draw
        // one dot per landmark.
        drawLandmarksPoint(pOutput, landmarks);
    }
}

void CFacemarkLBF::drawDelaunay(std::shared_ptr<CGraphicsProcessOutput> &pOutput, std::vector<cv::Point2f> &landmarks)
{
    auto pInput = std::dynamic_pointer_cast<CImageProcessIO>(getInput(0));
    cv::Rect rect(0, 0, (int)pInput->getImage().getNbCols(), (int)pInput->getImage().getNbRows());

    // Create an instance of Subdiv2D
    cv::Subdiv2D subdiv(rect);
    // Insert points into subdiv
    for( std::vector<cv::Point2f>::iterator it = landmarks.begin(); it != landmarks.end(); it++)
        subdiv.insert(*it);

    std::vector<cv::Vec6f> triangleList;
    subdiv.getTriangleList(triangleList);
    std::vector<cv::Point> pt(3);

    GraphicsPolygonProperty prop = *m_graphicsContextPtr->getPolygonPropertyPtr();

    for( size_t i = 0; i < triangleList.size(); i++ )
    {
        cv::Vec6f t = triangleList[i];
        pt[0] = cv::Point(cvRound(t[0]), cvRound(t[1]));
        pt[1] = cv::Point(cvRound(t[2]), cvRound(t[3]));
        pt[2] = cv::Point(cvRound(t[4]), cvRound(t[5]));

        // Draw rectangles completely inside the image.
        if ( rect.contains(pt[0]) && rect.contains(pt[1]) && rect.contains(pt[2]))
        {
            auto pTriangle = new CGraphicsPolygon(prop, pOutput->getLayer());
            pTriangle->addPoint(QPointF(pt[0].x, pt[0].y));
            pTriangle->addPoint(QPointF(pt[1].x, pt[1].y));
            pTriangle->addPoint(QPointF(pt[2].x, pt[2].y));
            pOutput->addItem(pTriangle);
        }
    }
}

void CFacemarkLBF::manageInputGraphics(const CMat &imgSrc)
{
    // Find face
    // Clear previous faces
    m_faces.clear();

    auto pGraphicsInput = std::dynamic_pointer_cast<CGraphicsProcessInput>(getInput(1));
    if(pGraphicsInput == nullptr)
        return;

    auto pLayer = pGraphicsInput->getLayer();
    if(pLayer == nullptr)
        return;

    auto childItems = pLayer->getChildItems();

    // Loop over bounding boxes
    for(auto&& it : childItems)
    {
        int x = it->boundingRect().x();
        int y = it->boundingRect().y();
        int w = it->boundingRect().width();
        int h = it->boundingRect().height();
        // Check if whole bb is inside image domain
        if(x >= 0 && y >= 0 && x+w < imgSrc.cols && y+h < imgSrc.rows)
        {
            cv::Rect roi(x, y, w, h);
            m_faces.push_back(roi);
        }
    }
}

void CFacemarkLBF::manageOutput(std::vector<std::vector<cv::Point2f> > &landmarks)
{
    auto pParam = std::dynamic_pointer_cast<CFacemarkLBFParam>(m_pParam);
    auto pGraphicOutput = std::dynamic_pointer_cast<CGraphicsProcessOutput>(getOutput(getOutputCount() - 1));
    if(pGraphicOutput == nullptr)
        throw CException(CoreExCode::NULL_POINTER, "Invalid graphics output", __func__, __FILE__, __LINE__);

    pGraphicOutput->emplaceLayer(new CGraphicsLayer(QString::fromStdString(getName())));
    pGraphicOutput->setImageIndex(0);

    // If successful, render the landmarks on the face
    switch(pParam->m_displayType)
    {
    case 0:
        for(size_t i = 0; i < landmarks.size(); i++)
            drawLandmarksPoint(pGraphicOutput, landmarks[i]);
        break;

    case 1:
        for(size_t i = 0; i < landmarks.size(); i++)
            drawLandmarksFace(pGraphicOutput, landmarks[i]);
        break;

    case 2:
        for(size_t i = 0; i < landmarks.size(); i++)
            drawDelaunay(pGraphicOutput, landmarks[i]);
    }

    auto pNumericOutput = std::dynamic_pointer_cast<CFeatureProcessIO<cv::Point2f>>(getOutput(1));
    if(pNumericOutput)
    {
        pNumericOutput->clearData();
        for(size_t i = 0; i < landmarks.size(); i++)
            pNumericOutput->addValueList(landmarks[i]);
    }
}

/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#include "SceneWidget.h"
#include "Delegate.h"
#include "PlayerWidget.h"
#include "SceneLoader.h"
#include "VPDFile.h"
#include "World.h"

#include <QtGui/QtGui>
#include <btBulletDynamicsCommon.h>
#include <vpvl/vpvl.h>
#include <vpvl/gl/Renderer.h>
#include "util.h"

namespace internal
{

class Grid {
public:
    static const int kLimit = 50;

    Grid() : m_vbo(0), m_cbo(0), m_ibo(0), m_list(0) {}
    ~Grid() {
        m_vertices.clear();
        m_colors.clear();
        m_indices.clear();
        if (m_vbo) {
            glDeleteBuffers(1, &m_vbo);
            m_vbo = 0;
        }
        if (m_cbo) {
            glDeleteBuffers(1, &m_cbo);
            m_cbo = 0;
        }
        if (m_ibo) {
            glDeleteBuffers(1, &m_ibo);
            m_ibo = 0;
        }
        if (m_list) {
            glDeleteLists(m_list, 1);
            m_list = 0;
        }
    }

    void initialize() {
        // draw black grid
        btVector3 lineColor(0.5f, 0.5f, 0.5f);
        uint16_t index = 0;
        for (int x = -kLimit; x <= kLimit; x += 5)
            addLine(btVector3(x, 0.0, -kLimit), btVector3(x, 0.0, x == 0 ? 0.0 : kLimit), lineColor, index);
        for (int z = -kLimit; z <= kLimit; z += 5)
            addLine(btVector3(-kLimit, 0.0f, z), btVector3(z == 0 ? 0.0f : kLimit, 0.0f, z), lineColor, index);
        // X coordinate (red)
        addLine(btVector3(0.0f, 0.0f, 0.0f), btVector3(kLimit, 0.0f, 0.0f), btVector3(1.0f, 0.0f, 0.0f), index);
        // Y coordinate (green)
        addLine(btVector3(0.0f, 0.0f, 0.0f), btVector3(0.0f, kLimit, 0.0f), btVector3(0.0f, 1.0f, 0.0f), index);
        // Z coordinate (blue)
        addLine(btVector3(0.0f, 0.0f, 0.0f), btVector3(0.0f, 0.0f, kLimit), btVector3(0.0f, 0.0f, 1.0f), index);
        m_list = glGenLists(1);
        glGenBuffers(1, &m_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(btVector3), &m_vertices[0], GL_STATIC_DRAW);
        glGenBuffers(1, &m_cbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_cbo);
        glBufferData(GL_ARRAY_BUFFER, m_colors.size() * sizeof(btVector3), &m_colors[0], GL_STATIC_DRAW);
        glGenBuffers(1, &m_ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(uint16_t), &m_indices[0], GL_STATIC_DRAW);
        // start compiling to render with list cache
        glNewList(m_list, GL_COMPILE);
        glDisable(GL_LIGHTING);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glVertexPointer(3, GL_FLOAT, sizeof(btVector3), 0);
        glBindBuffer(GL_ARRAY_BUFFER, m_cbo);
        glColorPointer(3, GL_FLOAT, sizeof(btVector3), 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        glDrawElements(GL_LINES, m_indices.size(), GL_UNSIGNED_SHORT, 0);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glEnable(GL_LIGHTING);
        glEndList();
    }

    void draw() const {
        glCallList(m_list);
    }

private:
    void addLine(const btVector3 &from, const btVector3 &to, const btVector3 &color, uint16_t &index) {
        m_vertices.push_back(from);
        m_vertices.push_back(to);
        m_colors.push_back(color);
        m_colors.push_back(color);
        m_indices.push_back(index);
        index++;
        m_indices.push_back(index);
        index++;
    }

    btAlignedObjectArray<btVector3> m_vertices;
    btAlignedObjectArray<btVector3> m_colors;
    btAlignedObjectArray<uint16_t> m_indices;
    GLuint m_vbo;
    GLuint m_cbo;
    GLuint m_ibo;
    GLuint m_list;
};

}

const QString SceneWidget::kWindowTileFormat = tr("%1 - Rendering scene (FPS: %2)");

SceneWidget::SceneWidget(QWidget *parent) :
    QGLWidget(QGLFormat(QGL::SampleBuffers), parent),
    m_bone(0),
    m_delegate(0),
    m_player(0),
    m_loader(0),
    m_world(0),
    m_grid(0),
    m_settings(0),
    m_frameCount(0),
    m_currentFPS(0),
    m_defaultFPS(60),
    m_interval(1000.0f / m_defaultFPS),
    m_internalTimerID(0),
    m_visibleBones(false),
    m_playing(false)
{
    m_delegate = new Delegate(this);
    m_grid = new internal::Grid();
    m_world = new World(m_defaultFPS);
    setAcceptDrops(true);
    setAutoFillBackground(false);
    setMinimumSize(540, 480);
}

SceneWidget::~SceneWidget()
{
    delete m_grid;
    m_grid = 0;
    delete m_world;
    m_world = 0;
    delete m_renderer;
    m_renderer = 0;
    delete m_delegate;
    m_delegate = 0;
}

void SceneWidget::play()
{
    m_title = windowTitle();
    m_renderer->scene()->seek(0.0f);
    m_playing = true;
    setWindowTitle(kWindowTileFormat.arg(qAppName()).arg(0));
}

void SceneWidget::stop()
{
    m_playing = false;
    m_renderer->scene()->seek(0.0f);
    setWindowTitle(m_title);
}

vpvl::PMDModel *SceneWidget::findModel(const QString &name)
{
    return m_loader->findModel(name);
}

void SceneWidget::setCurrentFPS(int value)
{
    if (value > 0) {
        m_defaultFPS = value;
        m_world->setCurrentFPS(value);
        m_renderer->scene()->setPreferredFPS(value);
    }
}

vpvl::PMDModel *SceneWidget::selectedModel() const
{
    return m_renderer->selectedModel();
}

vpvl::VMDMotion *SceneWidget::selectedMotion() const
{
    vpvl::PMDModel *model = selectedModel();
    return m_loader->findModelMotion(model);
}

void SceneWidget::setSelectedModel(vpvl::PMDModel *value)
{
    m_renderer->setSelectedModel(value);
    emit modelDidSelect(value);
}

void SceneWidget::addModel()
{
    QFileInfo fi(openFileDialog("sceneWidget/lastPMDDirectory", tr("Open PMD file"), tr("PMD file (*.pmd)")));
    if (fi.exists()) {
        QProgressDialog *progress = getProgressDialog("Loading the model...", 0);
        vpvl::VMDMotion *motion = 0;
        vpvl::PMDModel *model = m_loader->loadModel(fi.fileName(), fi.dir(), motion);
        if (model && motion) {
            emit modelDidAdd(model);
            setSelectedModel(model);
            emit motionDidAdd(motion, model);
        }
        else {
            QMessageBox::warning(this, tr("Loading model error"),
                                 tr("%1 cannot be loaded").arg(fi.fileName()));
        }
        delete progress;
    }
}

void SceneWidget::insertMotionToAllModels()
{
    QString fileName = openFileDialog("sceneWidget/lastVMDDirectory", tr("Open VMD (for model) file"), tr("VMD file (*.vmd)"));
    if (QFile::exists(fileName)) {
        QList<vpvl::PMDModel *> models;
        vpvl::VMDMotion *motion = m_loader->loadModelMotion(fileName, models);
        if (motion) {
            foreach (vpvl::PMDModel *model, models)
                emit motionDidAdd(motion, model);
        }
        else {
            QMessageBox::warning(this, tr("Loading model motion error"),
                                 tr("%1 cannot be loaded").arg(QFileInfo(fileName).fileName()));
        }
    }
}

void SceneWidget::insertMotionToSelectedModel()
{
    vpvl::PMDModel *selected = m_renderer->selectedModel();
    if (selected) {
        QString fileName = openFileDialog("sceneWidget/lastVMDDirectory", tr("Open VMD (for model) file"), tr("VMD file (*.vmd)"));
        if (QFile::exists(fileName)) {
            vpvl::VMDMotion *motion = m_loader->loadModelMotion(fileName, selected);
            if (motion)
                emit motionDidAdd(motion, selected);
            else
                QMessageBox::warning(this, tr("Loading model motion error"),
                                     tr("%1 cannot be loaded").arg(QFileInfo(fileName).fileName()));
        }
    }
    else {
        QMessageBox::warning(this, tr("The model is not selected."), tr("Select a model to insert the motion"));
    }
}

void SceneWidget::setEmptyMotion()
{
    vpvl::PMDModel *selected = m_renderer->selectedModel();
    if (selected) {
        vpvl::VMDMotion *motion = new vpvl::VMDMotion();
        m_loader->setModelMotion(motion, selected, QString());
        emit motionDidAdd(motion, selected);
    }
    else {
        QMessageBox::warning(this, tr("The model is not selected."), tr("Select a model to insert the motion"));
    }
}

void SceneWidget::setModelPose()
{
    vpvl::PMDModel *selected = m_renderer->selectedModel();
    if (selected) {
        QString fileName = openFileDialog("sceneWidget/lastVPDDirectory", tr("Open VPD file"), tr("VPD file (*.vpd)"));
        if (QFile::exists(fileName)) {
            VPDFile *pose = m_loader->loadPose(fileName, selected);
            if (pose)
                emit modelDidMakePose(pose, selected);
            else
                QMessageBox::warning(this, tr("Loading model pose error"),
                                     tr("%1 cannot be loaded").arg(QFileInfo(fileName).fileName()));
        }
    }
    else {
        QMessageBox::warning(this, tr("The model is not selected."), tr("Select a model to set the pose"));
    }
}

void SceneWidget::addAsset()
{
    QFileInfo fi(openFileDialog("sceneWidget/lastAssetDirectory", tr("Open X file"), tr("DirectX mesh file (*.x)")));
    if (fi.exists()) {
        QProgressDialog *progress = getProgressDialog("Loading the stage...", 0);
        vpvl::XModel *asset = m_loader->loadAsset(fi.fileName(), fi.dir());
        if (asset)
            emit assetDidAdd(asset);
        else
            QMessageBox::warning(this, tr("Loading stage error"),
                                 tr("%1 cannot be loaded").arg(fi.fileName()));
        delete progress;
    }
}

void SceneWidget::seekMotion(float frameIndex)
{
    vpvl::Scene *scene = m_renderer->scene();
    scene->updateModelView(0);
    scene->updateProjection(0);
    scene->seek(frameIndex);
    updateGL();
}

void SceneWidget::setCamera()
{
    QString fileName = openFileDialog("sceneWidget/lastCameraDirectory", tr("Open VMD (for camera) file"), tr("VMD file (*.vmd)"));
    if (QFile::exists(fileName)) {
        vpvl::VMDMotion *motion = m_loader->loadCameraMotion(fileName);
        if (motion)
            emit cameraMotionDidSet(motion);
        else
            QMessageBox::warning(this, tr("Loading camera motion error"),
                                 tr("%1 cannot be loaded").arg(QFileInfo(fileName).fileName()));
    }
}

void SceneWidget::deleteSelectedModel()
{
    vpvl::PMDModel *selected = m_renderer->selectedModel();
    if (m_loader->deleteModel(selected)) {
        emit modelDidDelete(selected);
        emit modelDidSelect(0);
    }
    else {
        QMessageBox::warning(this, tr("The model is not selected or exist."),
                             tr("Select a model to delete"));
    }
}

void SceneWidget::resetCamera()
{
    vpvl::Scene *scene = m_renderer->scene();
    scene->resetCamera();
    emit cameraPerspectiveDidSet(scene->position(), scene->angle(), scene->fovy(), scene->distance());
}

void SceneWidget::setCameraPerspective(btVector3 *pos, btVector3 *angle, float *fovy, float *distance)
{
    vpvl::Scene *scene = m_renderer->scene();
    btVector3 posValue, angleValue;
    float fovyValue, distanceValue;
    posValue = !pos ? scene->position() : *pos;
    angleValue = !angle ? scene->angle() : *angle;
    fovyValue = !fovy ? scene->fovy() : *fovy;
    distanceValue = !distance ? scene->distance() : *distance;
    scene->setCameraPerspective(posValue, angleValue, fovyValue, distanceValue);
    emit cameraPerspectiveDidSet(posValue, angleValue, fovyValue, distanceValue);
}

void SceneWidget::setBones(const QList<vpvl::Bone *> &bones)
{
    m_bone = bones.isEmpty() ? 0 : bones.last();
}

void SceneWidget::rotate(float x, float y)
{
    vpvl::Scene *scene = m_renderer->scene();
    btVector3 pos = scene->position(), angle = scene->angle();
    float fovy = scene->fovy(), distance = scene->distance();
    angle.setValue(angle.x() + x, angle.y() + y, angle.z());
    scene->setCameraPerspective(pos, angle, fovy, distance);
    emit cameraPerspectiveDidSet(pos, angle, fovy, distance);
}

void SceneWidget::translate(float x, float y)
{
    vpvl::Scene *scene = m_renderer->scene();
    btVector3 pos = scene->position(), angle = scene->angle();
    float fovy = scene->fovy(), distance = scene->distance();
    pos.setValue(pos.x() + x, pos.y() + y, pos.z());
    scene->setCameraPerspective(pos, angle, fovy, distance);
    emit cameraPerspectiveDidSet(pos, angle, fovy, distance);
}

void SceneWidget::zoom(bool up, const Qt::KeyboardModifiers &modifiers)
{
    vpvl::Scene *scene = m_renderer->scene();
    btVector3 pos = scene->position(), angle = scene->angle();
    float fovy = scene->fovy(), distance = scene->distance();
    float fovyStep = 1.0f, distanceStep = 4.0f;
    if (modifiers & Qt::ControlModifier && modifiers & Qt::ShiftModifier) {
        fovy = up ? fovy - fovyStep : fovy + fovyStep;
    }
    else {
        if (modifiers & Qt::ControlModifier)
            distanceStep *= 5.0f;
        else if (modifiers & Qt::ShiftModifier)
            distanceStep *= 0.2f;
        if (distanceStep != 0.0f)
            distance = up ? distance - distanceStep : distance + distanceStep;
    }
    scene->setCameraPerspective(pos, angle, fovy, distance);
    emit cameraPerspectiveDidSet(pos, angle, fovy, distance);
}

void SceneWidget::closeEvent(QCloseEvent *event)
{
    killTimer(m_internalTimerID);
    event->accept();
}

void SceneWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void SceneWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

void SceneWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void SceneWidget::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        const QList<QUrl> urls = mimeData->urls();
        vpvl::PMDModel *model = m_renderer->selectedModel();
        foreach (const QUrl url, urls) {
            QString path = url.toLocalFile();
            if (path.endsWith(".pmd", Qt::CaseInsensitive)) {
                QFileInfo modelPath(path);
                vpvl::VMDMotion *motion = 0;
                vpvl::PMDModel *model = m_loader->loadModel(modelPath.baseName(), modelPath.dir(), motion);
                if (model && motion) {
                    emit modelDidAdd(model);
                    setSelectedModel(model);
                    emit motionDidAdd(motion, model);
                }
            }
            else if (path.endsWith(".vmd") && model) {
                vpvl::VMDMotion *motion = m_loader->loadModelMotion(path, model);
                if (motion)
                    emit motionDidAdd(motion, model);
            }
            qDebug() << "Proceeded a dropped file:" << path;
        }
    }
}

void SceneWidget::initializeGL()
{
    GLenum err;
    if (!vpvl::gl::Renderer::initializeGLEW(err))
        qFatal("Cannot initialize GLEW: %s", glewGetErrorString(err));
    else
        qDebug("GLEW version: %s", glewGetString(GLEW_VERSION));
    qDebug("VPVL version: %s (%d)", VPVL_VERSION_STRING, VPVL_VERSION);
    m_renderer = new vpvl::gl::Renderer(m_delegate, width(), height(), m_defaultFPS);
    m_loader = new SceneLoader(m_renderer);
    m_renderer->setDebugDrawer(m_world->mutableWorld());
    vpvl::Scene *scene = m_renderer->scene();
    scene->setViewMove(0);
    // scene->setWorld(m_world->mutableWorld());
    m_grid->initialize();
    m_timer.start();
    m_internalTimerID = startTimer(m_interval);
}

void SceneWidget::mousePressEvent(QMouseEvent *event)
{
    m_prevPos = event->pos();
    vpvl::PMDModel *selected = m_renderer->selectedModel();
    if (selected) {
        const vpvl::BoneList &bones = selectedModel()->bones();
        const uint32_t nBones = bones.count();
        btVector3 coordinate;
        m_renderer->getObjectCoordinate(event->pos().x(), event->pos().y(), coordinate);
        QList< QPair<float, vpvl::Bone *> > result;
        for (uint32_t i = 0; i < nBones; i++) {
            vpvl::Bone *bone = bones[i];
            const btVector3 &p = coordinate - bone->localTransform().getOrigin();
            result.append(QPair<float, vpvl::Bone *>(p.length2(), bone));
        }
        qSort(result);
        QPair<float, vpvl::Bone *> pair = result.first();
        qDebug() << qPrintable(internal::toQString(pair.second)) << pair.first;
    }
}

void SceneWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        Qt::KeyboardModifiers modifiers = event->modifiers();
        QPoint diff = event->pos() - m_prevPos;
        if (modifiers & Qt::ControlModifier && modifiers & Qt::ShiftModifier) {
            vpvl::Scene *scene = m_renderer->scene();
            btVector3 position = scene->lightPosition();
            btQuaternion rx(0.0f, diff.y() * vpvl::radian(0.1f), 0.0f),
                    ry(0.0f, diff.x() * vpvl::radian(0.1f), 0.0f);
            position = position * btMatrix3x3(rx * ry);
            scene->setLightSource(scene->lightColor(), position);
        }
        else if (modifiers & Qt::ShiftModifier) {
            translate(diff.x() * -0.1f, diff.y() * 0.1f);
        }
        else {
            rotate(diff.y() * 0.5f, diff.x() * 0.5f);
        }
        m_prevPos = event->pos();
    }
}

void SceneWidget::paintGL()
{
    qglClearColor(Qt::white);
    m_renderer->initializeSurface();
    m_renderer->drawSurface();
    drawBones();
    drawGrid();
    emit surfaceDidUpdate();
}

void SceneWidget::resizeGL(int w, int h)
{
    m_renderer->resize(w, h);
}

void SceneWidget::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_internalTimerID) {
        vpvl::Scene *scene = m_renderer->scene();
        scene->updateModelView(0);
        scene->updateProjection(0);
        if (m_playing)
            scene->update(0.5f);
        updateGL();
    }
}

void SceneWidget::wheelEvent(QWheelEvent *event)
{
    zoom(event->delta() > 0, event->modifiers());
}

void SceneWidget::drawBones()
{
    if (m_visibleBones)
        m_renderer->drawModelBones(true, true);
    m_renderer->drawBoneTransform(m_bone);
}

void SceneWidget::drawGrid()
{
    m_grid->draw();
}

void SceneWidget::updateFPS()
{
    if (m_timer.elapsed() > 1000) {
        m_currentFPS = m_frameCount;
        m_frameCount = 0;
        m_timer.restart();
        setWindowTitle(kWindowTileFormat.arg(qAppName()).arg(m_currentFPS));
    }
    m_frameCount++;
}

QProgressDialog *SceneWidget::getProgressDialog(const QString &label, int max)
{
    QProgressDialog *progress = new QProgressDialog(label, tr("Cancel"), 0, max, this);
    progress->setMinimumDuration(0);
    progress->setValue(0);
    progress->setWindowModality(Qt::WindowModal);
    return progress;
}

const QString SceneWidget::openFileDialog(const QString &name, const QString &desc, const QString &exts)
{
    const QString path = m_settings->value(name).toString();
    const QString fileName = QFileDialog::getOpenFileName(this, desc, path, exts);
    if (!fileName.isEmpty()) {
        QDir dir(fileName);
        dir.cdUp();
        m_settings->setValue(name, dir.absolutePath());
    }
    return fileName;
}

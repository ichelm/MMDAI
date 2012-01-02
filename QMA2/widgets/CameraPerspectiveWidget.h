/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
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

#ifndef PERSPECTIONWIDGET_H
#define PERSPECTIONWIDGET_H

#include <QtGui/QWidget>
#include <vpvl/Common.h>

class QDoubleSpinBox;
class QLabel;
class QPushButton;

class CameraPerspectiveWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CameraPerspectiveWidget(QWidget *parent = 0);

public slots:
    void setCameraPerspective(const vpvl::Vector3 &pos, const vpvl::Vector3 &angle, float fovy, float distance);

signals:
    void cameraPerspectiveDidChange(vpvl::Vector3 *pos, vpvl::Vector3 *angle, float *fovy, float *distance);

private slots:
    void retranslate();
    void setCameraPerspectiveFront();
    void setCameraPerspectiveBack();
    void setCameraPerspectiveTop();
    void setCameraPerspectiveLeft();
    void setCameraPerspectiveRight();
    void updatePositionX(double value);
    void updatePositionY(double value);
    void updatePositionZ(double value);
    void updateRotationX(double value);
    void updateRotationY(double value);
    void updateRotationZ(double value);
    void updateFovy(double value);
    void updateDistance(double value);

private:
    vpvl::Vector3 m_currentPosition;
    vpvl::Vector3 m_currentAngle;
    QPushButton *m_frontLabel;
    QPushButton *m_backLabel;
    QPushButton *m_topLabel;
    QPushButton *m_leftLabel;
    QPushButton *m_rightLabel;
    QPushButton *m_cameraLabel;
    QLabel *m_positionLabel;
    QLabel *m_rotationLabel;
    QLabel *m_fovyLabel;
    QLabel *m_distanceLabel;
    QDoubleSpinBox *m_px;
    QDoubleSpinBox *m_py;
    QDoubleSpinBox *m_pz;
    QDoubleSpinBox *m_rx;
    QDoubleSpinBox *m_ry;
    QDoubleSpinBox *m_rz;
    QDoubleSpinBox *m_fovy;
    QDoubleSpinBox *m_distance;
    float m_currentFovy;
    float m_currentDistance;

    Q_DISABLE_COPY(CameraPerspectiveWidget)
};

#endif // PERSPECTIONWIDGET_H
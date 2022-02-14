/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "glwidget.h"
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QCoreApplication>
#include <math.h>

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent) {

}

GLWidget::~GLWidget() {
    cleanup();
}

QSize GLWidget::minimumSizeHint() const {
    return QSize(400, 400);
}

QSize GLWidget::sizeHint() const {
    return QSize(400, 400);
}

void GLWidget::setMesh(Mesh *mesh) {
    if (this->mesh != nullptr) {
        delete this->mesh;
        meshVBO.destroy();
    }

    this->mesh = mesh;
    meshIsReady = true;

    update();
}

static void qNormalizeAngle(int &angle) {
    while (angle < 0)
        angle += 360 * 16;
    while (angle > 360 * 16)
        angle -= 360 * 16;
}

void GLWidget::setXRotation(int angle) {
    qNormalizeAngle(angle);
    if (angle != xrot) {
        xrot = angle;
        emit xRotationChanged(angle);
        update();
    }
}

void GLWidget::setYRotation(int angle) {
    qNormalizeAngle(angle);
    if (angle != yrot) {
        yrot = angle;
        emit yRotationChanged(angle);
        update();
    }
}

void GLWidget::setZRotation(int angle) {
    qNormalizeAngle(angle);
    if (angle != zrot) {
        zrot = angle;
        emit zRotationChanged(angle);
        update();
    }
}

void GLWidget::cleanup() {
    if (shaderProgram == nullptr)
        return;
    makeCurrent();

    if (mesh != nullptr) {
        delete mesh;
        mesh = nullptr;

        meshVBO.destroy();
    }
    delete shaderProgram;
    shaderProgram = nullptr;

    doneCurrent();
}

static const char *vertexShaderSource =
    "attribute vec4 vertex;\n"
    "attribute vec3 normal;\n"
    "varying vec3 vert;\n"
    "varying vec3 vertNormal;\n"
    "uniform mat4 projMatrix;\n"
    "uniform mat4 mvMatrix;\n"
    "uniform mat3 normalMatrix;\n"
    "void main() {\n"
    "   vert = vertex.xyz;\n"
    "   vertNormal = normalMatrix * normal;\n"
    "   gl_Position = projMatrix * mvMatrix * vertex;\n"
    "}\n";

static const char *fragmentShaderSource =
    "varying highp vec3 vert;\n"
    "varying highp vec3 vertNormal;\n"
    "uniform highp vec3 lightPos;\n"
    "void main() {\n"
    "   highp vec3 L = normalize(lightPos - vert);\n"
    "   highp float NL = max(dot(normalize(vertNormal), L), 0.0);\n"
    "   highp vec3 color = vec3(0.39, 1.0, 0.0);\n"
    "   highp vec3 col = clamp(color * 0.2 + color * 0.8 * NL, 0.0, 1.0);\n"
    "   gl_FragColor = vec4(col, 1.0);\n"
    "}\n";

void GLWidget::initializeGL() {
    // In this example the widget's corresponding top-level window can change
    // several times during the widget's lifetime. Whenever this happens, the
    // QOpenGLWidget's associated context is destroyed and a new one is created.
    // Therefore we have to be prepared to clean up the resources on the
    // aboutToBeDestroyed() signal, instead of the destructor. The emission of
    // the signal will be followed by an invocation of initializeGL() where we
    // can recreate all resources.
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &GLWidget::cleanup);

    initializeOpenGLFunctions();
    glClearColor(0, 0, 0, 1);

    shaderProgram = new QOpenGLShaderProgram;
    shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    shaderProgram->bindAttributeLocation("vertex", 0);
    shaderProgram->bindAttributeLocation("normal", 1);
    shaderProgram->link();

    shaderProgram->bind();
    projMatrixLoc = shaderProgram->uniformLocation("projMatrix");
    mvMatrixLoc = shaderProgram->uniformLocation("mvMatrix");
    normalMatrixLoc = shaderProgram->uniformLocation("normalMatrix");
    lightPosLoc = shaderProgram->uniformLocation("lightPos");

    // Our camera never changes in this example.
    camera.setToIdentity();
    camera.translate(0, 0, -50);

    // Light position is fixed.
    shaderProgram->setUniformValue(lightPosLoc, QVector3D(0, 0, 70));

    shaderProgram->release();
}

void GLWidget::setupVertexAttribs() {
    meshVBO.bind();
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glEnableVertexAttribArray(0);
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
                             nullptr);
    f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
                             reinterpret_cast<void *>(3 * sizeof(GLfloat)));
    meshVBO.release();
}

void GLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    world.setToIdentity();
    world.rotate(180.0f - (xrot / 16.0f), 1, 0, 0);
    world.rotate(yrot / 16.0f, 0, 1, 0);
    world.rotate(zrot / 16.0f, 0, 0, 1);

    shaderProgram->bind();
    shaderProgram->setUniformValue(projMatrixLoc, proj);
    shaderProgram->setUniformValue(mvMatrixLoc, camera * world);
    QMatrix3x3 normalMatrix = world.normalMatrix();
    shaderProgram->setUniformValue(normalMatrixLoc, normalMatrix);

    if (meshIsReady) {
        meshIsReady = false;

        // Setup our vertex buffer object.
        meshVBO.create();
        meshVBO.bind();
        meshVBO.allocate(mesh->constData(), mesh->vertexCount() * 6 * sizeof(GLfloat));

        setupVertexAttribs();
    }

    if (mesh != nullptr) {
        glDrawArrays(GL_TRIANGLES, 0, mesh->vertexCount());
    }

    shaderProgram->release();
}

void GLWidget::resizeGL(int w, int h) {
    proj.setToIdentity();
    proj.perspective(fovAngle, GLfloat(w) / h, 0.01f, 100.0f);
}

void GLWidget::mousePressEvent(QMouseEvent *event) {
    lastPos = event->pos();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event) {
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        setXRotation(xrot + 8 * dy);
        setYRotation(yrot + 8 * dx);
    } else if (event->buttons() & Qt::RightButton) {
        setXRotation(xrot + 8 * dy);
        setZRotation(zrot + 8 * dx);
    }
    lastPos = event->pos();
}

void GLWidget::wheelEvent(QWheelEvent *event) {
    float delta = -event->delta() / 50.f;
    fovAngle += delta;
    fovAngle = std::max(5.f, std::min(150.f, fovAngle));

    proj.setToIdentity();
    proj.perspective(fovAngle, GLfloat(width()) / height(), 0.01f, 100.0f);

    update();
}

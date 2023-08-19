// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "glwidget.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

#define FRAME_SIZE 3 * 256 * 256

GLWidget::GLWidget()
{
    pixelVector.resize(FRAME_SIZE, 0);
}

GLWidget::~GLWidget()
{
    makeCurrent();
    vbo.destroy();
    for (int i = 0; i < 6; ++i)
        delete textures[i];
    delete program;
    doneCurrent();
}

void GLWidget::SetData(const void *data)
{
    uint8_t* newData = (uint8_t*)data;
    for (int i = 0; i < FRAME_SIZE; i++)
        pixelVector[i] = newData[i];
}

QSize GLWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize GLWidget::sizeHint() const
{
    return QSize(200, 200);
}

void GLWidget::rotateBy(int xAngle, int yAngle, int zAngle)
{
    xRot += xAngle;
    yRot += yAngle;
    zRot += zAngle;
    update();
}

void GLWidget::setClearColor(const QColor &color)
{
    clearColor = color;
    update();
}

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    makeObject();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_TEXCOORD_ATTRIBUTE 1

    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char *vsrc =
        "attribute highp vec4 vertex;\n"
        "attribute mediump vec4 texCoord;\n"
        "varying mediump vec4 texc;\n"
        "uniform mediump mat4 matrix;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = matrix * vertex;\n"
        "    texc = texCoord;\n"
        "}\n";
    vshader->compileSourceCode(vsrc);

    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    const char *fsrc =
        "uniform sampler2D texture;\n"
        "varying mediump vec4 texc;\n"
        "void main(void)\n"
        "{\n"
        "    gl_FragColor = texture2D(texture, texc.st);\n"
        "}\n";
    fshader->compileSourceCode(fsrc);

    program = new QOpenGLShaderProgram;
    program->addShader(vshader);
    program->addShader(fshader);
    program->bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
    program->bindAttributeLocation("texCoord", PROGRAM_TEXCOORD_ATTRIBUTE);
    program->link();

    program->bind();
    program->setUniformValue("texture", 0);
}

void GLWidget::paintGL()
{
    //for (int i = 0; i < 6; ++i)
        textures[0]->setData(QOpenGLTexture::RGB, QOpenGLTexture::UInt8, pixelVector.data());
    glClearColor(clearColor.redF(), clearColor.greenF(), clearColor.blueF(), clearColor.alphaF());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QMatrix4x4 m;
    m.ortho(-0.5f, +0.5f, +0.5f, -0.5f, 4.0f, 15.0f);
    m.translate(0.0f, 0.0f, -10.0f);
    m.rotate(xRot / 16.0f, 1.0f, 0.0f, 0.0f);
    m.rotate(yRot / 16.0f, 0.0f, 1.0f, 0.0f);
    m.rotate(zRot / 16.0f, 0.0f, 0.0f, 1.0f);

    program->setUniformValue("matrix", m);
    program->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    program->enableAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE);
    program->setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));
    program->setAttributeBuffer(PROGRAM_TEXCOORD_ATTRIBUTE, GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));

    textures[0]->bind();
    for (int i = 0; i < 6; ++i) {
        
        glDrawArrays(GL_TRIANGLE_FAN, i * 4, 4);
    }
}
void GLWidget::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);
}

void GLWidget::makeObject()
{
    static const int coords[6][4][3] = {
        { { +1, -1, -1 }, { -1, -1, -1 }, { -1, +1, -1 }, { +1, +1, -1 } },
        { { +1, +1, -1 }, { -1, +1, -1 }, { -1, +1, +1 }, { +1, +1, +1 } },
        { { +1, -1, +1 }, { +1, -1, -1 }, { +1, +1, -1 }, { +1, +1, +1 } },
        { { -1, -1, -1 }, { -1, -1, +1 }, { -1, +1, +1 }, { -1, +1, -1 } },
        { { +1, -1, +1 }, { -1, -1, +1 }, { -1, -1, -1 }, { +1, -1, -1 } },
        { { -1, -1, +1 }, { +1, -1, +1 }, { +1, +1, +1 }, { -1, +1, +1 } }
    };

    for (int j = 0; j < 6; ++j)
    {
        textures[j] = new QOpenGLTexture(QOpenGLTexture::Target2D);
        textures[j]->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
        textures[j]->create();

        textures[j]->setMagnificationFilter(QOpenGLTexture::Nearest);
        textures[j]->setMinificationFilter(QOpenGLTexture::Nearest);

        // given some `width`, `height` and `data_ptr`
        textures[j]->setSize(256, 256, 4);
        textures[j]->setFormat(QOpenGLTexture::RGBA8_UNorm);
        textures[j]->allocateStorage();     
    }



    std::vector<GLfloat> vertData;
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 4; ++j) {
            // vertex position
            vertData.push_back(0.2 * coords[i][j][0]);
            vertData.push_back(0.2 * coords[i][j][1]);
            vertData.push_back(0.2 * coords[i][j][2]);
            // texture coordinate
            vertData.push_back(j == 0 || j == 3);
            vertData.push_back(j == 0 || j == 1);
        }
    }

    vbo.create();
    vbo.bind();
    vbo.allocate(&vertData[0], vertData.size() * sizeof(GLfloat));
}

#ifndef SHADERTOYWIDGET_H
#define SHADERTOYWIDGET_H

#include "../core/shadereffect.h"

#include <QElapsedTimer>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QPointF>

class ShaderToyWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit ShaderToyWidget(QWidget *parent = nullptr);
    ~ShaderToyWidget() override;

    void setShaderEffect(ShaderEffect effect);
    ShaderEffect shaderEffect() const;
    void restartAnimation();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    bool ensureProgram();
    QString fragmentSourceForEffect(ShaderEffect effect) const;
    void destroyProgram();
    QVector4D mouseUniformValue() const;

    ShaderEffect m_shaderEffect = ShaderEffect::None;
    ShaderEffect m_compiledEffect = ShaderEffect::None;
    QOpenGLShaderProgram *m_program = nullptr;
    QOpenGLBuffer m_vertexBuffer;
    QOpenGLVertexArrayObject m_vertexArray;
    QElapsedTimer m_elapsedTimer;
    QPointF m_mousePosition;
    QPointF m_mousePressPosition;
    bool m_mousePressed = false;
};

#endif // SHADERTOYWIDGET_H

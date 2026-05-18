#ifndef SHADERTOYWIDGET_H
#define SHADERTOYWIDGET_H

#include "../core/shadereffect.h"

#include <QElapsedTimer>
#include <QImage>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QPointF>
#include <QString>

class ShaderToyWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit ShaderToyWidget(QWidget *parent = nullptr);
    ~ShaderToyWidget() override;

    void setShaderEffect(ShaderEffect effect);
    void setDreamShaderEnabled(bool enabled);
    void setExternalFragmentShaderFile(const QString &path);
    void setExternalChannel0UsesNoiseTexture(bool useNoiseTexture);
    void setSourcePixmap(const QPixmap &pixmap);
    ShaderEffect shaderEffect() const;
    bool isDreamShaderEnabled() const;
    QString externalFragmentShaderFile() const;
    bool externalChannel0UsesNoiseTexture() const;
    bool hasActiveShader() const;
    void restartAnimation();

signals:
    void shaderCompileFailed(const QString &message);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    bool ensureProgram();
    QString currentProgramKey() const;
    QString currentFragmentSource(QString *errorMessage) const;
    bool effectUsesSourceTexture(ShaderEffect effect) const;
    bool effectUsesNoiseTexture(ShaderEffect effect) const;
    QString fragmentSourceForEffect(ShaderEffect effect) const;
    QString buildExternalFragmentSource(const QString &source) const;
    void destroyProgram();
    void destroyTextures();
    void updateSourceTexture();
    void updateNoiseTexture();
    QVector4D mouseUniformValue() const;
    void reportShaderCompileFailure(const QString &message);

    ShaderEffect m_shaderEffect = ShaderEffect::None;
    QString m_externalFragmentShaderFile;
    QString m_compiledProgramKey;
    QString m_lastFailedProgramKey;
    QOpenGLShaderProgram *m_program = nullptr;
    QOpenGLTexture *m_sourceTexture = nullptr;
    QOpenGLTexture *m_noiseTexture = nullptr;
    QOpenGLBuffer m_vertexBuffer;
    QOpenGLVertexArrayObject m_vertexArray;
    QElapsedTimer m_elapsedTimer;
    QImage m_sourceImage;
    QPointF m_mousePosition;
    QPointF m_mousePressPosition;
    bool m_mousePressed = false;
    bool m_dreamShaderEnabled = false;
    bool m_externalChannel0UsesNoiseTexture = false;
    bool m_sourceTextureDirty = false;
    bool m_noiseTextureDirty = true;
};

#endif // SHADERTOYWIDGET_H

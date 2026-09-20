#pragma once

#if defined(HAVE_QT6)
#include <QWidget>
#include <QImage>
#include "editor/EditorEngine.h"
#include "render/RenderEngine.h"
#include "media/MediaLibrary.h"

namespace catchim::ui {

class PreviewWidget : public QWidget {
    Q_OBJECT
public:
    PreviewWidget(
        editor::EditorEngine& engine,
        render::RenderEngine& renderEngine,
        media::MediaLibrary& mediaLibrary,
        QWidget* parent = nullptr
    );

    void setZoomFactor(double factor) { zoomFactor_ = factor; update(); }
    double zoomFactor() const noexcept { return zoomFactor_; }
    void fitToScreen() { zoomFactor_ = -1.0; update(); }

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    editor::EditorEngine& engine_;
    render::RenderEngine& renderEngine_;
    media::MediaLibrary& mediaLibrary_;

    double zoomFactor_{-1.0}; // -1.0 means auto fit
    QPoint panOffset_{0, 0};
    QPoint lastMousePos_;
    bool isPanning_{false};
};

} // namespace catchim::ui
#endif

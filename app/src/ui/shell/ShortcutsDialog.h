#pragma once

#if defined(HAVE_QT6)
#include <QDialog>
#include <QString>
#include <vector>

namespace catchim::ui {

class ShortcutsDialog : public QDialog {
    Q_OBJECT
public:
    explicit ShortcutsDialog(QWidget* parent = nullptr);

private:
    void setupUi();
};

} // namespace catchim::ui
#endif

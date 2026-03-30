//
// Created by MoreW on 2026/3/25.
//

#ifndef CAM_DIALOGADDTOOL_H
#define CAM_DIALOGADDTOOL_H

#include <QDialog>


QT_BEGIN_NAMESPACE

namespace Ui {
    class DialogAddTool;
}

QT_END_NAMESPACE

class DialogAddTool : public QDialog {
    Q_OBJECT

public:
    explicit DialogAddTool(QWidget *parent = nullptr);

    ~DialogAddTool() override;

    [[nodiscard]] QString get_tool_name() const {
        return m_toolName;
    }

    [[nodiscard]] double get_diameter() const {
        return m_diameter;
    }

    [[nodiscard]] double get_flute_length() const {
        return m_fluteLength;
    }

    [[nodiscard]] double get_total_length() const {
        return m_totalLength;
    }

    [[nodiscard]] double get_corner_radius() const {
        return m_cornerRadius;
    }

    [[nodiscard]] int get_flute_count() const {
        return m_fluteCount;
    }

    [[nodiscard]] QString get_material() const {
        return m_material;
    }

    [[nodiscard]] QString get_type() const {
        return m_type;
    }

private:
    Ui::DialogAddTool *ui;

    QString m_toolName;
    double m_diameter;
    double m_fluteLength;
    double m_totalLength;
    double m_cornerRadius;
    int m_fluteCount;
    QString m_material;
    QString m_type;

private slots:
    void on_buttonBoxDialogAddTool_accepted();

    void on_buttonBoxDialogAddTool_rejected();
};


#endif //CAM_DIALOGADDTOOL_H

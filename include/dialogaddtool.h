/**
 * @file dialogaddtool.h
 * @brief 添加刀具对话框
 * @details 提供用户手动输入刀具参数的表单，确认后返回数据给主窗口
 */

#ifndef CAM_DIALOGADDTOOL_H
#define CAM_DIALOGADDTOOL_H

#include <QDialog>

QT_BEGIN_NAMESPACE

namespace Ui {
    class DialogAddTool;
}

QT_END_NAMESPACE

/**
 * @class DialogAddTool
 * @brief 添加刀具对话框
 * @details 用户填写刀具名称、几何参数、材质和类型后点击确认，
 *          通过 getter 方法将数据传递给调用方
 */
class DialogAddTool : public QDialog {
    Q_OBJECT

public:
    explicit DialogAddTool(QWidget *parent = nullptr);
    ~DialogAddTool() override;

    /** @brief 返回刀具名称 */
    [[nodiscard]] QString get_tool_name() const { return m_toolName; }

    /** @brief 返回刀具直径 (mm) */
    [[nodiscard]] double get_diameter() const { return m_diameter; }

    /** @brief 返回刃长 (mm) */
    [[nodiscard]] double get_flute_length() const { return m_fluteLength; }

    /** @brief 返回总长 (mm) */
    [[nodiscard]] double get_total_length() const { return m_totalLength; }

    /** @brief 返回刀尖圆角半径 (mm) */
    [[nodiscard]] double get_corner_radius() const { return m_cornerRadius; }

    /** @brief 返回刃数 */
    [[nodiscard]] int get_flute_count() const { return m_fluteCount; }

    /** @brief 返回材质描述 */
    [[nodiscard]] QString get_material() const { return m_material; }

    /** @brief 返回刀具类型 */
    [[nodiscard]] QString get_type() const { return m_type; }

private:
    Ui::DialogAddTool *ui;

    // ---- 由对话框 accept 时填充的输出数据 ----
    QString m_toolName;     ///< 刀具名称
    double  m_diameter;     ///< 直径 (mm)
    double  m_fluteLength;  ///< 刃长 (mm)
    double  m_totalLength;  ///< 总长 (mm)
    double  m_cornerRadius; ///< 刀尖圆角 (mm)
    int     m_fluteCount;   ///< 刃数
    QString m_material;     ///< 材质
    QString m_type;         ///< 刀具类型

private slots:
    /** @brief 用户点击确认，从 UI 控件读取并存储各参数 */
    void on_buttonBoxDialogAddTool_accepted();

    /** @brief 用户点击取消，关闭对话框 */
    void on_buttonBoxDialogAddTool_rejected();
};

#endif //CAM_DIALOGADDTOOL_H

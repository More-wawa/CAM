/**
 * @file dialogaddtool.cpp
 * @brief 添加刀具对话框实现
 * @details 读取用户输入的刀具参数并做基本合法性校验
 */

#include "../include/dialogaddtool.h"
#include "../ui/ui_DialogAddTool.h"
#include <QMessageBox>

DialogAddTool::DialogAddTool(QWidget *parent) : QDialog(parent), ui(new Ui::DialogAddTool) {
    ui->setupUi(this);
}

DialogAddTool::~DialogAddTool() {
    delete ui;
}

/**
 * @brief 确认按钮回调
 * @details 读取控件值，校验合法性后存储到成员变量并关闭对话框
 */
void DialogAddTool::on_buttonBoxDialogAddTool_accepted() {
    QString name     = ui->lineEditName->text().trimmed();
    QString type     = ui->lineEditType->text();
    QString material = ui->lineEditMaterial->text();

    bool okD, okFL, okTL, okCR, okFC;
    double diameter     = ui->lineEditDiameter->text().toDouble(&okD);
    double fluteLength  = ui->lineEditFluteLength->text().toDouble(&okFL);
    double totalLength  = ui->lineEditTotalLength->text().toDouble(&okTL);
    double cornerRadius = ui->lineEditCornerRadius->text().toDouble(&okCR);
    int    fluteCount   = ui->lineEditFluteCount->text().toInt(&okFC);

    // 必填项校验
    if (name.isEmpty()) {
        QMessageBox::warning(this, "提示", "刀具名称不能为空");
        return;
    }

    // 数值格式校验
    if (!okD || !okFL || !okTL || !okCR || !okFC) {
        QMessageBox::warning(this, "错误", "直径、刃长、总长、刀具圆角、刃数请输入正确的数字格式");
        return;
    }

    // 逻辑合法性校验
    if (fluteLength > totalLength) {
        QMessageBox::warning(this, "逻辑错误", "有效刃长不能大于总长度");
        return;
    }

    // 存储参数
    m_toolName    = name;
    m_diameter    = diameter;
    m_fluteLength = fluteLength;
    m_totalLength = totalLength;
    m_cornerRadius = cornerRadius;
    m_fluteCount  = fluteCount;
    m_material    = material;
    m_type        = type;

    accept();
}

void DialogAddTool::on_buttonBoxDialogAddTool_rejected() {
    reject();
}

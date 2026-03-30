//
// Created by MoreW on 2026/3/25.
//

// You may need to build the project (run Qt uic code generator) to get "ui_DialogAddTool.h" resolved

#include "../include/dialogaddtool.h"
#include "../ui/ui_DialogAddTool.h"
#include <QMessageBox>


DialogAddTool::DialogAddTool(QWidget *parent) : QDialog(parent), ui(new Ui::DialogAddTool) {
    ui->setupUi(this);
}

DialogAddTool::~DialogAddTool() {
    delete ui;
}
void DialogAddTool::on_buttonBoxDialogAddTool_accepted() {
    QString name = ui->lineEditName->text().trimmed();
    QString type = ui->lineEditType->text();
    QString material = ui->lineEditMaterial->text();
    bool okD, okFL, okTL, okCR, okFC;
    double diameter = ui->lineEditDiameter->text().toDouble(&okD);
    double fluteLength = ui->lineEditFluteLength->text().toDouble(&okFL);
    double totalLength = ui->lineEditTotalLength->text().toDouble(&okTL);
    double cornerRadius = ui->lineEditCornerRadius->text().toDouble(&okCR);
    int fluteCount = ui->lineEditFluteCount->text().toInt(&okFC);

    if (name.isEmpty()) {
        QMessageBox::warning(this, "提示", "刀具名称不能为空");
        return;
    }

    if (!okD || !okFL || !okTL || !okCR || !okFC) {
        QMessageBox::warning(this, "错误", "直径、刃长、总长、刀具圆角、刃数请输入正确的数字格式");
        return;
    }

    if (fluteLength > totalLength) {
        QMessageBox::warning(this, "逻辑错误", "有效刃长不能大于总长度");
        return;
    }

    this->m_toolName = name;
    this->m_diameter = diameter;
    this->m_fluteLength = fluteLength;
    this->m_totalLength = totalLength;
    this->m_cornerRadius = cornerRadius;
    this->m_fluteCount = fluteCount;
    this->m_material = material;
    this->m_type = type;

    accept();
}

void DialogAddTool::on_buttonBoxDialogAddTool_rejected() {
    reject();
}

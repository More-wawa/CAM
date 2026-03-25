//
// Created by MoreW on 2026/3/25.
//

// You may need to build the project (run Qt uic code generator) to get "ui_DialogAddTool.h" resolved

#include "../include/dialogaddtool.h"
#include "../ui/ui_DialogAddTool.h"


DialogAddTool::DialogAddTool(QWidget *parent) : QDialog(parent), ui(new Ui::DialogAddTool) {
    ui->setupUi(this);
}

DialogAddTool::~DialogAddTool() {
    delete ui;
}
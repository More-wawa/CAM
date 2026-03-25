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

private:
    Ui::DialogAddTool *ui;
};


#endif //CAM_DIALOGADDTOOL_H
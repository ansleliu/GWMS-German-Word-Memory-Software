#ifndef SPEAKDIALOG_H
#define SPEAKDIALOG_H

#include <QDialog>

namespace Ui {
class SpeakDialog;
}

class SpeakDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SpeakDialog(QWidget *parent = 0);
    ~SpeakDialog();
    
private:
    Ui::SpeakDialog *ui;
};

#endif // SPEAKDIALOG_H

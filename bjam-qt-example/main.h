#ifndef AA
#define AA

#include <qwidget.h>
#include <qpushbutton.h>
#include <qapplication.h>

class My_widget : public QWidget
{
    Q_OBJECT
public:
    My_widget();

private slots:
    void theSlot();
   
};

#endif

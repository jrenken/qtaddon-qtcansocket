#ifndef CONNECTDLG_H
#define CONNECTDLG_H

#include <QtGui/QDialog>
#include <QPair>
#include <qcansocket.h>
#include "ui_connectdlg.h"

class ConnectDlg : public QDialog
{
    Q_OBJECT

public:
    ConnectDlg(QWidget *parent = 0);
    ~ConnectDlg();

    QString interface() const;
    bool setInterface(const QString& iface);

    void setFilter(QPair<quint32, quint32>);
    QPair<quint32, quint32> filter() const;

    bool filterMessages() const {
    	return ui.groupBoxFilter->isChecked();
    }

    void setFilterMessages(bool val) {
    	ui.groupBoxFilter->setChecked(val);
    }

    void setErrorFrameMask(quint32 mask);
    quint32 errorFrameMask() const;

    bool generateErrorFrames() const {
    	return ui.groupBoxErrorFrames->isChecked();
    }

public slots:
	void on_radioButtonStandard_clicked();
	void on_radioButtonExtended_clicked();

private:
    Ui::ConnectDlgClass ui;
};

#endif // CONNECTDLG_H

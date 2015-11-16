/*
 *  file:		canmessagedlg.cpp
 *  author:		jrenken
 *
 *  $Rev: 74 $
 *  $Author: jrenken $
 *  $Date: 2012-08-30 15:50:09 +0200 (Do, 30 Aug 2012) $
 *  $Id: canmessagedlg.cpp 74 2012-08-30 13:50:09Z jrenken $
 */
#include "canmessagedlg.h"

CanMessageDlg::CanMessageDlg(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
	connect(ui.checkBoxExtended, SIGNAL(toggled(bool)), this, SLOT(setExtendedId(bool)));
	ui.lineEditId->setFocus();
//	ui.lineEditId->setCursorPosition(0);
}

CanMessageDlg::CanMessageDlg(const QCanMessage& msg, QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
	connect(ui.checkBoxExtended, SIGNAL(toggled(bool)), this, SLOT(setExtendedId(bool)));
	setMessage(msg);
}

CanMessageDlg::~CanMessageDlg()
{

}
void CanMessageDlg::setMessage(const QCanMessage& msg)
{
	if (msg.frame.isExtendedId()) {
		ui.checkBoxExtended->setChecked(true);
		ui.lineEditId->setText(QString("%1").arg(msg.frame.id(), 8, 16, QLatin1Char( '0' )));
	} else {
		ui.lineEditId->setText(QString("%1").arg(msg.frame.id(), 3, 16, QLatin1Char( '0' )));
	}
	ui.spinBoxDlc->setValue(msg.frame.dlc());
	ui.checkBoxRtr->setChecked(msg.frame.isRtr());
	ui.lineEditData0->setText(QString("%1").arg((quint8)msg.frame.data()[0], 2, 16, QLatin1Char( '0' )));
	ui.lineEditData1->setText(QString("%1").arg((quint8)msg.frame.data()[1], 2, 16, QLatin1Char( '0' )));
	ui.lineEditData2->setText(QString("%1").arg((quint8)msg.frame.data()[2], 2, 16, QLatin1Char( '0' )));
	ui.lineEditData3->setText(QString("%1").arg((quint8)msg.frame.data()[3], 2, 16, QLatin1Char( '0' )));
	ui.lineEditData4->setText(QString("%1").arg((quint8)msg.frame.data()[4], 2, 16, QLatin1Char( '0' )));
	ui.lineEditData5->setText(QString("%1").arg((quint8)msg.frame.data()[5], 2, 16, QLatin1Char( '0' )));
	ui.lineEditData6->setText(QString("%1").arg((quint8)msg.frame.data()[6], 2, 16, QLatin1Char( '0' )));
	ui.lineEditData7->setText(QString("%1").arg((quint8)msg.frame.data()[7], 2, 16, QLatin1Char( '0' )));
	ui.lineEditPeriod->setText(QString::number(msg.period));
//	ui.lineEditId->selectAll();
}


QCanMessage CanMessageDlg::getMessage() const
{
	bool ok;
	quint32	id = ui.lineEditId->text().toInt(&ok, 16);
	int	dlc = ui.spinBoxDlc->value();
	CanFrame frame(id, ui.checkBoxExtended->isChecked(), ui.checkBoxRtr->isChecked(), dlc);
	if (!frame.isRtr()) {
		frame[0] = ui.lineEditData0->text().toInt(&ok, 16);
		frame[1] = ui.lineEditData1->text().toInt(&ok, 16);
		frame[2] = ui.lineEditData2->text().toInt(&ok, 16);
		frame[3] = ui.lineEditData3->text().toInt(&ok, 16);
		frame[4] = ui.lineEditData4->text().toInt(&ok, 16);
		frame[5] = ui.lineEditData5->text().toInt(&ok, 16);
		frame[6] = ui.lineEditData6->text().toInt(&ok, 16);
		frame[7] = ui.lineEditData7->text().toInt(&ok, 16);
	}
	QCanMessage msg;
	msg.frame = frame;
	int period = ui.lineEditPeriod->text().toInt(&ok);
	if (ok && period > 0) {
		msg.period = period;
		msg.autoTrigger = true;
	}

	return msg;
}

void CanMessageDlg::setExtendedId(bool ext)
{
	if (ext) {
		ui.lineEditId->setInputMask("HHHHHHHH");
	} else {
		ui.lineEditId->setInputMask("HHH");
	}
}

void CanMessageDlg::showEvent(QShowEvent *)
{
	ui.lineEditId->selectAll();
}

void CanMessageDlg::on_spinBoxDlc_valueChanged(int val)
{
	ui.lineEditData0->setVisible(val > 0);
	ui.lineEditData1->setVisible(val > 1);
	ui.lineEditData2->setVisible(val > 2);
	ui.lineEditData3->setVisible(val > 3);
	ui.lineEditData4->setVisible(val > 4);
	ui.lineEditData5->setVisible(val > 5);
	ui.lineEditData6->setVisible(val > 6);
	ui.lineEditData7->setVisible(val > 7);
}

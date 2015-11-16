#include "connectdlg.h"
#include <linux/can.h>
#include <linux/can/error.h>

using namespace QtAddOn;



ConnectDlg::ConnectDlg(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
	QStringList	ifaces = QCanSocket::canInterfaces();
	ui.comboBoxInterface->insertItems(0, ifaces);
}

ConnectDlg::~ConnectDlg()
{

}

QString ConnectDlg::interface() const
{
	return ui.comboBoxInterface->currentText();
}

bool ConnectDlg::setInterface(const QString& iface)
{
	int i;
	if ((i = ui.comboBoxInterface->findText(iface)) != -1) {
		ui.comboBoxInterface->setCurrentIndex(i);
		return true;
	}
	return false;
}


void ConnectDlg::on_radioButtonStandard_clicked()
{
	if (ui.lineEditId->text().toLower() == "1fffffff")
		ui.lineEditId->setText("7ff");
	if (ui.lineEditMask->text().toLower() == "00000000")
		ui.lineEditMask->setText("000");
	ui.lineEditId->setInputMask("HHH");
	ui.lineEditMask->setInputMask("HHH");
}

void ConnectDlg::on_radioButtonExtended_clicked()
{
	ui.lineEditId->setInputMask("HHHHHHHH");
	ui.lineEditMask->setInputMask("HHHHHHHH");
	if (ui.lineEditId->text().toLower() == "7ff")
		ui.lineEditId->setText("1fffffff");
	if (ui.lineEditMask->text().toLower() == "000")
		ui.lineEditMask->setText("00000000");
}

void ConnectDlg::setFilter(QPair<quint32, quint32> filter)
{
	ui.lineEditId->setText(QString::number(filter.first, 16));
	ui.lineEditMask->setText( QString::number(filter.second, 16));
	ui.groupBoxFilter->setChecked(true);
}

QPair<quint32, quint32> ConnectDlg::filter() const
{
	return qMakePair(ui.lineEditId->text().toUInt(0, 16), ui.lineEditMask->text().toUInt(0, 16));
}

void ConnectDlg::setErrorFrameMask(quint32 mask)
{
	ui.checkBoxAllErrors->setChecked(mask == CAN_ERR_MASK);
	ui.checkBoxTxTimeout->setChecked(mask & CAN_ERR_TX_TIMEOUT);
	ui.checkBoxBusOff->setChecked(mask & CAN_ERR_BUSOFF);
	ui.checkBoxBusError->setChecked(mask & CAN_ERR_BUSERROR);
	if (mask)
		ui.groupBoxErrorFrames->setChecked(true);
}

quint32 ConnectDlg::errorFrameMask() const
{

	if (ui.checkBoxAllErrors->isChecked())
		return  CAN_ERR_MASK;

	quint32 mask = 0;

	mask |= (ui.checkBoxTxTimeout->isChecked() ?  CAN_ERR_TX_TIMEOUT : 0);
	mask |= (ui.checkBoxBusOff->isChecked() ?  CAN_ERR_BUSOFF : 0);
	mask |= (ui.checkBoxBusError->isChecked() ?  CAN_ERR_BUSERROR : 0);
	return mask;
}

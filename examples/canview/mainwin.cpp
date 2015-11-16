/*
 *  file:		mainwin.cpp
 *  author:		jrenken
 *
 *  $Rev: 74 $
 *  $Author: jrenken $
 *  $Date: 2012-08-30 15:50:09 +0200 (Do, 30 Aug 2012) $
 *  $Id: mainwin.cpp 74 2012-08-30 13:50:09Z jrenken $
 */

#include <QInputDialog>
#include <QDateTime>
#include <QMessageBox>
#include <QSettings>
#include <QItemSelectionModel>
#include <QDebug>
#include <linux/can.h>
#include <linux/can/error.h>

#include "mainwin.h"
#include "canmessagedlg.h"
#include "qcanmessage.h"
#include "cansendscheduler.h"
#include "connectdlg.h"

#define STR(x)   #x
#define XSTR(x)  STR(x)

using namespace QtAddOn;



MainWin::MainWin(QWidget *parent)
    : QMainWindow(parent)
{
	ui.setupUi(this);

	connect(ui.actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	QSettings settings;
    move(settings.value("Pos", QPoint(200, 200)).toPoint());
    resize(settings.value("Size", QSize(800, 600)).toSize());
	ui.splitter->restoreState(settings.value("Splitter").toByteArray());

	ui.statusbar->addPermanentWidget(ui.labelDevice);
	ui.statusbar->addPermanentWidget(ui.labelBusError);


	connect(&mSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
	connect(&mSocket, SIGNAL(error(quint32)), this, SLOT(onCanbusError(quint32)));
	mReceiveModel = new CanMessageModel(CanMessageModel::Receive, this);
	mReceiveModel->setShowTrigger(false);
	ui.tableViewReceive->setModel(mReceiveModel);
	ui.tableViewReceive->horizontalHeader()->resizeSection(2, 200);
	ui.tableViewReceive->horizontalHeader()->resizeSection(0, 100);
	ui.tableViewReceive->horizontalHeader()->resizeSection(1, 60);
	ui.tableViewReceive->horizontalHeader()->resizeSection(3, 60);
	ui.tableViewReceive->horizontalHeader()->resizeSection(4, 60);
	ui.tableViewReceive->horizontalHeader()->resizeSection(5, 60);

	mSendModel = new CanMessageModel(CanMessageModel::Transmit, this);
	ui.tableViewSend->setModel(mSendModel);
	ui.tableViewSend->horizontalHeader()->resizeSection(2, 200);
	ui.tableViewSend->horizontalHeader()->resizeSection(0, 100);
	ui.tableViewSend->horizontalHeader()->resizeSection(1, 60);
	ui.tableViewSend->horizontalHeader()->resizeSection(3, 60);
	ui.tableViewSend->horizontalHeader()->resizeSection(4, 60);
	ui.tableViewSend->horizontalHeader()->resizeSection(5, 60);

	mScheduler = new CanSendScheduler(this);
	connect(mScheduler, SIGNAL(jobScheduled(QCanMessage&)), this, SLOT(sendMessage(QCanMessage&)));

	QItemSelectionModel *selectionModel = new QItemSelectionModel(mSendModel);
    ui.tableViewSend->setSelectionModel(selectionModel);

    mConnection.device = settings.value("Socket/Name", QString()).toString();
    mConnection.filter = qMakePair(settings.value("Socket/FilterId", 0).toUInt(), settings.value("Socket/FilterMask", 0).toUInt());
    mConnection.appFilter = settings.value("Socket/DoFilter", false).toBool();
    mConnection.errorMask = settings.value("Socket/ErrorMask", 0).toUInt();
}

MainWin::~MainWin()
{
}

void MainWin::on_actionConnect_triggered()
{

	ConnectDlg	dlg;

	if (!mConnection.device.isEmpty()) {
		dlg.setInterface(mConnection.device);
	}
	if (mConnection.appFilter) {
		dlg.setFilter(mConnection.filter);
	}
	if (mConnection.errorMask) {
		dlg.setErrorFrameMask(mConnection.errorMask);
	}
	if (dlg.exec() == QDialog::Accepted) {
		QString iface = dlg.interface();
		if (!iface.isEmpty()) {
			mSocket.close();
			if (mSocket.bind(iface, QIODevice::ReadWrite)) {
				ui.actionDisconnect->setEnabled(true);
				ui.labelBusError->setText("Ok");
				mConnection.device = iface;
				ui.labelDevice->setText(iface);
				if (dlg.filterMessages()) {
					QPair<quint32, quint32> filter = dlg.filter();
					mSocket.setMessageFilter(filter);
					mConnection.filter = filter;
					mConnection.appFilter = true;
				} else {
					mConnection.filter = QPair<quint32, quint32>(0, 0);
					mConnection.appFilter = false;
				}
				if (dlg.generateErrorFrames()) {
					mSocket.setErrorFilter(dlg.errorFrameMask());
					mConnection.errorMask = dlg.errorFrameMask();
				} else {
					mConnection.errorMask = 0;
				}

			}
		}

	}
}

void MainWin::on_actionDisconnect_triggered()
{
	mSocket.close();
	ui.labelBusError->setText("offline");
	ui.actionDisconnect->setEnabled(false);
}

void MainWin::sendMessage(QCanMessage& msg)
{
	mSocket.write(msg.frame.frame(), msg.frame.size());
	msg.count++;
	mSendModel->addMessage(msg);
}

void MainWin::on_actionSend_triggered()
{
	QModelIndex idx = ui.tableViewSend->selectionModel()->currentIndex();
	if (!idx.isValid()) return;

	QByteArray ba = idx.data(Qt::UserRole).toByteArray();
	QCanMessage *msg = reinterpret_cast<QCanMessage*>(ba.data());
	sendMessage(*msg);
}

void MainWin::on_actionNew_triggered()
{
	CanMessageDlg dlg;
	if (dlg.exec() == QDialog::Accepted) {
		QCanMessage msg = dlg.getMessage();
		mSendModel->addMessage(msg, false);
		mScheduler->addJob(msg);
	}
}

void MainWin::on_actionDelete_triggered()
{
	QModelIndex idx = ui.tableViewSend->selectionModel()->currentIndex();
	if (!idx.isValid()) return;

	QByteArray ba = idx.data(Qt::UserRole).toByteArray();
	QCanMessage *msg = reinterpret_cast<QCanMessage*>(ba.data());
	mSendModel->deleteMessage(*msg);
	mScheduler->removeJob(*msg);
}

void MainWin::on_actionClearAll_triggered()
{
	mSendModel->deleteAll();
	mScheduler->clearAll();
}

void MainWin::on_actionEdit_triggered()
{
	QModelIndex idx = ui.tableViewSend->selectionModel()->currentIndex();

	on_tableViewSend_doubleClicked(idx);
}

void MainWin::on_actionClear_triggered()
{
	mReceiveModel->deleteAll();
	ui.textBrowserTrace->clear();
}

void MainWin::onReadyRead()
{
	CanFrame frame;
	qint64 time;
	if (mSocket.readMessage(frame.canFrame(), &time)) {
		QCanMessage msg;
		if (time != 0) {
			msg.time = time;
		} else {
			QDateTime now = QDateTime::currentDateTimeUtc();
			msg.time = now.toMSecsSinceEpoch();
		}
		msg.frame = frame;
		mReceiveModel->addMessage(msg);
		QString trace = QString("%1.%2      %3    %4    ").arg(msg.time / 1000, 16, 10, QLatin1Char(' ')).arg(msg.time % 1000, 3, 10, QLatin1Char('0'))
								.arg(msg.frame.id(), 8, 16).arg(msg.frame.dlc());
		for (int i = 0; i < msg.frame.dlc(); i++) {
			trace += QString("%1  ").arg(msg.frame[i], 2, 16, QLatin1Char('0'));
		}

		ui.textBrowserTrace->append(trace);
	}
}

void MainWin::onCanbusError(quint32 error)
{
	QString s;
	if (error) {
		if (error & CAN_ERR_TX_TIMEOUT)
			s += "TX timeout ";
		if (error & CAN_ERR_LOSTARB )
			s += "| lost arb. ";
		if (error & CAN_ERR_CRTL)
			s += "| ctrl err. ";
		if (error & CAN_ERR_PROT)
			s += "| proto. viol. ";
		if (error & CAN_ERR_TRX)
			s += "| transceiver status ";
		if (error & CAN_ERR_ACK)
			s += "| no ACK ";
		if (error & CAN_ERR_BUSOFF)
			s += "| bus off ";
		if (error & CAN_ERR_BUSERROR)
			s += "| bus error ";
		if (error & CAN_ERR_RESTARTED)
			s += "| contrl. restarted";
		if (s.startsWith('|'))
			ui.labelBusError->setText(s.mid(2));
		else
			ui.labelBusError->setText(s);
	} else {
		ui.statusbar->showMessage(mSocket.errorString(), 3000);
	}
}

void MainWin::on_tableViewSend_doubleClicked(const QModelIndex& index)
{

	if (index.isValid()) {
		QByteArray ba = index.data(Qt::UserRole).toByteArray();
		QCanMessage *msg = reinterpret_cast<QCanMessage*>(ba.data());
		if (index.column() == 0) {
			sendMessage(*msg);
		} else {
			CanMessageDlg dlg(*msg);
			if (dlg.exec() == QDialog::Accepted) {
				QCanMessage msg1 = dlg.getMessage();
				mSendModel->addMessage(msg1, false);
				mScheduler->addJob(msg1);
			}
		}
	}
}

void MainWin::on_actionAbout_triggered()
{
    QString versionInfo =   "<B>canview</B><p>"
                            "Author: jrenken    <a href=mailto:renken@marum.de>renken@marum.de</a><p>"
                            "(C) <i>Copyright 2013</i>, <b>University of Bremen, Marum</b>";
#ifdef GITHASH
    versionInfo += "<p>Commit: <b>" XSTR(GITHASH) "</b>";
#endif
    QMessageBox::about ( this,
                    "About Canview", versionInfo);


}

void MainWin::closeEvent(QCloseEvent *)
{
    QSettings settings;
    settings.setValue("Pos", pos());
    settings.setValue("Size", size());
    settings.setValue( "Splitter", ui.splitter->saveState());
    if (!mConnection.device.isEmpty()) {
    	settings.setValue("Socket/Name", mConnection.device);
    	settings.setValue("Socket/FilterId", mConnection.filter.first);
    	settings.setValue("Socket/FilterMask", mConnection.filter.second);
    	settings.setValue("Socket/DoFilter", mConnection.appFilter);
    	settings.setValue("Socket/ErrorMask", mConnection.errorMask);
    }
}

QString MainWin::messageToString(const QCanMessage& msg)
{
	QString result;

	if (msg.frame.isExtendedId()) {
		result = QString("%1  %2  ").arg(msg.frame.id(), 8, 16, QChar('0')).arg(msg.frame.dlc());
	} else {
		result = QString("     %1  %2  ").arg(msg.frame.id(), 3, 16, QChar('0')).arg(msg.frame.dlc());
	}
	for (int i = 0; i < 8; i++) {
		result += QString(" %1").arg((quint8)(msg.frame.data()[i]), 2, 16, QChar('0'));
	}
	return result;
}


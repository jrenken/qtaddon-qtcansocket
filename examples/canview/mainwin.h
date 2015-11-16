#ifndef MAINWIN_H
#define MAINWIN_H

#include <QtGui/QMainWindow>
#include "ui_mainwin.h"
#include "qcansocket.h"
#include "canmessagemodel.h"

using namespace QtAddOn;

class QItemSelectionModel;
class CanSendScheduler;


struct Connection {
	QString					device;
	QPair<quint32, quint32>	filter;
	bool					appFilter;
	quint32					errorMask;
	Connection()
		: device(QString()),
		  filter(QPair<quint32, quint32>(0, 0)),
		  appFilter(false),
		  errorMask(0) {}
};

class MainWin : public QMainWindow
{
    Q_OBJECT

public:
    MainWin(QWidget *parent = 0);
    ~MainWin();

public slots:
	void on_actionConnect_triggered();
	void on_actionDisconnect_triggered();
	void on_actionSend_triggered();
	void on_actionDelete_triggered();
	void on_actionEdit_triggered();
	void on_actionClearAll_triggered();
	void on_actionClear_triggered();
	void on_tableViewSend_doubleClicked(const QModelIndex& index);
	void on_actionNew_triggered();

	void onReadyRead();
	void onCanbusError(quint32 error);
	void on_actionAbout_triggered();
	void sendMessage(QCanMessage& msg);

	QString messageToString(const QCanMessage& msg);

protected:
	void closeEvent(QCloseEvent *);

private:
    Ui::MainWinClass 	ui;
    QCanSocket			mSocket;
    CanMessageModel		*mReceiveModel;
    CanMessageModel 	*mSendModel;
    CanSendScheduler	*mScheduler;
    QItemSelectionModel *mSelectionModel;
    Connection			mConnection;
    QCanMessage         mMsg402;

    void displayPdo1(const CanFrame &frame);

};

#endif // MAINWIN_H

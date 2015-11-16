/*
 *  file:	qcansocket_p.h
 *  author:	jrenken
 *
 *  $Rev: 73 $
 *  $Author: jrenken $
 *  $Date: 2012-08-29 17:27:04 +0200 (Mi, 29 Aug 2012) $
 *  $Id: cansocket_p.h 73 2012-08-29 15:27:04Z jrenken $
 */


#ifndef QCANSOCKET_P_H_
#define QCANSOCKET_P_H_

#include <QtCore/qglobal.h>
#include <QtCore/qstring.h>
#include <QtCore/qsocketnotifier.h>
#include <QtCore/qiodevice.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qpair.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>



namespace QtAddOn {

class QCanSocket;

class QCanSocketPrivate : public QObject
{
    Q_OBJECT
	Q_DECLARE_PUBLIC(QCanSocket)
public:
    QCanSocketPrivate(QCanSocket *parent);
    virtual ~QCanSocketPrivate();

    bool bind(const QString& interface);

	qint64 readData(char *data, qint64 maxlen);
	qint64 readLineData(char *data, qint64 maxlen);
	qint64 writeData(const char *data, qint64 len);

	bool readMessage(struct can_frame *data, qint64 *time);

	void close();

	void setNotifiersEnabled(bool enable, QIODevice::OpenMode mode);

	bool ReadNotification();
	bool WriteNotification();
	bool ExceptionNotification();

	void setMessageFilter(const QPair<quint32, quint32>&);
	void setErrorFilter(quint32 filter);

	static QStringList canInterfaces();

	can_frame mErrorFrame;

protected:
	bool eventFilter(QObject *obj, QEvent *e);

private:
    QCanSocket		*q_ptr;
    struct ifreq	mIfr;
    int				mSocketFd;

    QSocketNotifier *mReadNotifier;
    QSocketNotifier *mWriteNotifier;
    QSocketNotifier *mExceptionNotifier;

    bool mReadNotificationCalled;
    bool mWriteNotificationCalled;
    bool mExceptionNotificationCalled;
    bool mCanWrite;
};

}  /* namespace QtAddOn */
#endif /* QCANSOCKET_P_H_ */

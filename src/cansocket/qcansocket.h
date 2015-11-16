/*
 *  file:	cansocket.h
 *  author:	jrenken
 *
 *  $Rev: 73 $
 *  $Author: jrenken $
 *  $Date: 2012-08-29 17:27:04 +0200 (Mi, 29 Aug 2012) $
 *  $Id: cansocket.h 73 2012-08-29 15:27:04Z jrenken $
 */

#ifndef QCANSOCKET_H_
#define QCANSOCKET_H_

#include <QtCore/qiodevice.h>
#include <QtCore/qobject.h>
//#ifndef QT_NO_DEBUG_STREAM
#include <QtCore/qdebug.h>
//#endif

#include <linux/can.h>


namespace QtAddOn
{

class CanFrame
{
	struct can_frame mFrame;

public:
	CanFrame();
	CanFrame(quint32 id, quint8 dlc, unsigned char const* data);
	CanFrame(quint32 id, bool ext = false, bool rtr = false, quint8 dlc = 8, unsigned char const* data = 0);

	quint32	id() const;

	bool isExtendedId() const {
		return (mFrame.can_id & CAN_EFF_FLAG);
	}
	int dlc() const {
		return mFrame.can_dlc;
	}

	bool isRtr() const {
		return mFrame.can_id & CAN_RTR_FLAG;
	}

	void setRtr(bool on);

	inline int size() const {
		return sizeof(struct can_frame);
	}

	const char* frame() const {
		return (const char*) &mFrame;
	}
	char* frame() {
		return (char*) &mFrame;
	}

	struct can_frame* canFrame() {
		return &mFrame;
	}

	const char* data() const {
		return (const char*) mFrame.data;
	}

	char* data()  {
		return (char*) mFrame.data;
	}

	quint8 &operator[](int idx) {
		Q_ASSERT_X(idx < 8, "operator[]", "index exceeds range");
		return mFrame.data[idx];
	}

};

class QCanSocketPrivate;

class Q_NETWORK_EXPORT QCanSocket: public QIODevice
{
	Q_OBJECT

public:
	QCanSocket(QObject *parent = 0);
	virtual ~QCanSocket();

	bool bind(const QString& interface, QIODevice::OpenMode mode = QIODevice::ReadWrite);

	virtual bool open(QIODevice::OpenMode mode);
	virtual void close();

	bool readMessage(struct can_frame *data, qint64 *time);

	void setMessageFilter(const QPair<quint32, quint32>&);
	void setErrorFilter(quint32 filter);

	static QStringList canInterfaces();

	can_frame lastErrorFrame() const;


Q_SIGNALS:
	void error(quint32 errorClass);

protected Q_SLOTS:

protected:
	qint64 readData(char *data, qint64 maxlen);
	qint64 readLineData(char *data, qint64 maxlen);
	qint64 writeData(const char *data, qint64 len);

private:
	QCanSocketPrivate	*d_ptr;
    Q_DECLARE_PRIVATE(QCanSocket)
    Q_DISABLE_COPY(QCanSocket)

};

#ifndef QT_NO_DEBUG_STREAM
//Q_NETWORK_EXPORT QDebug operator<<(QDebug, CanSocket::SocketError);
//Q_NETWORK_EXPORT QDebug operator<<(QDebug, CanSocket::SocketState);
#endif


} /* namespace QtAddOn */
#endif /* QCANSOCKET_H_ */

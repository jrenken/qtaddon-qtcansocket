/*
 *  file:	qcansocket.cpp
 *  author:	jrenken
 *
 *  $Rev: 76 $
 *  $Author: jrenken $
 *  $Date: 2013-05-30 11:16:09 +0200 (Do, 30. Mai 2013) $
 *  $Id: cansocket.cpp 76 2013-05-30 09:16:09Z jrenken $
 */

/*!
    \class QCanSocket
    \brief The QCanSocket class provides access to SocketCan socket.
    \reentrant
    \ingroup network
    \inmodule QtAddOn
    SocketCan  is a set of open source CAN drivers and a networking
    stack contributed by Volkswagen Research to the Linux kernel.
*/

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <error.h>
#include <unistd.h>
#include <string.h>
#include <stddef.h>
#include <fcntl.h>
#include <errno.h>
#include <QtCore/qcoreevent.h>
#include <QtCore/qdir.h>
#include <QDebug>
#include "qcansocket_p.h"
#include "qcansocket.h"


namespace QtAddOn
{

#define d d_func()
#define q q_func()


//! class
/*!
*
*
*	@author Jens Renken <renken@marum.de>
*/

CanFrame::CanFrame()
{
	::memset((void*) &mFrame, 0, sizeof(mFrame));
}

CanFrame::CanFrame(quint32 id, quint8 dlc, unsigned char const* data )
{
	Q_ASSERT_X(dlc <= 8, "set dlc", "dlc > 8");

	if (id > CAN_EFF_MASK) {
		mFrame.can_id = id & CAN_EFF_MASK;
		mFrame.can_id |= CAN_EFF_FLAG;
	} else {
		mFrame.can_id = id & CAN_SFF_MASK;
	}
	mFrame.can_dlc = dlc;
	::memcpy(mFrame.data, data, 8);
}

CanFrame::CanFrame(quint32 id, bool ext, bool rtr, quint8 dlc, unsigned char const* data)
{
	Q_ASSERT_X(dlc <= 8, "set dlc", "dlc > 8");
	if (ext) {
		mFrame.can_id = id & CAN_EFF_MASK;
		mFrame.can_id |= CAN_EFF_FLAG;
	} else {
		mFrame.can_id = id & CAN_SFF_MASK;
	}

	if (rtr)
		mFrame.can_id |= CAN_RTR_FLAG;

	mFrame.can_dlc = dlc;
	if (data)
		::memcpy(mFrame.data, data, 8);
	else
		::memset((void*) mFrame.data, 0, sizeof(mFrame.data));
}

quint32	CanFrame::id() const
{
	if (mFrame.can_id & CAN_EFF_FLAG)
		return (mFrame.can_id & CAN_EFF_MASK);
	return (mFrame.can_id & CAN_SFF_MASK);
}

void CanFrame::setRtr(bool rtr)
{
	if (rtr)
		mFrame.can_id |= CAN_RTR_FLAG;
	else
		mFrame.can_id &= ~CAN_RTR_FLAG;
}


//! class
/*!
*
*
*	@author Jens Renken <renken@marum.de>
*/

QCanSocketPrivate::QCanSocketPrivate(QCanSocket *parent)
	: q_ptr(parent)
	, mSocketFd(-1)
	, mReadNotifier(0)
	, mWriteNotifier(0)
	, mExceptionNotifier(0)
	, mReadNotificationCalled(false)
	, mWriteNotificationCalled(false)
	, mExceptionNotificationCalled(false)
	, mCanWrite(true)
{

}

QCanSocketPrivate::~QCanSocketPrivate()
{

}

bool QCanSocketPrivate::bind(const QString& interface)
{
	struct sockaddr_can addr;

	if ((mSocketFd = ::socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
		q_ptr->setErrorString("socket: " + QString(strerror(errno)));
		emit q_ptr->error(0);
		return false;
	}
	addr.can_family = AF_CAN;
	qstrncpy(mIfr.ifr_name, interface.toAscii().data(), IFNAMSIZ);
	if (::ioctl(mSocketFd, SIOCGIFINDEX, &mIfr) < 0) {
		q_ptr->setErrorString("ioctl: " + QString(strerror(errno)));
		emit q_ptr->error(0);
		return false;
	}
	addr.can_ifindex = mIfr.ifr_ifindex;
	if (::bind(mSocketFd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		q_ptr->setErrorString("bind: " + QString(strerror(errno)));
		emit q_ptr->error(0);
		return false;
	}
	int on = 1;
    if (setsockopt(mSocketFd, SOL_SOCKET, SO_TIMESTAMP, &on,
		   sizeof(on)) != 0) {
    	perror("setsockopt");
		q_ptr->setErrorString("setsocketopt: " + QString(strerror(errno)));
		emit q_ptr->error(0);
    	return false;
    }

	mReadNotifier = new QSocketNotifier(mSocketFd, QSocketNotifier::Read, this);
	mReadNotifier->installEventFilter(this);
	mWriteNotifier = new QSocketNotifier(mSocketFd, QSocketNotifier::Write, this);
	mWriteNotifier->installEventFilter(this);
	mExceptionNotifier = new QSocketNotifier(mSocketFd, QSocketNotifier::Exception, this);
	mExceptionNotifier->installEventFilter(this);
	mReadNotificationCalled = false;
	mWriteNotificationCalled = false;
	mExceptionNotificationCalled = false;
	return true;
}

void QCanSocketPrivate::close()
{
	delete mReadNotifier;
	mReadNotifier = 0;
	delete mWriteNotifier;
	mWriteNotifier = 0;
	delete mExceptionNotifier;
	mExceptionNotifier = 0;
	::close(mSocketFd);

}

bool QCanSocketPrivate::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::SockAct) {
        if (obj == mExceptionNotifier) {
//        	qDebug() << "ExceptionNotifier";
        	ExceptionNotification();
            return true;
        }
        if (obj == mReadNotifier) {
//        	qDebug() << "ReadNotifier";
        	ReadNotification();
        	return true;
        }
        if (obj == mWriteNotifier) {
//        	qDebug() << "WriteNotifier";
        	WriteNotification();
            return true;
        }
    }
	return QObject::eventFilter(obj, e);
}


bool QCanSocketPrivate::ReadNotification()
{
	if (!mReadNotificationCalled) {
		mReadNotifier->setEnabled(false);
		mReadNotificationCalled = true;
		emit q_ptr->readyRead();
	}
	return true;
}

bool QCanSocketPrivate::WriteNotification()
{
	if (!mWriteNotificationCalled) {
		mWriteNotifier->setEnabled(false);
		mWriteNotificationCalled = true;
	}
	return true;
}

bool QCanSocketPrivate::ExceptionNotification()
{
	return true;
}


bool QCanSocketPrivate::readMessage(struct can_frame *frame, qint64 *time)
{
	struct timeval tv;
	bool haveTimestamp = false;

	int res;
	char control[1024];
	struct iovec    iov;
	struct msghdr msgh;
	struct cmsghdr *cmsg;
	iov.iov_base = frame;
	iov.iov_len = sizeof(*frame);
	msgh.msg_name = 0;
	msgh.msg_namelen = 0;
	msgh.msg_control = control;
	msgh.msg_controllen = sizeof(control);
	msgh.msg_iovlen = 1;
	msgh.msg_iov = &iov;
	msgh.msg_flags = 0;

	res = ::recvmsg(mSocketFd, &msgh, 0);
	if (res == -1 && errno != EAGAIN) {
		perror("recvmsg");
		q_ptr->setErrorString("recvmsg: " + QString(strerror(errno)));
		emit q_ptr->error(0);
		return false;
	}
	if (mReadNotificationCalled) {
		mReadNotifier->setEnabled(true);
		mReadNotificationCalled = false;
	}


	if (res >= 0) {
		/* Receive auxiliary data in msgh */
		for (cmsg = CMSG_FIRSTHDR(&msgh); cmsg != NULL;
				cmsg = CMSG_NXTHDR(&msgh,cmsg)) {
			if (cmsg->cmsg_level == SOL_SOCKET
					&& cmsg->cmsg_type == SO_TIMESTAMP) {
				tv = *(struct timeval*)(void*)CMSG_DATA(cmsg);
				haveTimestamp = true;
				break;
			}
		}
		if (cmsg == NULL) {
			*time = 0;
		}

		if (iov.iov_len == 0)
			return false;
	}


	if (frame->can_id & CAN_ERR_FLAG) {
		qDebug() << "Errorframe";
		mErrorFrame = *frame;
		emit q_ptr->error(frame->can_id & CAN_ERR_MASK);
	}
	if (haveTimestamp)
		*time     =  tv.tv_sec * 1000 + tv.tv_usec / 1000;
	return true;
}

qint64 QCanSocketPrivate::readData(char *data, qint64 maxlen)
{
	if ((unsigned int) maxlen < sizeof(struct can_frame))
		return 0;

	int bytes = ::read(mSocketFd, data, maxlen);
	if (bytes < 0) {
		perror("read");
		q_ptr->setErrorString("read: " + QString(strerror(errno)));
		emit q_ptr->error(0);
		return -1;
	}
	if ((unsigned int) bytes < sizeof(struct can_frame)) {
		qDebug() << "incomplete frame";
		return -1;
	}
	if (mReadNotificationCalled) {
		mReadNotifier->setEnabled(true);
		mReadNotificationCalled = false;
	}

	quint32 id = reinterpret_cast<can_frame*>(data)->can_id;
	if (id & CAN_ERR_FLAG) {
		qDebug() << "Errorframe";
		mErrorFrame = *reinterpret_cast<can_frame*>(data);
		emit q_ptr->error(id & CAN_ERR_MASK);
	}
	return bytes;
}

qint64 QCanSocketPrivate::readLineData(char *data, qint64 maxlen)
{
	Q_UNUSED(data);
	Q_UNUSED(maxlen);
	return 0;

}

qint64 QCanSocketPrivate::writeData(const char *data, qint64 len)
{
	qint64 written = 0;

	if (mWriteNotificationCalled) {
		written = ::write(mSocketFd, data, len);
//		qDebug() << "Write";
		if (written > 0) {
			emit q_ptr->bytesWritten(written);
		}
		mWriteNotificationCalled = false;
		mWriteNotifier->setEnabled(true);
	}

	return written;
}

void QCanSocketPrivate::setNotifiersEnabled(bool enable, QIODevice::OpenMode mode)
{
	if (enable) {
		if (mode == QIODevice::ReadOnly) {
			mReadNotifier->setEnabled(true);
			mWriteNotifier->setEnabled(false);
		} else if (mode == QIODevice::WriteOnly) {
			mReadNotifier->setEnabled(false);
			mWriteNotifier->setEnabled(true);
		} else {
			mReadNotifier->setEnabled(true);
			mWriteNotifier->setEnabled(true);
		}
		mExceptionNotifier->setEnabled(true);
	} else {
		mReadNotifier->setEnabled(false);
		mWriteNotifier->setEnabled(false);
		mExceptionNotifier->setEnabled(false);
	}
}

QStringList QCanSocketPrivate::canInterfaces()
{
	QStringList tmp;

	QDir 	dir("/sys/class/net");
	dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

	QFileInfoList list = dir.entryInfoList();

	for (int i = 0; i < list.size(); ++i) {
		QFile f(list.at(i).absoluteFilePath() + "/type");
		if (f.open(QIODevice::ReadOnly)) {
			QByteArray ba = f.readLine();
			if (ba.trimmed().toInt() == 280) {
				tmp << list.at(i).fileName();
			}
		}
	}
	return tmp;
}

void QCanSocketPrivate::setMessageFilter(const QPair<quint32, quint32>& filter)
{
	if (mSocketFd == -1) return;
	struct can_filter rfilter[1];
	rfilter[0].can_id   = filter.first;
	rfilter[0].can_mask = filter.second;
	qDebug() << rfilter[0].can_id << rfilter[0].can_mask;
	::setsockopt(mSocketFd, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));

}

void QCanSocketPrivate::setErrorFilter(quint32 filter)
{
	can_err_mask_t err_mask = filter;

	setsockopt(mSocketFd, SOL_CAN_RAW, CAN_RAW_ERR_FILTER,
	               &err_mask, sizeof(err_mask));
}

/*!
    Creates a QCanSocket object in state \c UnconnectedState.
    \a parent is passed on to the QObject constructor.
    \sa socketType()
*
*   @author Jens Renken <renken@marum.de>
*/

QCanSocket::QCanSocket(QObject *parent)
	: QIODevice(parent),
	  d_ptr(new QCanSocketPrivate(this))
{
	// TODO Auto-generated constructor stub

}

/*!
    Destroys the socket, closing the connection if necessary.
    \sa close()
*/

QCanSocket::~QCanSocket()
{
	delete d;
}

bool QCanSocket::bind(const QString& interface, QIODevice::OpenMode mode)
{
	if (d->bind(interface)) {
		d->setNotifiersEnabled(true, mode);
		return QIODevice::open(QIODevice::ReadWrite | QIODevice::Unbuffered);
	}
	return false;
}

bool QCanSocket::open(QIODevice::OpenMode)
{
	Q_ASSERT_X(false, "open", "Use bind to open Can device");
	return false;
}

void QCanSocket::close()
{
	d->close();
	QIODevice::close();
}
bool QCanSocket::readMessage(struct can_frame *frame, qint64 *time)
{
	return d->readMessage(frame, time);
}

qint64 QCanSocket::readData(char *data, qint64 maxlen)
{
	return d->readData(data, maxlen);
}

qint64 QCanSocket::readLineData(char *data, qint64 maxlen)
{
	return d->readLineData(data, maxlen);
}

qint64 QCanSocket::writeData(const char *data, qint64 len)
{
	return d->writeData(data, len);
}

QStringList QCanSocket::canInterfaces()
{
	return QCanSocketPrivate::canInterfaces();
}

void QCanSocket::setMessageFilter(const QPair<quint32, quint32>& filter)
{
	d->setMessageFilter(filter);
}

void QCanSocket::setErrorFilter(quint32 filter)
{
	d->setErrorFilter(filter);
}

can_frame QCanSocket::lastErrorFrame() const
{
	return d->mErrorFrame;
}


} /* namespace QtAddOn */





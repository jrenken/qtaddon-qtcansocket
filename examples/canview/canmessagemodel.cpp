/*
 *  file:		canmessagemodel.cpp
 *  author:		jrenken
 *
 *  $Rev: 74 $
 *  $Author: jrenken $
 *  $Date: 2012-08-30 15:50:09 +0200 (Do, 30 Aug 2012) $
 *  $Id: canmessagemodel.cpp 74 2012-08-30 13:50:09Z jrenken $
 */

#include <QByteArray>
#include "canmessagemodel.h"

CanMessageModel::CanMessageModel(Type type, QObject *parent)
	: QAbstractTableModel(parent),
	  mType(type),
	  mShowTrigger(true)
{
	// TODO Auto-generated constructor stub
}

CanMessageModel::~CanMessageModel()
{
	// TODO Auto-generated destructor stub
}

void CanMessageModel::setData(const CanMessages & data)
{
	mMessages = data;
}

void CanMessageModel::addMessage(const QCanMessage& msg, bool count)
{
	QCanMessage nmsg = msg;
	bool isNew = false;
	if (mMessages.contains(msg.frame.id())) {
		nmsg.sinceLast = msg.time - mMessages.value(msg.frame.id()).time;
		if (count)
			nmsg.count = mMessages.value(msg.frame.id()).count + 1;
	} else {
		nmsg.sinceLast = 0;
		if (count)
			nmsg.count = 1;
		isNew = true;
	}
	mMessages.insert(msg.frame.id(), nmsg);
	int idx = mMessages.keys().indexOf(msg.frame.id());
	if (isNew) {
		reset();
	}
	emit dataChanged(index(idx, 1), index(idx, 4));
}

void CanMessageModel::replaceMessage(const QCanMessage& oldMsg, const QCanMessage& newMsg)
{
	mMessages.remove(oldMsg.frame.id());
	addMessage(newMsg);
}

void CanMessageModel::deleteMessage(const QCanMessage& msg)
{
	if (mMessages.remove(msg.frame.id()) > 0)
		reset();
}

void CanMessageModel::deleteAll()
{
	mMessages.clear();
	reset();
}


void CanMessageModel::setShowTrigger(bool show)
{
	mShowTrigger = show;
	reset();
}


int CanMessageModel::rowCount(const QModelIndex &) const
{
	return mMessages.size();
}

int CanMessageModel::columnCount(const QModelIndex &) const
{
	if (mShowTrigger)
		return 6;
	return 5;
}

QVariant CanMessageModel::data(const QModelIndex &index, int role) const
{
	if ((role != Qt::DisplayRole) && (role != Qt::UserRole))
        return QVariant();

	QCanMessage msg = mMessages.value(mMessages.keys().at(index.row()));

	if (role == Qt::UserRole) {
		return QByteArray((const char*) &msg, sizeof(QCanMessage));
	}

	switch (index.column()) {
	case 0:
		if (msg.frame.isExtendedId()) {
			return QString("0x%1").arg(msg.frame.id(), 8, 16, QLatin1Char('0'));
		}
		return QString("0x%1").arg(msg.frame.id(), 3, 16, QLatin1Char('0'));
	case 1:
		return msg.frame.dlc();
	case 2:
		if (msg.frame.isRtr()) {
			return tr("Remote Request");
		} else {
			QString s;
			for (int i = 0; i < msg.frame.dlc(); i++) {
				s += QString("%1 ").arg(msg.frame[i], 2, 16, QLatin1Char('0'));
			}
			return s;
		}
		break;
	case 3:
		if (mType == Transmit) {
			if (msg.autoTrigger)
				return msg.period;
			return "Wait";
		}
		return msg.sinceLast;
	case 4:
		return msg.count;
	case 5:
		return ( msg.autoTrigger ? tr("Auto") : tr("Manual"));

	}
	return QVariant();
}

QVariant CanMessageModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
    	switch (section) {
    	case 0:
    		return tr("Message");
    	case 1:
    		return tr("Length");
    	case 2:
    		return tr("Data");
    	case 3:
    		return tr("Period");
    	case 4:
    		return tr("Count");
    	case 5:
    		return tr("Trigger");
    	}
    }
    return QVariant();

}

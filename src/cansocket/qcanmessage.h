/*
 *  file:		canmessage.h
 *  author:		jrenken
 *
 *  $Rev: 74 $
 *  $Author: jrenken $
 *  $Date: 2012-08-30 15:50:09 +0200 (Do, 30 Aug 2012) $
 *  $Id: canmessage.h 74 2012-08-30 13:50:09Z jrenken $
 */

#ifndef CANMESSAGE_H_
#define CANMESSAGE_H_

#include <QMap>
#include "qcansocket.h"

using namespace QtAddOn;

struct QCanMessage {
	CanFrame	frame;
	qint64		time;
	qint64		sinceLast;
	qint64		period;
	bool		autoTrigger;
	int			count;
	QCanMessage()
		:	time(0),
		 	sinceLast(0),
		 	period(0),
		 	autoTrigger(false),
		 	count(0) {}
};

typedef QMap<quint32, QCanMessage> CanMessages;

#endif /* CANMESSAGE_H_ */

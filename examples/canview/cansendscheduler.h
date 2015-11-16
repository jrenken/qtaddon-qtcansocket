/*
 * CanSendScheduler.h
 *
 *  Created on: 31.05.2013
 *      Author: jrenken
 */

#ifndef CANSENDSCHEDULER_H_
#define CANSENDSCHEDULER_H_

#include <QObject>
#include <QMap>
#include <QPair>
#include <qcanmessage.h>


class CanSendScheduler: public QObject
{
	Q_OBJECT

public:
	CanSendScheduler(QObject * = 0);

	void addJob(const QCanMessage& msg);
	void removeJob(const QCanMessage& msg);
	void clearAll();

signals:
	void jobScheduled(QCanMessage& msg);

protected:
	void timerEvent(QTimerEvent* event);

private:
	QMap<int, QCanMessage>	mJobs;
};

#endif /* CANSENDSCHEDULER_H_ */

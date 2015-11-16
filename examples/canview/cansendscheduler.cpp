/*
 * CanSendScheduler.cpp
 *
 *  Created on: 31.05.2013
 *      Author: jrenken
 */

#include <QTimerEvent>
#include <QDebug>
#include "cansendscheduler.h"

CanSendScheduler::CanSendScheduler(QObject* parent)
	: QObject(parent)
{
}


void CanSendScheduler::addJob(const QCanMessage& msg)
{
	removeJob(msg);
	if (msg.autoTrigger && msg.period > 0) {
		int timer = startTimer(msg.period);
		mJobs.insert(timer, msg);
	}
}

void CanSendScheduler::removeJob(const QCanMessage& msg)
{
	foreach (int key, mJobs.keys()) {
		if (mJobs.value(key).frame.id() == msg.frame.id()) {
			killTimer(key);
			mJobs.remove(key);
		}
	}
}

void CanSendScheduler::clearAll()
{
	foreach (int key, mJobs.keys()) {
		killTimer(key);
	}
	mJobs.clear();
}
void CanSendScheduler::timerEvent(QTimerEvent* event)
{
	QCanMessage msg = mJobs.value(event->timerId());
	emit jobScheduled(msg);
}

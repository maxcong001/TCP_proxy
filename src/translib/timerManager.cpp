/*
 * Copyright (c) 2016-20017 Max Cong <savagecm@qq.com>
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "translib/timerManager.h"
#include "logger/logger.h"
namespace translib
{
bool TimerManager::init()
{

#if 0
	auto work_fun = std::bind(&TimerManager::do_work, this);
	// actually we want the thread never exit.
	// So we do not kill the thread.
	// start once and leave it there.
	// suppose when the loop stop, the thread will exit then.
	std::thread timer_thread(work_fun);
#endif
	__LOG(debug, "[TimerManager] init timer manager!");
	//start audit timer
	audit_timer = new Timer(_loop);
	audit_timer->startForever(AUDIT_TIMER, this, [&](void *usrdata) {
		((TimerManager *)usrdata)->audit();
	});
	_loop.start(true);
	return true;
}
bool TimerManager::audit()
{
	std::lock_guard<std::mutex> lck(mtx);
	timer_map tmp;
	__LOG(debug, "[TimerManager] there are " << t_map.size() << " timers");

	for_each(t_map.begin(), t_map.end(), [&](std::pair<const int, Timer::ptr_p> &i) {
		__LOG(debug, "[TimerManager] check timer with ID : " << i.first);
		if ((i.second)->isFinished())
		{
			__LOG(debug, "[TimerManager] delete timer, ID is : " << i.first);
		}
		else
		{
			__LOG(debug, "[TimerManager] timer still running  timer, ID is : " << i.first);
			tmp.emplace(i);
		}
	});
	t_map.swap(tmp);
	return true;
}
Timer::ptr_p TimerManager::getTimer(int *timerID)
{
	int tid = getUniqueID();
	Timer::ptr_p tmp_ptr(new Timer(_loop));
	std::lock_guard<std::mutex> lck(mtx);
	t_map.emplace(tid, tmp_ptr);
	if (timerID)
	{
		*timerID = tid;
	}
	return tmp_ptr;
}
// you can call timer.stop or pass in the ID and we will kill the timer
bool TimerManager::killTimer(int timerID)
{
	std::lock_guard<std::mutex> lck(mtx);

	auto iter = t_map.find(timerID);
	if (iter != t_map.end())
	{
		(iter->second)->stop();
	}
	else
	{
		__LOG(debug, "[TimerManager] did not find the timer ID");
	}
	return true;
}

} /* namespace translib */

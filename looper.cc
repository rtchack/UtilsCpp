/**
 * Created by xing on 4/12/16.
 */

#include "base/looper.h"

namespace base{

	void Looper::Post(const Task &tsk, bool flush)
  noexcept {
		{
			std::lock_guard<std::mutex> bar{op_mut};
			if(flush) msg_queue.clear();
			msg_queue.push_back(tsk);
		}
		cv.notify_one();
	}

	void Looper::Deactivate() noexcept {
		PreDeactivate();

    Post([]{return true;}, true);
		{
			std::lock_guard<std::mutex> bar{run_mut};
			if(!running){
				cInf("is not active");
				return;
			}
      worker.Reset();
      running = false;
		}

		PostDeactivate();
	}

	void Looper::Activate() noexcept {
		PreActivate();

		{
			std::lock_guard<std::mutex> bar{run_mut};
			if(running){
				cInf("is already active");
				return;
			}
			worker.Attach(std::thread{&Looper::Entry, this});
      running = true;
		}

		PostActivate();
	}

	void Looper::Entry() noexcept {
		Task tsk;
		loop{
			{
				std::unique_lock<std::mutex> lk{op_mut};
				cv.wait(lk, [this]{return !msg_queue.empty();});
				//if(msg_queue.empty()) continue;
				tsk = msg_queue.front();
				msg_queue.pop_front();
			}

			stat.Succ();

			if(tsk()){
				cInf("message of quiting");
        running = false;
				return;
			}
		}
	}

}

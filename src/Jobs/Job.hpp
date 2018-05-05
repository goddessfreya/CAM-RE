/*
 * Copyright (C) 2018 Hal Gentz
 *
 * This file is part of CAM-RE.
 *
 * CAM-RE is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Bash is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * CAM-RE. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This is a simple job class.
 *
 * Please do not allocate it directly. Instead, request one from your
 * WorkerPool.
 */

#ifndef CAM_JOBS_JOB_HPP
#define CAM_JOBS_JOB_HPP

#include <vector>
#include <atomic>
#include <cstdint>
#include <functional>
#include <new>

#include "../Utils/CountedSharedMutex.hpp"
#include "../Utils/ConditionalContinue.hpp"
#include "JobPool.hpp"
#include "../Utils/Aligner.tpp"
#include "../Utils/Assert.hpp"

namespace CAM
{
namespace Jobs
{
class WorkerPool;
class Job;

struct JobD
{
	using JobFunc = std::function<void(WorkerPool* wp, size_t thread, Job* thisJob)>;

	std::atomic<JobPool*> owner;
	mutable Utils::ConditionalContinue ownerCC;
	mutable Utils::ConditionalContinue depsCC;
	JobFunc job;
	mutable std::atomic<size_t> dependencesIncomplete;

	mutable CAM::Utils::CountedSharedMutex dependsOnMeMutex;
	std::vector<Job*> dependsOnMe;

	bool mainThreadOnly;
};

class Job : private Utils::Aligner<JobD>
{
	public:
	inline Job(JobFunc job, size_t depsOnMe, bool mainThreadOnly) { Reset(job, depsOnMe, mainThreadOnly); }
	inline Job() { }
	inline void Reset(JobFunc job, size_t depsOnMe, bool mainThreadOnly)
	{
		ownerCC.Signal();
		depsCC.Signal();
		while(dependsOnMeMutex.LockersLeft() != 0) {}
		while(dependsOnMeMutex.SharedLockersLeft() != 0) {}
		ownerCC.Reset();
		depsCC.Reset();

		this->job = job;

		std::unique_lock<CAM::Utils::CountedSharedMutex> lock(dependsOnMeMutex);
		dependsOnMe.clear();
		dependsOnMe.reserve(depsOnMe);

		this->mainThreadOnly = mainThreadOnly;

		dependencesIncomplete.store(0, std::memory_order_relaxed);
		owner.store(nullptr, std::memory_order_acq_rel);
	}

	[[nodiscard]] inline std::unique_ptr<Job> DoJob(WorkerPool* wp, size_t thread)
	{
		ASSERT(mainThreadOnly ? thread == 0 : true, "This is a main-thread-only job. Please insure only the main thread attempts to complete it.");

		depsCC.Wait([this] { return CanRun(); } );

		job(wp, thread, this);

		Job* toRun = nullptr;
		auto deps = GetDepsOnMe();
		for (auto& dep : deps.first)
		{
			// TODO: Make it so we go do other jobs then come back.
			dep->ownerCC.Wait([&dep] { return dep->owner.load(std::memory_order_acquire) != nullptr; } );

			size_t val = dep->dependencesIncomplete.load(std::memory_order_acquire);

			while(!atomic_compare_exchange_weak_explicit
			(
				&dep->dependencesIncomplete,
				&val,
				val - 1,
				std::memory_order_acq_rel,
				std::memory_order_relaxed
			)) {}

			dep->depsCC.Signal();

			if (val == 1) // I was last to subtract
			{
				if (toRun != nullptr)
				{
					dep->owner.load(std::memory_order_acquire)->MakeRunnable(dep);
				}
				else
				{
					toRun = dep;
				}
			}
		}

		if (toRun != nullptr)
		{
			ASSERT(toRun->CanRun(), "Sync error, should be runnable.");
			return toRun->owner.load(std::memory_order_acquire)->PullDepJob(toRun);
		}

		return nullptr;
	}

	[[nodiscard]] inline bool CanRun() const
	{
		return dependencesIncomplete.load(std::memory_order_acquire) == 0;
	}

	// TODO: Make events a type of dependency
	inline void DependsOn(Job* other)
	{
		other->DependsOnMe(this);
	}
	inline void DependsOnMe(Job* other)
	{
		if (other == nullptr)
		{
			return;
		}
		other->dependencesIncomplete.fetch_add(1, std::memory_order_release);
		depsCC.Signal();
		std::unique_lock<CAM::Utils::CountedSharedMutex> lock(other->dependsOnMeMutex);
		dependsOnMe.push_back(other);
	}

	inline void SetOwner(JobPool* owner)
	{
		this->owner.store(owner, std::memory_order_release);
		ownerCC.Signal();
	}

	[[nodiscard]] inline size_t NumberOfDepsOnMe() const
	{
		std::shared_lock<CAM::Utils::CountedSharedMutex> lock(dependsOnMeMutex);
		return dependsOnMe.size();
	}
	inline bool MainThreadOnly() const { return mainThreadOnly; }

	inline void SameThingsDependOnMeAs(Job* other)
	{
		auto otherDeps = other->GetDepsOnMe();
		std::unique_lock<CAM::Utils::CountedSharedMutex> lock(dependsOnMeMutex);

		dependsOnMe.reserve(dependsOnMe.size() + otherDeps.first.size());

		for (auto& dep : otherDeps.first)
		{
			DependsOnMe(dep);
		}
	}

	private:
	[[nodiscard]] inline std::pair
	<
		const std::vector<Job*>&,
		std::shared_lock<CAM::Utils::CountedSharedMutex>
	> GetDepsOnMe() const
	{
		return {dependsOnMe, std::shared_lock<CAM::Utils::CountedSharedMutex>(dependsOnMeMutex)};
	}
};
}
}

#endif

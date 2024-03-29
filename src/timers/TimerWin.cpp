/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _WIN32
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>

#include "april.h"
#include "RenderSystem.h"
#include "Timer.h"

namespace april
{
	Timer::Timer()
	{
		this->dt = 0;
		this->td = 0;
		this->td2 = 0;
		this->frequency = 0;
		this->performanceTimerStart = 0;
		this->resolution = 0;
		this->mTimerStart = 0;
		this->mTimerElapsed = 0;
		this->performanceTimerElapsed = 0;
		this->performanceTimer = false;
		
		if (!QueryPerformanceFrequency((LARGE_INTEGER*)&this->frequency))
		{
			hlog::warn(logTag, "Performance timer not available, multimedia timer will be used instead!");
			this->performanceTimer = false;
			this->mTimerStart = htickCount();
			this->resolution = 0.001f;
			this->frequency = 1000;
			this->mTimerElapsed = (unsigned long)this->mTimerStart;
		}
		else
		{
			QueryPerformanceCounter((LARGE_INTEGER*)&this->performanceTimerStart);
			this->performanceTimer = true;
			this->resolution = (float)(1.0 / this->frequency);
			this->performanceTimerElapsed = this->performanceTimerStart;
		}
	}
	
	Timer::~Timer()
	{
		
	}
	
	float Timer::getTime()
	{
		__int64 time;
		if (this->performanceTimer)
		{
			QueryPerformanceCounter((LARGE_INTEGER*)&time);
			return ((float)(time - this->performanceTimerStart) * this->resolution * 1000.0f);
		}
		else
		{
			return ((float)(htickCount() - this->mTimerStart) * this->resolution * 1000.0f);
		}
	}
	
	float Timer::diff(bool update)
	{
		if (update)
		{
			this->update();
		}
		return this->dt;
	}
	
	void Timer::update()
	{
		this->td2 = this->getTime();
		this->dt = (this->td2 - this->td) * 0.001f;
		this->td = this->td2;
	}
	
}
#endif
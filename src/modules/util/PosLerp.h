/**
 * @file
 */

#pragma once

#include "core/Log.h"
#include "core/GLM.h"

namespace util {

class PosLerp {
private:
	long MOVETIME = 200L;
	long _now = 0L;
	glm::vec3 _currentPosition;
	glm::vec3 _lastPosition;
	glm::vec3 _nextPosition;
	long _nextPosTime;
public:
	PosLerp() :
			_currentPosition(0.0f), _lastPosition(0.0f), _nextPosition(0.0f), _nextPosTime(0L) {
	}

	inline const glm::vec3& position() const {
		return _currentPosition;
	}

	inline void setStartPosition(const glm::vec3& position) {
		_lastPosition = position;
		_currentPosition = position;
		_nextPosition = position;
		_nextPosTime = 0l;
	}

	inline void setTargetPosition(const glm::vec3& position) {
		_lastPosition = _currentPosition;
		_nextPosition = position;
		// TODO: there might be a delta between _now and "old" _nextPosTime, take that into account
		_nextPosTime = _now + MOVETIME;
	}

	void update(long dt) {
		_now += dt;
		if (_now < _nextPosTime) {
			const long remaining = _nextPosTime - _now;
			const long passed = MOVETIME - remaining;
			const float lerp = passed / (float) MOVETIME;
			_currentPosition = _lastPosition + ((_nextPosition - _lastPosition) * lerp);
		} else {
			_currentPosition = _nextPosition;
		}
	}
};

}

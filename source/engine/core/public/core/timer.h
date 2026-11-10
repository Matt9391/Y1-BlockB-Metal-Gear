/// @file
/// @brief A stopwatch for measuring how long something takes.

#pragma once

#include <chrono>

namespace buz
{
	//--------------------------------------------------------------
	/// @brief Measures elapsed wall clock time with high precision.
	///
	/// A freshly constructed timer starts measuring immediately:
	/// @code{.cpp}
	/// buz::timer stopwatch;
	/// expensive_work();
	/// buz::log::info("took {} seconds", stopwatch.elapsed_seconds());
	/// @endcode
	class timer
	{
	public:
		//----------------------------------------------------------
		/// @brief Starts measuring on construction.
		timer();

		//----------------------------------------------------------
		/// @brief Restarts the measurement from now.
		void reset();

		//----------------------------------------------------------
		/// @brief Returns the time passed since construction or the last reset.
		/// @return Elapsed time in seconds.
		float elapsed_seconds() const;

	private:
		std::chrono::steady_clock::time_point m_start;
	};
}

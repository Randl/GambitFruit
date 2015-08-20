// posix.cpp

// includes

#include <cerrno>
#include <cstdio> // REMOVE ME?
#include <ctime>

#if defined(_WIN32) || defined(_WIN64)

#  include <windows.h>

#else // assume POSIX
#  include <sys/resource.h>
// #  include <sys/select.h>
#  include <sys/time.h>
#  include <sys/types.h>
#  include <unistd.h>
#endif

#include "posix.h"

// constants

static constexpr bool UseDebug = false;

// prototypes

#if !defined(_WIN32) && !defined(_WIN64)
static double duration (const struct timeval * tv);
#endif

// functions

// input_available()

bool input_available() {

#if defined(_WIN32) || defined(_WIN64)

	static bool init = false, is_pipe;
	static HANDLE stdin_h;
	DWORD val, error;

	// val = 0; // needed to make the compiler happy?

	// have a look at the "local" buffer first, *this time before init (no idea if it helps)*
	if (UseDebug && !init) printf("info string init=%d stdin->_cnt=%d\n", int(init), stdin->_cnt);
	if (stdin->_cnt > 0) return true; // HACK: assumes FILE internals

	// input init (only done once)

	if (!init) {

		init = true;

		stdin_h = GetStdHandle(STD_INPUT_HANDLE);

		if (UseDebug && (stdin_h == nullptr || stdin_h == INVALID_HANDLE_VALUE)) {
			error = GetLastError();
			printf("info string GetStdHandle() failed, error=%lu\n", error);
		}

		is_pipe = !GetConsoleMode(stdin_h, &val); // HACK: assumes pipe on failure

		if (UseDebug) printf("info string init=%d is_pipe=%d\n", int(init), int(is_pipe));

		if (UseDebug && is_pipe) { // GetConsoleMode() failed, everybody assumes pipe then
			error = GetLastError();
			printf("info string GetConsoleMode() failed, error=%lu\n", error);
		}

		if (!is_pipe) {
			SetConsoleMode(stdin_h, val & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
			FlushConsoleInputBuffer(stdin_h); // no idea if we can lose data doing this
		}
	}

	// different polling depending on input type
	// does this code work at all for pipes?

	if (is_pipe) {
		if (!PeekNamedPipe(stdin_h, nullptr, 0, nullptr, &val, nullptr)) {
			if (UseDebug) { // PeekNamedPipe() failed, everybody assumes EOF then
				error = GetLastError();
				printf("info string PeekNamedPipe() failed, error=%lu\n", error);
			}
			return true; // HACK: assumes EOF on failure
		}

		//if (UseDebug && val < 0) printf("info string PeekNamedPipe(): val=%lu\n",val); //DWORD is unsigned

		return val != 0;

	} else {
		GetNumberOfConsoleInputEvents(stdin_h, &val);
		return val > 1; // no idea why 1
	}

	return false;

#else // assume POSIX

	fd_set set[1];

	FD_ZERO(set);
	FD_SET(STDIN_FILENO,set);

	struct timeval time_val[1];
	time_val->tv_sec = 0;
	time_val->tv_usec = 0;

	int_fast32_t val = select(STDIN_FILENO+1,set,nullptr,nullptr,time_val);
	if (val == -1 && errno != EINTR)
		my_fatal("input_available(): select(): %s\n",strerror(errno));

	return val > 0;

#endif
}

// now_real()

double now_real() {

#if defined(_WIN32) || defined(_WIN64)
	return double(GetTickCount()) / 1000.0;
#else // assume POSIX

	struct timeval tv[1];
	struct timezone tz[1];

	tz->tz_minuteswest = 0;
	tz->tz_dsttime = 0; // DST_NONE not declared in linux

	if (gettimeofday(tv,tz) == -1) // tz needed at all?
		my_fatal("now_real(): gettimeofday(): %s\n",strerror(errno));

	return duration(tv);

#endif
}

// now_cpu()

double now_cpu() {

#if defined(_WIN32) || defined(_WIN64)
	return double(clock()) / double(CLOCKS_PER_SEC); // OK if CLOCKS_PER_SEC is small enough
#else // assume POSIX

	struct rusage ru[1];

	if (getrusage(RUSAGE_SELF,ru) == -1)
		my_fatal("now_cpu(): getrusage(): %s\n",strerror(errno));

	return duration(&ru->ru_utime);

#endif
}

// duration()

#if !defined(_WIN32) && !defined(_WIN64)

static double duration(const struct timeval * tv) {
	ASSERT(tv!=nullptr);
	return double(tv->tv_sec) + double(tv->tv_usec) * 1E-6;
}

#endif

// end of posix.cpp

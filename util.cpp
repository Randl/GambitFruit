
// util.cpp

// includes

#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "posix.h"
#include "util.h"

// functions

// util_init()

void util_init() {

   setvbuf(stdin,nullptr,_IONBF,0);
   setvbuf(stdout,nullptr,_IONBF,0); // _IOLBF breaks on Windows!
}

// my_random_init()

void my_random_init() {

   srand(time(nullptr));
}

// my_random()

int_fast32_t my_random(int_fast32_t n) {

   double r;

   ASSERT(n>0);

   r = double(rand()) / (double(RAND_MAX) + 1.0);

   return int(floor(r*double(n)));
}

// my_atoll()

int_fast64_t my_atoll(const char string[]) {

   int_fast64_t n;

   sscanf(string,S64_FORMAT,&n);

   return n;
}

// my_round()

int_fast32_t my_round(double x) {

   return int(floor(x+0.5));
}

// my_malloc()

void * my_malloc(int_fast32_t size) {

   void * address;

   ASSERT(size>0);

   address = malloc(size);
   if (address == nullptr) my_fatal("my_malloc(): malloc(): %s\n",strerror(errno));

   return address;
}

// my_free()

void my_free(void * address) {

   ASSERT(address!=nullptr);

   free(address);
}

// my_fatal()

void my_fatal(const char format[], ...) {

   va_list ap;

   ASSERT(format!=nullptr);

   va_start(ap,format);
   vfprintf(stderr,format,ap);
   va_end(ap);

   exit(EXIT_FAILURE);
   // abort();
}

// my_file_read_line()

bool my_file_read_line(FILE * file, char string[], int_fast32_t size) {

   char * ptr;

   ASSERT(file!=nullptr);
   ASSERT(string!=nullptr);
   ASSERT(size>0);

   if (fgets(string,size,file) == nullptr) {
      if (feof(file)) {
         return false;
      } else { // error
         my_fatal("my_file_read_line(): fgets(): %s\n",strerror(errno));
      }
   }

   // suppress '\n'

   ptr = strchr(string,'\n');
   if (ptr != nullptr) *ptr = '\0';

   return true;
}

// my_string_empty()

bool my_string_empty(const char string[]) {

   return string == nullptr || string[0] == '\0';
}

// my_string_equal()

bool my_string_equal(const char string_1[], const char string_2[]) {

   int_fast32_t c1, c2;

   ASSERT(string_1!=nullptr);
   ASSERT(string_2!=nullptr);

   while (true) {

      c1 = *string_1++;
      c2 = *string_2++;

      if (tolower(c1) != tolower(c2)) return false;
      if (c1 == '\0') return true;
   }

   return false;
}

// my_strdup()

char * my_strdup(const char string[]) {

   char * address;

   ASSERT(string!=nullptr);

   // strdup() is not ANSI C

   address = (char *) my_malloc(strlen(string)+1);
   strcpy(address,string);

   return address;
}

// my_string_clear()

void my_string_clear(const char * * variable) {

   ASSERT(variable!=nullptr);

   if (*variable != nullptr) {
      my_free((void*)(*variable));
      *variable = nullptr;
   }
}

// my_string_set()

void my_string_set(const char * * variable, const char string[]) {

   ASSERT(variable!=nullptr);
   ASSERT(string!=nullptr);

   if (*variable != nullptr) my_free((void*)(*variable));
   *variable = my_strdup(string);
}

// my_timer_reset()

void my_timer_reset(my_timer_t * timer) {

   ASSERT(timer!=nullptr);

   timer->start_real = 0.0;
   timer->start_cpu = 0.0;
   timer->elapsed_real = 0.0;
   timer->elapsed_cpu = 0.0;
   timer->running = false;
}

// my_timer_start()

void my_timer_start(my_timer_t * timer) {

   ASSERT(timer!=nullptr);

   ASSERT(timer->start_real==0.0);
   ASSERT(timer->start_cpu==0.0);
   ASSERT(!timer->running);

   timer->running = true;
   timer->start_real = now_real();
   timer->start_cpu = now_cpu();
}

// my_timer_stop()

void my_timer_stop(my_timer_t * timer) {

   ASSERT(timer!=nullptr);

   ASSERT(timer->running);

   timer->elapsed_real += now_real() - timer->start_real;
   timer->elapsed_cpu += now_cpu() - timer->start_cpu;
   timer->start_real = 0.0;
   timer->start_cpu = 0.0;
   timer->running = false;
}

// my_timer_elapsed_real()

double my_timer_elapsed_real(const my_timer_t * timer) {

   double elapsed;

   ASSERT(timer!=nullptr);

   elapsed = timer->elapsed_real;
   if (timer->running) elapsed += now_real() - timer->start_real;

   if (elapsed < 0.0) elapsed = 0.0;

   return elapsed;
}

// my_timer_elapsed_cpu()

double my_timer_elapsed_cpu(const my_timer_t * timer) {

   double elapsed;

   ASSERT(timer!=nullptr);

   elapsed = timer->elapsed_cpu;
   if (timer->running) elapsed += now_cpu() - timer->start_cpu;

   if (elapsed < 0.0) elapsed = 0.0;

   return elapsed;
}

// my_timer_cpu_usage()

double my_timer_cpu_usage(const my_timer_t * timer) {

   double real, cpu;
   double usage;

   ASSERT(timer!=nullptr);

   real = my_timer_elapsed_real(timer);
   cpu = my_timer_elapsed_cpu(timer);

   if (real <= 0.0 || cpu <= 0.0) return 0.0;

   usage = cpu / real;
   if (usage >= 1.0) usage = 1.0;

   return usage;
}

// end of util.cpp


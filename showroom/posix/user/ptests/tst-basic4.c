/* Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include <rtai_posix.h>

static void
final_test (void)
{
  puts ("final_test has been called");

#define THE_SIGNAL SIGUSR1
  kill (getpid (), SIGUSR1);
}


static void *
tf (void *a)
{
  pthread_setschedparam_np(0, SCHED_FIFO, 0, 0xF, PTHREAD_HARD_REAL_TIME);
  pid_t pid = fork ();
  if (pid == -1)
    {
      puts ("fork failed");
      exit (1);
    }

  if (pid == 0)
    {
  pthread_setschedparam_np(0, SCHED_FIFO, 0, 0xF, PTHREAD_HARD_REAL_TIME);
      atexit (final_test);

      pthread_exit (NULL);
    }

  int r;
  int e = TEMP_FAILURE_RETRY (waitpid (pid, &r, 0));
  if (e != pid)
    {
      puts ("waitpid failed");
      exit (1);
    }

  if (! WIFSIGNALED (r))
    {
      puts ("child not signled");
      exit (1);
    }

  if (WTERMSIG (r) != THE_SIGNAL)
    {
      puts ("child's termination signal wrong");
      exit (1);
    }

  return NULL;
}


int
do_test (void)
{
  pthread_t th;

  if (pthread_create (&th, NULL, tf, NULL) != 0)
    {
      puts ("create failed");
      _exit (1);
    }

  if (pthread_join (th, NULL) != 0)
    {
      puts ("join failed");
      exit (1);
    }

  return 0;
}

int main(void)
{
        pthread_setschedparam_np(0, SCHED_FIFO, 0, 0xF, PTHREAD_HARD_REAL_TIME);
        start_rt_timer(0);
        do_test();
        return 0;
}

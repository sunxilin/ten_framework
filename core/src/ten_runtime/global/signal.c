//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#include "include_internal/ten_runtime/global/signal.h"

#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "include_internal/ten_runtime/app/close.h"
#include "ten_runtime/app/app.h"
#include "ten_runtime/common/errno.h"
#include "ten_runtime/global/global.h"
#include "ten_utils/container/list.h"
#include "ten_utils/container/list_node_ptr.h"
#include "ten_utils/log/log.h"
#include "ten_utils/macro/check.h"
#include "ten_utils/macro/memory.h"
#include "ten_utils/sanitizer/memory_check.h"

#if !defined(OS_WINDOWS)

// Global variable to count SIGINT signals.
static volatile sig_atomic_t sigint_count = 0;

// See https://github.com/joyent/libuv/issues/1254.
static void ten_global_ignore_sigpipe(void) {
  struct sigaction act;
  memset(&act, 0, sizeof act);

  act.sa_flags = 0;
  act.sa_handler = SIG_IGN;
  sigemptyset(&act.sa_mask);

  if (sigaction(SIGPIPE, &act, NULL) < 0) {
    TEN_LOGF("Failed to ignore SIGPIPE.");
    TEN_ASSERT(0, "Should not happen.");
    exit(EXIT_FAILURE);
  }
}

static void ten_global_signal_handler(int signo, TEN_UNUSED siginfo_t *info,
                                      TEN_UNUSED void *context) {
  if (signo == SIGINT || signo == SIGTERM) {
    TEN_LOGW("Received SIGINT/SIGTERM.");

    ten_mutex_lock(g_apps_mutex);

    ten_list_foreach (&g_apps, iter) {
      ten_app_t *app = ten_ptr_listnode_get(iter.node);
      TEN_ASSERT(app, "Invalid argument.");

      ten_app_close(app, NULL);
    }

    ten_mutex_unlock(g_apps_mutex);

    sigint_count++;
    if (sigint_count >= 2) {
      TEN_LOGW("Received SIGINT/SIGTERM twice, exit directly.");
      exit(EXIT_FAILURE);
    }
  }

  if (signo == SIGUSR1) {
    TEN_LOGW("Received SIGUSR1.");
    ten_sanitizer_memory_record_dump();
  }
}

// The alternate stack size.
  #define ALT_STACK_SIZE (unsigned long)(1024 * 1024)

void *g_alt_stack = NULL;

void ten_global_signal_alt_stack_create(void) {
  TEN_ASSERT(!g_alt_stack, "Should not happen.");

  g_alt_stack = ten_malloc(ALT_STACK_SIZE);
}

void ten_global_signal_alt_stack_destroy(void) {
  TEN_ASSERT(g_alt_stack, "Should not happen.");

  TEN_FREE(g_alt_stack);
}

// Setup relevant stuffs for signal handling.
static void ten_global_setup_sig_handler(void) {
  struct sigaction act;
  memset(&act, 0, sizeof act);

  act.sa_flags = SA_SIGINFO;

  stack_t ss;
  ss.ss_sp = g_alt_stack;
  if (ss.ss_sp == NULL) {
    exit(-TEN_ERRNO_GENERIC);
  }
  ss.ss_size = ALT_STACK_SIZE;
  ss.ss_flags = 0;
  if (sigaltstack(&ss, NULL) == -1) {
    perror("sigaltstack");
    exit(-TEN_ERRNO_GENERIC);
  }

  // If the app process runs on a GO runtime, the `SA_ONSTACK` flag must be set
  // to make sure that the stack for the handler is big enough. In GO app, the
  // signal handler is not always called from a system stack (the g0 stack for
  // each native thread, which is big enough for C functions), maybe from GC
  // goroutine whose stack is 2K by default.
  act.sa_flags |= SA_ONSTACK;

  act.sa_sigaction = ten_global_signal_handler;

  if (0 != sigaction(SIGINT, &act, NULL)) {
    TEN_LOGF("Failed to install SIGINT handler.");
    exit(-TEN_ERRNO_GENERIC);
  }

  if (0 != sigaction(SIGTERM, &act, NULL)) {
    TEN_LOGF("Failed to install SIGTERM handler.");
    exit(-TEN_ERRNO_GENERIC);
  }

  if (0 != sigaction(SIGUSR1, &act, NULL)) {
    TEN_LOGF("Failed to install SIGUSR1 handler.");
    exit(-TEN_ERRNO_GENERIC);
  }
}

void ten_global_setup_signal_stuff(void) {
  const char *disable_signal_trap = getenv("TEN_DISABLE_SIGNAL_TRAP");
  if (disable_signal_trap && !strcmp(disable_signal_trap, "true")) {
    // No trap signal, for nodejs / python / java bindings
  } else {
    ten_global_ignore_sigpipe();
    ten_global_setup_sig_handler();
  }
}

#else

void ten_global_setup_signal_stuff(void) {}

void ten_global_signal_alt_stack_create(void) {}

void ten_global_signal_alt_stack_destroy(void) {}

#endif

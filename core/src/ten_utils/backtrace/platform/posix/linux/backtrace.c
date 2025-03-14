//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#include "ten_utils/backtrace/backtrace.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ten_utils/backtrace/common.h"
#include "ten_utils/backtrace/platform/posix/internal.h"
#include "ten_utils/lib/alloc.h"

ten_backtrace_t *ten_backtrace_create(void) {
  ten_backtrace_posix_t *self =
      ten_malloc_without_backtrace(sizeof(ten_backtrace_posix_t));
  assert(self && "Failed to allocate memory.");

  ten_backtrace_common_init(&self->common, ten_backtrace_default_dump_cb,
                            ten_backtrace_default_error_cb);

  self->get_file_line = NULL;
  self->get_file_line_data = NULL;
  self->get_syminfo = NULL;
  self->get_syminfo_data = NULL;
  self->file_line_init_failed = 0;

  return (ten_backtrace_t *)self;
}

void ten_backtrace_destroy(ten_backtrace_t *self) {
  assert(self && "Invalid argument.");

  ten_backtrace_common_deinit(self);

  ten_free_without_backtrace(self);
}

void ten_backtrace_dump(ten_backtrace_t *self, size_t skip) {
  assert(self && "Invalid argument.");
  ten_backtrace_dump_posix(self, skip);
}

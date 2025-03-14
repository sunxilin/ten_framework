//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#pragma once

#ifndef OFFSET_OF
  #define OFFSET_OF(x, y) ((char *)(&(((x *)0)->y)))
#endif

#ifndef MEMBER_SIZE
  #define MEMBER_SIZE(type, member) sizeof(((type *)0)->member)
#endif

#ifndef CONTAINER_OF_FROM_OFFSET
  #define CONTAINER_OF_FROM_OFFSET(field_ptr, offset) \
    ((void *)((char *)(field_ptr) - (offset)))
#endif

#ifndef CONTAINER_OF_FROM_FIELD
  #define CONTAINER_OF_FROM_FIELD(field_ptr, base_type, field_name) \
    /* NOLINTNEXTLINE(performance-no-int-to-ptr) */                 \
    CONTAINER_OF_FROM_OFFSET(field_ptr, OFFSET_OF(base_type, field_name))
#endif

#ifndef FIELD_OF_FROM_OFFSET
  #define FIELD_OF_FROM_OFFSET(base_ptr, offset) \
    ((void *)((char *)(base_ptr) + (offset)))
#endif

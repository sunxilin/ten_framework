//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "common.h"

#define TEN_GO_NO_RESPONSE_HANDLER 0
#define TEN_GO_TEN_SIGNATURE 0xCCCC1DD4BB4CA743U

typedef struct ten_env_t ten_env_t;
typedef struct ten_go_extension_t ten_go_extension_t;
typedef struct ten_go_metadata_t ten_go_metadata_t;
typedef struct ten_go_msg_t ten_go_msg_t;
typedef struct ten_go_ten_env_t ten_go_ten_env_t;
typedef struct ten_go_value_t ten_go_value_t;

void ten_go_ten_env_on_init_done(uintptr_t bridge_addr);

void ten_go_ten_env_on_start_done(uintptr_t bridge_addr);

void ten_go_ten_env_on_stop_done(uintptr_t bridge_addr);

void ten_go_ten_env_on_deinit_done(uintptr_t bridge_addr);

void ten_go_ten_env_on_create_extensions_done(
    uintptr_t bridge_addr, const void *extension_bridge_array, int size);

void ten_go_ten_env_on_destroy_extensions_done(uintptr_t bridge_addr);

void ten_go_ten_env_on_create_instance_done(uintptr_t bridge_addr,
                                            bool is_extension,
                                            uintptr_t instance_bridge_addr,
                                            uintptr_t context_addr);

bool ten_go_ten_env_set_property_async(uintptr_t bridge_addr, const char *path,
                                       ten_go_value_t *value,
                                       ten_go_handle_t callback_id);

bool ten_go_ten_env_get_property_async(uintptr_t bridge_addr, const char *path,
                                       ten_go_handle_t callback_id);

ten_go_status_t ten_go_ten_env_return_result(uintptr_t bridge_addr,
                                             uintptr_t cmd_result_bridge_addr,
                                             uintptr_t cmd_bridge_addr);

ten_go_status_t ten_go_ten_env_return_result_directly(
    uintptr_t bridge_addr, uintptr_t cmd_result_bridge_addr);

ten_go_status_t ten_go_ten_env_return_json_bytes(uintptr_t bridge_addr,
                                                 int status_code,
                                                 const void *detail,
                                                 int detail_len,
                                                 uintptr_t cmd_bridge_addr);

ten_go_status_t ten_go_ten_env_return_string(uintptr_t bridge_addr,
                                             int status_code,
                                             const void *detail, int detail_len,
                                             uintptr_t cmd_bridge_addr);

ten_go_status_t ten_go_ten_env_return_bool(uintptr_t bridge_addr,
                                           int status_code, bool detail,
                                           uintptr_t cmd_bridge_addr);

ten_go_status_t ten_go_ten_env_return_int8(uintptr_t bridge_addr,
                                           int status_code, int8_t detail,
                                           uintptr_t cmd_bridge_addr);

ten_go_status_t ten_go_ten_env_return_int16(uintptr_t bridge_addr,
                                            int status_code, int16_t detail,
                                            uintptr_t cmd_bridge_addr);

ten_go_status_t ten_go_ten_env_return_int32(uintptr_t bridge_addr,
                                            int status_code, int32_t detail,
                                            uintptr_t cmd_bridge_addr);

ten_go_status_t ten_go_ten_env_return_int64(uintptr_t bridge_addr,
                                            int status_code, int64_t detail,
                                            uintptr_t cmd_bridge_addr);

ten_go_status_t ten_go_ten_env_return_uint8(uintptr_t bridge_addr,
                                            int status_code, uint8_t detail,
                                            uintptr_t cmd_bridge_addr);

ten_go_status_t ten_go_ten_env_return_uint16(uintptr_t bridge_addr,
                                             int status_code, uint16_t detail,
                                             uintptr_t cmd_bridge_addr);

ten_go_status_t ten_go_ten_env_return_uint32(uintptr_t bridge_addr,
                                             int status_code, uint32_t detail,
                                             uintptr_t cmd_bridge_addr);

ten_go_status_t ten_go_ten_env_return_uint64(uintptr_t bridge_addr,
                                             int status_code, uint64_t detail,
                                             uintptr_t cmd_bridge_addr);

ten_go_status_t ten_go_ten_env_return_float32(uintptr_t bridge_addr,
                                              int status_code, float detail,
                                              uintptr_t cmd_bridge_addr);

ten_go_status_t ten_go_ten_env_return_float64(uintptr_t bridge_addr,
                                              int status_code, double detail,
                                              uintptr_t cmd_bridge_addr);

ten_go_status_t ten_go_ten_env_return_buf(uintptr_t bridge_addr,
                                          int status_code, void *detail,
                                          int detail_len,
                                          uintptr_t cmd_bridge_addr);

/**
 * @param json_bytes The pointer to the underlying buffer of the GO string or
 * slice, which is passed with unsafe.Pointer in GO world, so the type of @a
 * json_bytes is void*, not char*. Only the read operation is permitted. And the
 * buffer is managed by GO, do not read it after the blocking cgo call.
 *
 * @param json_bytes_len The length of @a json_bytes.
 *
 * @param handler_id The index pointing to the callback function in the handle
 * map in GO world. Note that the value `0` represents the callback is NULL.
 */
ten_go_status_t ten_go_ten_env_send_json(uintptr_t bridge_addr,
                                         const void *json_bytes,
                                         int json_bytes_len,
                                         ten_go_handle_t handler_id);

ten_go_status_t ten_go_ten_env_send_cmd(uintptr_t bridge_addr,
                                        uintptr_t cmd_bridge_addr,
                                        ten_go_handle_t handler_id);

ten_go_status_t ten_go_ten_env_send_data(uintptr_t bridge_addr,
                                         uintptr_t data_bridge_addr);

ten_go_status_t ten_go_ten_env_send_video_frame(
    uintptr_t bridge_addr, uintptr_t video_frame_bridge_addr);

bool ten_go_ten_env_send_audio_frame(uintptr_t bridge_addr,
                                     uintptr_t audio_frame_bridge_addr);

bool ten_go_ten_env_is_cmd_connected(uintptr_t bridge_addr,
                                     const char *cmd_name);

bool ten_go_ten_env_addon_create_extension(uintptr_t bridge_addr,
                                           const char *addon_name,
                                           const char *instance_name,
                                           ten_go_handle_t callback);

void ten_go_ten_env_addon_destroy_extension(uintptr_t bridge_addr,
                                            uintptr_t extension_bridge_addr,
                                            ten_go_handle_t callback);

void ten_go_ten_env_finalize(uintptr_t bridge_addr);

const char *ten_go_ten_env_debug_info(uintptr_t bridge_addr);

// In general, there are two ways to get a property from a ten_env_t instance.
//
// - One is getting property based on an explicit type, ex:
//   ten_go_ten_property_get_int8. But if the type is string or buffer, the
//   property size must be known first, in other words, we need to fall back to
//   the second way even through we know the property type. That's why we do not
//   provide a function such as `ten_go_ten_property_get_string`.
//
// - Another is the property type is unknown, or the property size must be known
//   first. In this case, the `ten_go_ten_property_get_type_and_size` function
//   will be called first. And as the ten_value_t is cloned during
//   `ten_go_ten_property_get_type_and_size`, so next we can get the property
//   from the preflight ten_value_t.

/**
 * @brief Get the type and size of a property of the ten_env_t. This function is
 * called before getting property from the GO side, as the GO side needs to do
 * some preparation if the property is not a primitive type. Ex: the memory
 * should be allocated from the GO side, if the property is a buffer.
 *
 * @param path The underlying buffer of the GO string, which is passed with
 * unsafe.Pointer in GO world, so the type of @a path is void*, not char*. Only
 * the read operation is permitted. And the buffer is managed by GO, do not read
 * it after the blocking cgo call.
 *
 * @param path_len The length of the underlying buffer.
 *
 * @param type The output for the type of the property.
 *
 * @param size The output for the size of the property. The size is only used if
 * the property is not a primitive type.
 *
 * @param value_addr The output for the ten_value_t* of the property.
 */
ten_go_status_t ten_go_ten_env_property_get_type_and_size(
    uintptr_t bridge_addr, const void *path, int path_len, uint8_t *type,
    uintptr_t *size, uintptr_t *value_addr);

/**
 * @brief Get the value of a property as int8 from the ten_env_t.
 *
 * @param path The underlying buffer of the GO string, which is passed with
 * unsafe.Pointer in GO world, so the type of @a path is void*, not char*. Only
 * the read operation is permitted. And the buffer is managed by GO, do not read
 * it after the blocking cgo call.
 *
 * @param path_len The length of the underlying buffer.
 *
 * @param value The output for the value of the property.
 */
ten_go_status_t ten_go_ten_env_property_get_int8(uintptr_t bridge_addr,
                                                 const void *path, int path_len,
                                                 int8_t *value);

ten_go_status_t ten_go_ten_env_property_get_int16(uintptr_t bridge_addr,
                                                  const void *path,
                                                  int path_len, int16_t *value);

ten_go_status_t ten_go_ten_env_property_get_int32(uintptr_t bridge_addr,
                                                  const void *path,
                                                  int path_len, int32_t *value);

ten_go_status_t ten_go_ten_env_property_get_int64(uintptr_t bridge_addr,
                                                  const void *path,
                                                  int path_len, int64_t *value);

ten_go_status_t ten_go_ten_env_property_get_uint8(uintptr_t bridge_addr,
                                                  const void *path,
                                                  int path_len, uint8_t *value);

ten_go_status_t ten_go_ten_env_property_get_uint16(uintptr_t bridge_addr,
                                                   const void *path,
                                                   int path_len,
                                                   uint16_t *value);

ten_go_status_t ten_go_ten_env_property_get_uint32(uintptr_t bridge_addr,
                                                   const void *path,
                                                   int path_len,
                                                   uint32_t *value);

ten_go_status_t ten_go_ten_env_property_get_uint64(uintptr_t bridge_addr,
                                                   const void *path,
                                                   int path_len,
                                                   uint64_t *value);

ten_go_status_t ten_go_ten_env_property_get_float32(uintptr_t bridge_addr,
                                                    const void *path,
                                                    int path_len, float *value);

ten_go_status_t ten_go_ten_env_property_get_float64(uintptr_t bridge_addr,
                                                    const void *path,
                                                    int path_len,
                                                    double *value);

ten_go_status_t ten_go_ten_env_property_get_bool(uintptr_t bridge_addr,
                                                 const void *path, int path_len,
                                                 bool *value);

ten_go_status_t ten_go_ten_env_property_get_ptr(uintptr_t bridge_addr,
                                                const void *path, int path_len,
                                                ten_go_handle_t *value);

ten_go_status_t ten_go_ten_env_property_set_bool(uintptr_t bridge_addr,
                                                 const void *path, int path_len,
                                                 bool value);

ten_go_status_t ten_go_ten_env_property_set_int8(uintptr_t bridge_addr,
                                                 const void *path, int path_len,
                                                 int8_t value);

ten_go_status_t ten_go_ten_env_property_set_int16(uintptr_t bridge_addr,
                                                  const void *path,
                                                  int path_len, int16_t value);

ten_go_status_t ten_go_ten_env_property_set_int32(uintptr_t bridge_addr,
                                                  const void *path,
                                                  int path_len, int32_t value);

ten_go_status_t ten_go_ten_env_property_set_int64(uintptr_t bridge_addr,
                                                  const void *path,
                                                  int path_len, int64_t value);

ten_go_status_t ten_go_ten_env_property_set_uint8(uintptr_t bridge_addr,
                                                  const void *path,
                                                  int path_len, uint8_t value);

ten_go_status_t ten_go_ten_env_property_set_uint16(uintptr_t bridge_addr,
                                                   const void *path,
                                                   int path_len,
                                                   uint16_t value);

ten_go_status_t ten_go_ten_env_property_set_uint32(uintptr_t bridge_addr,
                                                   const void *path,
                                                   int path_len,
                                                   uint32_t value);

ten_go_status_t ten_go_ten_env_property_set_uint64(uintptr_t bridge_addr,
                                                   const void *path,
                                                   int path_len,
                                                   uint64_t value);

ten_go_status_t ten_go_ten_env_property_set_float32(uintptr_t bridge_addr,
                                                    const void *path,
                                                    int path_len, float value);

ten_go_status_t ten_go_ten_env_property_set_float64(uintptr_t bridge_addr,
                                                    const void *path,
                                                    int path_len, double value);

ten_go_status_t ten_go_ten_env_property_set_string(uintptr_t bridge_addr,
                                                   const void *path,
                                                   int path_len,
                                                   const void *value,
                                                   int value_len);

ten_go_status_t ten_go_ten_env_property_set_buf(uintptr_t bridge_addr,
                                                const void *path, int path_len,
                                                void *value, int value_len);

ten_go_status_t ten_go_ten_env_property_set_ptr(uintptr_t bridge_addr,
                                                const void *path, int path_len,
                                                ten_go_handle_t value);

/**
 * @brief Get the json string and the size of the json string of a property.
 *
 * @param bridge_addr The bit pattern of the pointer to the struct
 * ten_go_ten_env_t.
 * @param json_str_len The output for the size of the json string.
 * @param json_str A double pointer to the json string.
 *
 * A GO slice will be created with a capacity of @a json_str_len in GO world,
 * and @a json_str will be copied to the slice and destroy using
 * ten_go_copy_c_str_to_slice_and_free.
 */
ten_go_status_t ten_go_ten_env_property_get_json_and_size(
    uintptr_t bridge_addr, const void *path, int path_len,
    uintptr_t *json_str_len, const char **json_str);

/**
 * @brief Parse @a json_str as a json object and set it as a property.
 *
 * @param json_str The pointer to the underlying array of the GO slice. The data
 * it points to must be a valid json data.
 * @param json_str_len The length of the underlying array of the GO slice.
 */
ten_go_status_t ten_go_ten_env_property_set_json_bytes(uintptr_t bridge_addr,
                                                       const void *path,
                                                       int path_len,
                                                       const void *json_str,
                                                       int json_str_len);

ten_go_status_t ten_go_ten_env_init_property_from_json_bytes(
    uintptr_t bridge_addr, const void *json_str, int json_str_len);

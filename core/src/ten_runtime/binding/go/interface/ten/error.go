//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//

package ten

//#include "common.h"
import "C"

import (
	"fmt"
	"unsafe"
)

// TenError is the standard error returned to user from the golang binding. It
// shall be exported, as the callers will determine whether an error is
// TenError.
type TenError struct {
	errno  uint32
	errMsg string
}

// newTenError constructor of ApiError. Note that the ApiError is always created
// from the golang binding, so it's unexported.
func newTenError(errno uint32, errMsg string) error {
	return &TenError{
		errno:  errno,
		errMsg: errMsg,
	}
}

// withGoStatus creates an TenError based on the api status from C. Note that
// the `err_msg` in `status` will be freed after this function, do not access it
// again.
func withGoStatus(status *C.ten_go_status_t) error {
	if status.errno == 0 {
		return nil
	}

	if status.msg_size == 0 {
		return &TenError{
			errno: uint32(status.errno),
		}
	}

	// It's crucial to free the memory allocated in the C environment to prevent
	// memory leaks. Since C.GoString creates a copy of the memory content, it
	// is safe to release the 'err_msg' memory in C after its use.
	defer C.free(unsafe.Pointer(status.err_msg))

	return &TenError{
		errno:  uint32(status.errno),
		errMsg: C.GoString(status.err_msg),
	}
}

func (e *TenError) Error() string {
	return fmt.Sprintf("err_no: %d, err_msg: %s", e.errno, e.errMsg)
}

// Errno returns the inner error number.
func (e *TenError) Errno() uint32 {
	return e.errno
}

// ErrMsg returns the inner error message.
func (e *TenError) ErrMsg() string {
	return e.errMsg
}

// These definitions need to be the same as the TEN_ERRNO enum in C.
//
// Note: To achieve the best compatibility, any new enum item, should be added
// to the end to avoid changing the value of previous enum items.
const (
	// ErrnoGeneric is the default errno, for those users only care error
	// msgs.
	ErrnoGeneric = 1

	// ErrnoInvalidJSON means the json data is invalid.
	ErrnoInvalidJSON = 2

	// ErrnoInvalidArgument means invalid parameter.
	ErrnoInvalidArgument = 3

	// ErrnoInvalidType means unsupported property type.
	ErrnoInvalidType = 4
)

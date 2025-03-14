//
// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
//

package ten

//#include "cmd.h"
import "C"

// StatusCode is an alias of TEN_STATUS_CODE from TEN runtime.
type StatusCode int8

// StatusCode values. These definitions need to be the same as the
// TEN_STATUS_CODE enum in C.
//
// Note: To achieve the best compatibility, any new enum item, should be added
// to the end to avoid changing the value of previous enum items.
const (
	statusCodeInvalid StatusCode = -1

	// StatusCodeOk is an alias of TEN_STATUS_CODE_OK from TEN runtime.
	StatusCodeOk = 0

	// StatusCodeError is an alias of TEN_STATUS_CODE_ERROR from TEN runtime.
	StatusCodeError = 1

	statusCodeLast = 2
)

func (s StatusCode) valid() bool {
	return s > statusCodeInvalid && s < statusCodeLast
}

// CmdResult is the interface for the cmd result.
type CmdResult interface {
	CmdBase
	GetStatusCode() (StatusCode, error)
}

type cmdResult struct {
	*cmd
}

// NewCmdResult creates a new cmd result.
func NewCmdResult(statusCode StatusCode) (CmdResult, error) {
	res := globalPool.process(func() any {
		cmdBridge := C.ten_go_cmd_create_cmd_result(
			C.int(statusCode),
		)
		cmdInstance := newCmdResult(cmdBridge)
		return cmdInstance
	})
	switch res := res.(type) {
	case error:
		return nil, res
	case *cmdResult:
		return res, nil
	default:
		panic("Should not happen.")
	}
}

var _ CmdResult = new(cmdResult)

func newCmdResult(bridge C.uintptr_t) *cmdResult {
	cs := &cmdResult{cmd: newCmd(bridge)}

	return cs
}

func (p *cmdResult) GetStatusCode() (StatusCode, error) {
	return globalPool.process(func() any {
		defer p.keepAlive()

		return (StatusCode)(C.ten_go_cmd_result_get_status_code(p.cPtr))
	}).(StatusCode), nil
}

//export tenGoCreateCmdResult
func tenGoCreateCmdResult(bridge C.uintptr_t) C.uintptr_t {
	cmdStatusInstance := newCmdResult(bridge)

	id := newhandle(cmdStatusInstance)
	cmdStatusInstance.goObjID = id

	return C.uintptr_t(id)
}

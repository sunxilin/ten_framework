// This file is part of the TEN Framework project.
// See https://github.com/TEN-framework/ten_framework/LICENSE for license
// information.
package main

import (
	"fmt"
	"time"

	"ten_framework/ten"
)

type defaultApp struct {
	ten.DefaultApp
}

func (p *defaultApp) OnDeinit(tenEnv ten.TenEnv) {
	fmt.Println("DefaultApp onDeinit")

	tenEnv.OnDeinitDone()
}

func appRoutine(app ten.App, stopped chan<- struct{}) {
	app.Run(false)
	stopped <- struct{}{}
}

func appTimeout(app ten.App, timeout time.Duration) {
	time.Sleep(timeout)
	app.Close()
}

func main() {
	// test app
	app, err := ten.NewApp(&defaultApp{})
	if err != nil {
		fmt.Println("failed to create app.")
	}

	stopped := make(chan struct{}, 1)
	go appRoutine(app, stopped)
	go appTimeout(app, time.Duration(5*time.Second))
	<-stopped

	fmt.Println("ten leak obj Size:", ten.LeakObjSize())
}

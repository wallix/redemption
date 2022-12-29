# -*- coding: utf-8 -*-
#
# Copyright (c) 2020 WALLIX, SAS. All rights reserved.
# Licensed computer software. Property of WALLIX.
# Author(s) : Meng Tan <mtan@wallix.com>
# URL: $URL$
# Module description:
# Compile with:
#

import time

from typing import Optional, Dict


STEPS = {
    "PRIMARY_CONN": "primary_connection",
    "PRIMARY_AUTH": "primary_authentication",
    "FETCH_RIGHTS": "fetch_rights",
    "CHECKOUT_TARGET": "checkout_target",
    "TARGET_CONN": "target_connection",
}


DEBUG = False


def set_time(func):
    def wrapper(*args, **kwargs):
        current_time = kwargs.get('current_time')
        if current_time is None:
            kwargs['current_time'] = time.time()
        return func(*args, **kwargs)
    return wrapper


def get_time(current_time: Optional[float] = None) -> float:
    if current_time is None:
        return time.time()
    return current_time


if DEBUG:
    def print_debug(msg):
        print(msg)
else:
    def print_debug(msg):
        pass


class Logtime:
    saved_times: Dict[str, float]
    current_step: Optional[str]
    last_time: float
    step_time: float
    paused: bool

    def __init__(self):
        self.saved_times = {}
        self._reset()

    def _reset(self):
        self.current_step = None
        self.last_time = 0
        self.step_time = 0
        self.paused = False

    @set_time
    def _add_time(self, current_time: float) -> None:
        diff_time = (current_time - self.last_time)
        print_debug(f">>>>> Time ADDED {diff_time} for state {self.current_step}")
        self.step_time += diff_time
        self.last_time = current_time

    @set_time
    def _save_time(self, current_time: float) -> None:
        print_debug(f">>>>> Time SAVED {self.step_time} for state {self.current_step}")
        self.saved_times[self.current_step] = round(self.step_time, 3)
        self._reset()

    @set_time
    def start(self, step: str, current_time: float) -> None:
        print_debug(f">>>>> START {step}")
        if self.current_step == step:
            # step already started
            # resume it if it was paused
            self.resume(step, current_time)
            return
        if self.current_step is not None:
            # stop current step
            self.stop(step=self.current_step, current_time=current_time)
        self.current_step = step
        self.last_time = current_time

    def stop(self, step: Optional[str] = None, current_time: Optional[float] = None) -> None:
        print_debug(f">>>>> STOP {step or self.current_step}")
        if step is not None and step != self.current_step:
            return
        if self.current_step is not None:
            current_time = get_time(current_time)
            if not self.paused:
                self._add_time(current_time=current_time)
            self._save_time(current_time=current_time)

    def pause(self, step: Optional[str] = None, current_time: Optional[float] = None) -> None:
        print_debug(f">>>>> PAUSE {step or self.current_step}")
        if step is not None and step != self.current_step:
            # ignore this call as the step is not concerned
            return
        if self.current_step and not self.paused:
            current_time = get_time(current_time)
            self._add_time(current_time=current_time)
            self.paused = True
        print_debug(self.get_metrics())

    def resume(self, step: Optional[str] = None, current_time: Optional[float] = None) -> None:
        print_debug(f">>>>> RESUME {step or self.current_step}")
        if step is not None and step != self.current_step:
            # ignore this call as the step is not concerned
            return
        if self.current_step and self.paused:
            current_time = get_time(current_time)
            self.last_time = current_time
            self.paused = False

    def is_paused(self) -> bool:
        return self.paused

    def is_started(self, step=None) -> bool:
        if step is None:
            return self.current_step is not None
        return self.current_step == step

    def get_metrics(self) -> Dict[str, float]:
        return self.saved_times

    def total_metrics(self) -> float:
        total = sum(self.saved_times.values())
        return round(total, 3)

    def report_metrics(self) -> Dict[str, float]:
        metrics = {
            STEPS[step]: value
            for step, value in self.saved_times.items()
            if step in STEPS
        }
        metrics['total'] = self.total_metrics()
        return metrics

    def add_step_time(self, step: str, step_time: float) -> None:
        self.saved_times[step] = round(step_time, 3)


logtimer = Logtime()


def logtime_prim_auth(func):
    def cb_auth_wrapper(*args, **kwargs):
        logtimer.resume("PRIMARY_AUTH")
        res = func(*args, **kwargs)
        logtimer.pause("PRIMARY_AUTH")
        return res
    return cb_auth_wrapper


def logtime_function_resume(func):
    def wrapper(*args, **kwargs):
        init_paused = logtimer.is_paused()
        init_started = logtimer.is_started()
        if init_paused:
            logtimer.resume()
        res = func(*args, **kwargs)
        just_started = not init_started and logtimer.is_started()
        if init_paused or just_started:
            logtimer.pause()
        return res
    return wrapper


def logtime_function_pause(func):
    def wrapper(*args, **kwargs):
        logtimer.pause()
        res = func(*args, **kwargs)
        logtimer.resume()
        return res
    return wrapper


def logtime_function_step_resume(step):
    def real_decorator(func):
        def wrapper(*args, **kwargs):
            init_paused = logtimer.is_paused()
            init_started = logtimer.is_started(step)
            if init_paused:
                logtimer.resume(step)
            res = func(*args, **kwargs)
            just_started = not init_started and logtimer.is_started(step)
            if init_paused or just_started:
                logtimer.pause(step)
            return res
        return wrapper
    return real_decorator


def logtime_function_step_pause(step):
    def real_decorator(func):
        def wrapper(*args, **kwargs):
            logtimer.pause(step)
            res = func(*args, **kwargs)
            logtimer.resume(step)
            return res
        return wrapper
    return real_decorator

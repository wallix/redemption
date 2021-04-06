#!/usr/bin/python
# -*- coding: utf-8 -*-

from __future__ import absolute_import
from contextlib import contextmanager


@contextmanager
def manage_transaction(wabengine, close=False):
    wabengine.begin_transaction(silent=True)
    try:
        yield
    except Exception:
        wabengine.rollback_transaction()
        raise
    else:
        wabengine.commit_transaction()
    finally:
        if close:
            wabengine.close_proxy()

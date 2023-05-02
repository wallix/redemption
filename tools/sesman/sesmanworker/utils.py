##
# Copyright (c) 2023 WALLIX. All rights reserved.
# Licensed computer software. Property of WALLIX.
##
import re

from typing import Iterable, Any, Tuple, Generator


def collection_has_more(
        iterable: Iterable[Any]
) -> Generator[Tuple[Any, bool], None, None]:
    it = iter(iterable)
    try:
        cur_item = next(it)
    except StopIteration:
        return
    for item in it:
        yield cur_item, True
        cur_item = item
    yield cur_item, False

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


def parse_duration(duration: str) -> int:
    """
    duration format: {hours}h{min}m or {hours}h or {min}m
    """
    if duration:
        mres = re.search(r"(?:(\d+)h)?(?:(\d+)m)?", duration)
        if mres is not None:
            d = (
                60 * 60 * int(mres.group(1) or 0)
              + 60 * int(mres.group(2) or 0)
            )
            return d or 3600

    return 3600

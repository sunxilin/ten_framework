#
# This file is part of the TEN Framework project.
# Licensed under the Apache License, Version 2.0.
# See the LICENSE file for more information.
#
import logging

logger = logging.getLogger("default_extension_python")
logger.setLevel(logging.INFO)

formatter_str = (
    "%(asctime)s - %(name)s - %(levelname)s - %(process)d - "
    "[%(filename)s:%(lineno)d] - %(message)s"
)
formatter = logging.Formatter(formatter_str)

console_handler = logging.StreamHandler()
console_handler.setFormatter(formatter)

logger.addHandler(console_handler)

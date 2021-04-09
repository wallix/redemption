import sys
import os
from unittest.mock import Mock

sys.modules["logger"] = Mock()

sys.path.append(os.path.join(os.path.dirname(__file__), "../sesmanworker"))

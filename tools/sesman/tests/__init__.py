import sys
import os
from unittest.mock import Mock

sys.modules["wallix.logger"] = Mock()

sys.path.append(os.path.join(os.path.dirname(__file__), "../sesmanworker"))

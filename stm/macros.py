import subprocess
from datetime import datetime

revision = subprocess.check_output(["git", "rev-parse", "HEAD"]).strip()
print("-D_GIT_REV={}".format(revision))
print("-D_BUILD_TIME=\"{}\"".format(datetime.now().strftime("%Y-%m-%d %H:%M:%S")))

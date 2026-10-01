"""Remote, read-only display process names/states for Gate B row 13."""

import datetime
import json
import pathlib


def now():
    return datetime.datetime.now(datetime.timezone.utc).isoformat()


started = now()
names = {"Xorg", "X", "Xwayland", "slim", "lightdm", "xdm"}
matches = []
for path in pathlib.Path("/proc").iterdir():
    if not path.name.isdigit():
        continue
    try:
        comm = (path / "comm").read_text().strip()
    except OSError:
        continue
    if comm not in names:
        continue
    state = None
    try:
        for line in (path / "status").read_text().splitlines():
            if line.startswith("State:"):
                state = line.split(":", 1)[1].strip()
                break
    except OSError:
        pass
    matches.append({"pid": int(path.name), "comm": comm, "state": state})
print(json.dumps({"target_started_utc": started, "processes": sorted(matches, key=lambda x: x["pid"]),
                  "target_finished_utc": now()}, sort_keys=True))

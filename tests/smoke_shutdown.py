"""Boot original world data in an isolated runtime, then request a clean stop.

No historical accounts, notes, logs or other runtime state are imported.
"""
import pathlib
import signal
import socket
import subprocess
import tempfile
import time
import zipfile

repo = pathlib.Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix="anathema-stop-") as directory:
    runtime = pathlib.Path(directory)
    for name in ("area", "player", "backup", "notes", "log"):
        (runtime / name).mkdir(mode=0o700)
    with zipfile.ZipFile(repo / "Genesis.zip") as archive:
        for name in archive.namelist():
            path = pathlib.PurePosixPath(name)
            if str(path.parent) == "Genesis/area" and (
                path.suffix in (".are", ".hlp") or path.name == "area.lst"
            ):
                (runtime / "area" / path.name).write_bytes(archive.read(name))
    with socket.socket() as reservation:
        reservation.bind(("127.0.0.1", 0))
        port = reservation.getsockname()[1]
    with (runtime / "server.log").open("w") as log:
        process = subprocess.Popen(
            [str(repo / "Genesis/src/merc"), str(port)],
            cwd=runtime / "area", stdout=log, stderr=log,
        )
        try:
            deadline = time.monotonic() + 20
            while True:
                assert process.poll() is None, "Server exited before greeting"
                try:
                    client = socket.create_connection(("127.0.0.1", port), timeout=1)
                    break
                except OSError:
                    if time.monotonic() >= deadline:
                        raise AssertionError("Server did not start within 20 seconds")
                    time.sleep(0.1)
            with client:
                client.settimeout(5)
                assert client.recv(8192), "Empty TCP greeting"
                # Keep a partial-login descriptor connected during shutdown.
                process.send_signal(signal.SIGTERM)
                assert process.wait(timeout=10) == 0, "SIGTERM did not exit cleanly"
        finally:
            if process.poll() is None:
                process.kill()
                process.wait()
    assert "Normal termination of game." in (runtime / "server.log").read_text()
    assert not list((runtime / "player").iterdir()), "Saved a partial-login account"
print("Isolated boot, TCP greeting and SIGTERM shutdown passed")

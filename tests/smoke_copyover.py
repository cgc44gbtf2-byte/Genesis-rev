"""Actual exec/recovery with one synthetic staff player and private world data."""
import pathlib
import signal
import socket
import subprocess
import tempfile
import time
import zipfile

repo = pathlib.Path(__file__).resolve().parents[1]


def until(client, needle, timeout=25):
    deadline = time.monotonic() + timeout
    received = b""
    client.settimeout(0.5)
    while needle.lower() not in received.lower():
        if time.monotonic() > deadline:
            raise AssertionError("Expected server response did not arrive")
        try:
            part = client.recv(65536)
        except socket.timeout:
            continue
        assert part, "Connection closed before expected response"
        received += part
        assert len(received) < 2_000_000, "Unexpected unbounded server output"
    return received


with tempfile.TemporaryDirectory(prefix="anathema-copyover-") as directory:
    runtime = pathlib.Path(directory)
    for name in ("area", "player", "backup", "notes", "log", "clans"):
        (runtime / name).mkdir(mode=0o700)
    (runtime / "src").symlink_to(repo / "Genesis/src", target_is_directory=True)
    with zipfile.ZipFile(repo / "Genesis.zip") as archive:
        for name in archive.namelist():
            path = pathlib.PurePosixPath(name)
            if str(path.parent) == "Genesis/area" and (
                path.suffix in (".are", ".hlp") or path.name == "area.lst"
            ):
                (runtime / "area" / path.name).write_bytes(archive.read(name))
    subprocess.run([str(repo / "tests/test_restart"), "--seed",
                    str(runtime / "area")], check=True)
    with socket.socket() as reservation:
        reservation.bind(("127.0.0.1", 0))
        port = reservation.getsockname()[1]
    with (runtime / "server.log").open("w") as log:
        process = subprocess.Popen([str(repo / "Genesis/src/merc"), str(port)],
                                   cwd=runtime / "area", stdout=log, stderr=log)
        staff = pending = newcomer = None
        try:
            deadline = time.monotonic() + 25
            while True:
                assert process.poll() is None, "Server exited before login"
                try:
                    staff = socket.create_connection(("127.0.0.1", port), timeout=1)
                    break
                except OSError:
                    if time.monotonic() > deadline:
                        raise AssertionError("Server did not start")
                    time.sleep(0.1)
            assert staff.recv(65536), "Empty TCP greeting"
            staff.sendall(b"Restarter\n")
            until(staff, b"Password")
            staff.sendall(b"test-only-passphrase\n")
            until(staff, b"Wizinvis")
            staff.sendall(b"n\n")
            until(staff, b"Welcome to Godwars")
            staff.sendall(b"save\n")
            until(staff, b"Saved.")
            pending = socket.create_connection(("127.0.0.1", port), timeout=3)
            assert pending.recv(65536), "Empty pending-login greeting"

            # Both manual restart paths must refuse failed authoritative writes.
            (runtime / "player").rename(runtime / "unavailable")
            staff.sendall(b"shutdown\n")
            until(staff, b"Shutdown cancelled")
            assert process.poll() is None and not (runtime / "area/shutdown.txt").exists()
            staff.sendall(b"copyover\n")
            until(staff, b"Copyover cancelled")
            assert process.poll() is None and not (runtime / "area/copyover.data").exists()
            (runtime / "unavailable").rename(runtime / "player")

            # Production restart.c, real execl, original PID and active TCP socket.
            pid = process.pid
            staff.sendall(b"copyover\n")
            until(staff, b"Copyover recovery complete.")
            assert process.poll() is None and process.pid == pid
            assert not (runtime / "area/copyover.data").exists()
            staff.sendall(b"save\n")
            until(staff, b"Saved.")
            pending.settimeout(5)
            try:
                while pending.recv(65536):
                    pass
            except ConnectionResetError:
                pass
            newcomer = socket.create_connection(("127.0.0.1", port), timeout=3)
            assert newcomer.recv(65536), "Inherited listener did not greet"
            staff.sendall(b"shutdown\n")
            assert process.wait(timeout=15) == 0, "Checked shutdown did not exit cleanly"
        finally:
            for client in (staff, pending, newcomer):
                if client is not None:
                    client.close()
            if process.poll() is None:
                process.send_signal(signal.SIGTERM)
                try:
                    process.wait(timeout=10)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()
    text = (runtime / "server.log").read_text()
    assert text.count("ready to rock on port") == 2, "Actual re-exec was not observed"
    assert "Copyover recovery complete." in text and "Normal termination of game." in text
    assert not list((runtime / "player").glob(".*")), "Temporary player saves leaked"
print("Isolated save-failure refusal, real exec/TCP recovery and checked shutdown passed")

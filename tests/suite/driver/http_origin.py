#!/usr/bin/env python3

"""
A throwaway HTTP origin, just enough for one board to fetch from.

Deliberately not a real server and deliberately not somebody else's: pointing a
device at a site on the internet makes the test depend on that site being up,
on the link being fast enough not to trip a timeout, and on TLS material the
board may not carry. This binds a port of its own, serves a handful of shapes a
download command has to cope with, remembers what it was asked for, and goes
away with the test run.

The shapes matter more than the bytes. A body with a length, a body without one
so the client cannot compute a percentage, a length far larger than any device
filesystem so the refusal can be tested without transferring it, a body dripped
out in pieces so a progress display has something to redraw, and a plain 404.
"""

import socket
import threading
import time


class Origin(object):

    def __init__(self, host="0.0.0.0", port=0):
        self._host = host
        self._port = port
        self._sock = None
        self._thread = None
        self._stop = threading.Event()
        self._lock = threading.Lock()
        self._paths = []

    @property
    def port(self):
        return self._sock.getsockname()[1] if self._sock else self._port

    def start(self):
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._sock.bind((self._host, self._port))
        self._sock.listen(4)
        self._sock.settimeout(0.5)
        self._thread = threading.Thread(target=self._serve, daemon=True)
        self._thread.start()
        return self

    def stop(self):
        self._stop.set()
        if self._thread is not None:
            self._thread.join(timeout=5.0)
        if self._sock is not None:
            self._sock.close()
            self._sock = None

    def requested(self):
        """Every path asked for so far, in order."""
        with self._lock:
            return list(self._paths)

    def request_count(self):
        with self._lock:
            return len(self._paths)

    def _serve(self):
        while not self._stop.is_set():
            try:
                conn, _ = self._sock.accept()
            except socket.timeout:
                continue
            except OSError:
                break
            threading.Thread(target=self._handle, args=(conn,), daemon=True).start()

    def _handle(self, conn):
        try:
            conn.settimeout(10.0)
            request = b""
            while b"\r\n\r\n" not in request:
                chunk = conn.recv(1024)
                if not chunk:
                    return
                request += chunk

            path = request.split(b" ")[1].decode(errors="replace")
            with self._lock:
                self._paths.append(path)

            self._respond(conn, path.split("?")[0])
        except (OSError, IndexError):
            pass
        finally:
            try:
                conn.close()
            except OSError:
                pass

    def _respond(self, conn, path):
        if path == "/sized.bin":
            body = b"S" * 4000
            self._head(conn, 200, len(body))
            conn.sendall(body)

        elif path == "/drip.bin":
            body = b"D" * 4000
            self._head(conn, 200, len(body))
            for at in range(0, len(body), 500):
                conn.sendall(body[at:at + 500])
                time.sleep(0.02)

        elif path == "/nolength.bin":
            # closing the socket is what ends the body when no length is given
            conn.sendall(b"HTTP/1.1 200 OK\r\nConnection: close\r\n\r\n")
            conn.sendall(b"N" * 3000)

        elif path == "/huge.bin":
            # declared far larger than any device filesystem, never sent
            self._head(conn, 200, 100 * 1024 * 1024)

        elif path == "/small.txt":
            body = b"pdi"
            self._head(conn, 200, len(body))
            conn.sendall(body)

        else:
            body = b"not found"
            self._head(conn, 404, len(body))
            conn.sendall(body)

    def _head(self, conn, status, length):
        text = {200: "OK", 404: "Not Found"}.get(status, "OK")
        conn.sendall(("HTTP/1.1 %d %s\r\nContent-Length: %d\r\n"
                      "Connection: close\r\n\r\n" % (status, text, length)).encode())

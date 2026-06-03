import os
import socket
import sys
from http.server import (
    CGIHTTPRequestHandler,
    SimpleHTTPRequestHandler,
    ThreadingHTTPServer,
)

SimpleHTTPRequestHandler.extensions_map[".html"] = "text/html; charset=UTF-8"


def _get_best_family(*address):
    infos = socket.getaddrinfo(
        *address,
        type=socket.SOCK_STREAM,
        flags=socket.AI_PASSIVE,
    )
    family, type, proto, canonname, sockaddr = next(iter(infos))
    return family, sockaddr


def test(
    HandlerClass=SimpleHTTPRequestHandler,
    ServerClass=ThreadingHTTPServer,
    protocol="HTTP/1.0",
    port=8000,
    bind=None,
):
    """Test the HTTP request handler class.

    This runs an HTTP server on port 8000 (or the port argument).

    """
    ServerClass.address_family, addr = _get_best_family(bind, port)
    HandlerClass.protocol_version = protocol
    with ServerClass(addr, HandlerClass) as httpd:
        host, port = httpd.socket.getsockname()[:2]
        url_host = f"[{host}]" if ":" in host else host
        print(f"Serving HTTP on {host} port {port} " f"(http://{url_host}:{port}/) ...")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nKeyboard interrupt received, exiting.")
            sys.exit(0)


if __name__ == "__main__":
    import argparse
    import contextlib

    parser = argparse.ArgumentParser()
    parser.add_argument("--cgi", action="store_true", help="run as CGI server")
    parser.add_argument(
        "-b",
        "--bind",
        metavar="ADDRESS",
        help="bind to this address " "(default: all interfaces)",
    )
    parser.add_argument(
        "-d",
        "--directory",
        default=os.getcwd(),
        help="serve this directory " "(default: current directory)",
    )
    parser.add_argument(
        "-p",
        "--protocol",
        metavar="VERSION",
        default="HTTP/1.0",
        help="conform to this HTTP version " "(default: %(default)s)",
    )
    parser.add_argument(
        "port",
        default=8000,
        type=int,
        nargs="?",
        help="bind to this port " "(default: %(default)s)",
    )
    args = parser.parse_args()
    if args.cgi:
        handler_class = CGIHTTPRequestHandler
    else:
        handler_class = SimpleHTTPRequestHandler

    # ensure dual-stack is not disabled; ref #38907
    class DualStackServer(ThreadingHTTPServer):

        def server_bind(self):
            # suppress exception when protocol is IPv4
            with contextlib.suppress(Exception):
                self.socket.setsockopt(socket.IPPROTO_IPV6, socket.IPV6_V6ONLY, 0)
            return super().server_bind()

        def finish_request(self, request, client_address):
            self.RequestHandlerClass(
                request, client_address, self, directory=args.directory
            )

    test(
        HandlerClass=handler_class,
        ServerClass=DualStackServer,
        port=args.port,
        bind=args.bind,
        protocol=args.protocol,
    )

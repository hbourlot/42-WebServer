#!/usr/bin/env python3

import json
import os


def main():
    cookie_header = os.environ.get("HTTP_COOKIE", "")
    has_session = "sessionId=" in cookie_header

    payload = {
        "hasSession": has_session,
        "message": "Session detected" if has_session else "No session found",
    }

    print("Status: 200 OK")
    print("Content-Type: application/json")
    print()
    print(json.dumps(payload))


if __name__ == "__main__":
    main()

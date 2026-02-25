import os
import sys
# python3 terminal_connection.py /dev/pts/6
if len(sys.argv) != 2:
    print("Usage: python3 read_from_terminal.py /dev/pts/N")
    sys.exit(1)

terminal_path = sys.argv[1]

# Open the terminal device for reading
fd = os.open(terminal_path, os.O_RDONLY)
print(f"Listening on {terminal_path} (fd={fd})...")

try:
    while True:
        data = os.read(fd, 1024)
        if not data:
            break  # EOF
        print(f"Received on fd={fd}: {data.decode(errors='replace')}", end='')
except KeyboardInterrupt:
    print("\nExiting.")
finally:
    os.close(fd)

#!/usr/bin/env python3
"""
VLC Test Transmitter & Protocol Diagnostic Utility
Transmits encoded optical VLC control packets over a serial port (USB-to-TTL adapter)
connected to an LED or directly to the ESP32 receiver for bench testing.
"""

import sys
import time
import argparse

try:
    import serial
except ImportError:
    print("[ERROR] 'pyserial' is required. Install via: pip install pyserial")
    sys.exit(1)

SYNC_1 = 0xAA
SYNC_2 = 0x55
DEV_ID = 0x01

COMMANDS = {
    'play_pause': (0x10, 0x00, "Toggle Play / Pause"),
    'vol_up':     (0x20, 0x01, "Volume Up (+5%)"),
    'vol_down':   (0x21, 0x01, "Volume Down (-5%)"),
    'next':       (0x30, 0x00, "Next Track"),
    'prev':       (0x31, 0x00, "Previous Track"),
    'spk_a':      (0x40, 0x00, "Select Speaker A (Left)"),
    'spk_b':      (0x41, 0x00, "Select Speaker B (Right)"),
    'servo_center': (0x50, 90, "Aim Servo to 90 degrees"),
    'servo_left':   (0x50, 30, "Aim Servo to 30 degrees"),
    'servo_right':  (0x50, 150, "Aim Servo to 150 degrees"),
    'mute_all':   (0x60, 0x00, "Mute Both Amplifiers"),
}

def build_packet(opcode: int, value: int = 0x00) -> bytes:
    checksum = DEV_ID ^ opcode ^ value
    return bytes([SYNC_1, SYNC_2, DEV_ID, opcode, value, checksum])

def main():
    parser = argparse.ArgumentParser(description="VLC Protocol Diagnostic Tool")
    parser.add_argument("-p", "--port", required=True, help="Serial port (e.g., /dev/ttyUSB0 or COM3)")
    parser.add_argument("-b", "--baud", type=int, default=2400, help="Baud rate (default: 2400)")
    parser.add_argument("-c", "--cmd", choices=COMMANDS.keys(), help="Command name to transmit")
    parser.add_argument("--interactive", action="store_true", help="Launch interactive testing menu")
    args = parser.parse_args()

    try:
        ser = serial.Serial(args.port, args.baud, timeout=1)
        print(f"[OK] Connected to {args.port} at {args.baud} baud.")
    except Exception as e:
        print(f"[ERROR] Could not open {args.port}: {e}")
        sys.exit(1)

    if args.cmd:
        opcode, val, desc = COMMANDS[args.cmd]
        pkt = build_packet(opcode, val)
        ser.write(pkt)
        print(f"[TX] Sent '{args.cmd}' ({desc}): {[hex(b) for b in pkt]}")
        ser.close()
        return

    if args.interactive:
        print("\n--- Interactive VLC Protocol Transmitter ---")
        for key, (_, _, desc) in COMMANDS.items():
            print(f"  {key:<14} -> {desc}")
        print("  exit           -> Quit program\n")

        try:
            while True:
                choice = input("Enter command: ").strip().lower()
                if choice == "exit":
                    break
                if choice in COMMANDS:
                    opcode, val, desc = COMMANDS[choice]
                    pkt = build_packet(opcode, val)
                    ser.write(pkt)
                    print(f"  -> Transmitted: {[hex(b) for b in pkt]} ({desc})")
                else:
                    print("  [!] Unknown command. Try again.")
        except KeyboardInterrupt:
            pass
        finally:
            ser.close()
            print("\nClosed serial connection.")

if __name__ == "__main__":
    main()

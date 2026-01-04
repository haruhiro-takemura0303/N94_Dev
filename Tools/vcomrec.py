import sys
import serial
import wave
import time

MAGIC = b'PQS1'

def wait_magic(ser: serial.Serial) -> None:
    window = bytearray()
    while True:
        b = ser.read(1)
        if not b:
            continue
        window += b
        if len(window) > 4:
            window = window[-4:]
        if bytes(window) == MAGIC:
            return

def read_with_progress(ser: serial.Serial, n: int, idle_limit_sec: float = 5.0) -> bytes:
    buf = bytearray()
    last_rx = time.time()
    last_print = 0.0

    while len(buf) < n:
        chunk = ser.read(min(4096, n - len(buf)))
        now = time.time()

        if chunk:
            buf.extend(chunk)
            last_rx = now

        # 進捗表示（0.5秒に1回）
        if now - last_print >= 0.5:
            print(f"  received {len(buf)}/{n} bytes")
            last_print = now

        # 受信が途絶えたら終了
        if now - last_rx > idle_limit_sec:
            raise RuntimeError(f"受信が途絶えました（{idle_limit_sec}s）。受信済み={len(buf)} bytes / 期待={n} bytes")

    return bytes(buf)

def main():
    if len(sys.argv) < 3:
        print("Usage: python vcomrec.py COM5 out.wav")
        sys.exit(1)

    port = sys.argv[1]
    out_wav = sys.argv[2]

    sample_rate = 44100
    channels = 2
    sampwidth = 2
    frames = 176000  #176*1000
    total_bytes = frames * channels * sampwidth  # 176704

    # timeoutを短めにし、read_with_progress側で総合タイムアウト判定
    ser = serial.Serial(port=port, baudrate=115200, timeout=0.2)

    ser.reset_input_buffer()
    input("Enterキーでトリガ送信...")

    ser.write(b"\n")
    ser.flush()
    print("Trigger sent.")
    print("Waiting for PQS1 header...")
    wait_magic(ser)
    print("Header found. Receiving PCM...")

    payload = read_with_progress(ser, total_bytes, idle_limit_sec=10.0)

    with wave.open(out_wav, "wb") as wf:
        wf.setnchannels(channels)
        wf.setsampwidth(sampwidth)
        wf.setframerate(sample_rate)
        wf.writeframes(payload)

    print(f"OK: wrote {out_wav} ({frames} frames)")

if __name__ == "__main__":
    main()

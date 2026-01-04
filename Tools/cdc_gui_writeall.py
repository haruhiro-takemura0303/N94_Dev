import sys
import time
import threading
import queue
from dataclasses import dataclass
from pathlib import Path
from typing import Optional

import serial
import serial.tools.list_ports

from PySide6.QtCore import Qt, QTimer
from PySide6.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout, QTextEdit, QLineEdit,
    QPushButton, QLabel, QComboBox, QFileDialog, QSpinBox, QMessageBox, QCheckBox
)


@dataclass
class RxMessage:
    ts: float
    text: str


class SerialWorker:
    """
    pyserial の read/write をUIスレッドから分離するための簡易ワーカです。
    受信はバイト列→テキスト（UTF-8 replace）で行単位表示します。
    """
    def __init__(self):
        self.ser: Optional[serial.Serial] = None
        self.stop_event = threading.Event()
        self.rx_thread: Optional[threading.Thread] = None
        self.rx_queue: "queue.Queue[RxMessage]" = queue.Queue()

    def is_open(self) -> bool:
        return self.ser is not None and self.ser.is_open

    def open(self, port: str, baud: int, timeout: float = 0.05, write_timeout: float = 30.0):
        if self.is_open():
            self.close()

        self.ser = serial.Serial(
            port=port,
            baudrate=baud,        # CDC ACMでは無視されることが多いですがAPI都合で設定します
            timeout=timeout,
            write_timeout=write_timeout,
        )
        self.stop_event.clear()
        self.rx_thread = threading.Thread(target=self._rx_loop, daemon=True)
        self.rx_thread.start()

    def close(self):
        self.stop_event.set()
        if self.ser is not None:
            try:
                self.ser.close()
            except Exception:
                pass
        self.ser = None

    def _rx_loop(self):
        buf = bytearray()
        while not self.stop_event.is_set():
            if not self.is_open():
                time.sleep(0.05)
                continue

            try:
                n = self.ser.in_waiting
                data = self.ser.read(n or 1)
            except Exception as e:
                self.rx_queue.put(RxMessage(time.time(), f"[RX ERROR] {e}"))
                self.close()
                return

            if not data:
                continue

            buf.extend(data)
            while b"\n" in buf:
                line, _, rest = buf.partition(b"\n")
                buf = bytearray(rest)
                s = line.decode("utf-8", errors="replace")
                self.rx_queue.put(RxMessage(time.time(), s))

    def write_bytes(self, b: bytes):
        if not self.is_open():
            raise RuntimeError("Serial not open")
        self.ser.write(b)
        self.ser.flush()

    def write_text_line(self, s: str):
        self.write_bytes((s + "\n").encode("utf-8"))

    def send_trigger_then_wav(self, wav_path: Path, trigger_payload: bytes, chunk: int):
        """
        1) トリガ用OUTを1回送信（デバイス側は中身を見ない前提）
        2) 続けてwav本体を生バイトで送信（追加ヘッダなし）

        重要:
          - pyserial の write() は write_timeout を設定している場合、要求サイズより少ない
            バイト数だけを書いて戻る（短い書き込み）ことがあります。
          - 元の実装のように「要求分を書いた前提でオフセットを進める」とデータ欠落（歯抜け）になります。
          - 本実装は、各チャンクについて「全バイトが書けるまで再試行」します。
        """
        if not self.is_open():
            raise RuntimeError("Serial not open")

        data = wav_path.read_bytes()

        # トリガ（確実に全送信）
        self._write_all(trigger_payload)
        self.ser.flush()

        mv = memoryview(data)
        off = 0
        size = len(data)

        while off < size:
            end = min(off + chunk, size)
            self._write_all(mv[off:end])
            off = end

        self.ser.flush()

    def _write_all(self, buf) -> None:
        """
        buf（bytes または memoryview）を、全量書き込むまで write() を繰り返します。
        write_timeout により短い書き込みが発生する環境でのデータ欠落を防ぐためのヘルパです。
        """
        if not self.is_open():
            raise RuntimeError("Serial not open")

        mv = memoryview(buf)
        total = len(mv)
        sent = 0

        while sent < total:
            n = self.ser.write(mv[sent:])
            if n is None:
                n = 0
            if n <= 0:
                # 相手が詰まっている／OS側で進捗がない場合に備え、軽く待って再試行
                time.sleep(0.001)
                continue
            sent += n
class MainWindow(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("CDC Terminal + WAV Sender (Raw)")

        self.worker = SerialWorker()

        # UI部品
        self.port_combo = QComboBox()
        self.refresh_btn = QPushButton("更新")
        self.baud_spin = QSpinBox()
        self.baud_spin.setRange(1200, 3000000)
        self.baud_spin.setValue(115200)

        self.open_btn = QPushButton("接続")
        self.close_btn = QPushButton("切断")
        self.close_btn.setEnabled(False)

        self.console = QTextEdit()
        self.console.setReadOnly(True)
        self.console.setLineWrapMode(QTextEdit.NoWrap)

        self.input_line = QLineEdit()
        self.send_btn = QPushButton("送信")

        self.wav_path_line = QLineEdit()
        self.wav_browse_btn = QPushButton("参照...")
        self.wav_send_btn = QPushButton("WAV送信（Raw）")
        self.wav_send_btn.setEnabled(False)

        self.chunk_spin = QSpinBox()
        self.chunk_spin.setRange(512, 1024 * 1024)
        self.chunk_spin.setSingleStep(512)
        self.chunk_spin.setValue(512 * 32)  # 16KiB（HS/MPS=512の整数倍）

        self.trigger_hex_line = QLineEdit("00")  # 既定 1バイト 0x00
        self.trigger_hex_line.setMaxLength(64)   # 過大入力を抑制
        self.trigger_hex_line.setToolTip("トリガのHEX（例: 00 / DEADBEEF）。空なら 00 を送信します。")

        self.add_timestamp_cb = QCheckBox("受信に時刻を付与")
        self.add_timestamp_cb.setChecked(False)

        # レイアウト
        top = QHBoxLayout()
        top.addWidget(QLabel("ポート:"))
        top.addWidget(self.port_combo, 2)
        top.addWidget(self.refresh_btn)
        top.addWidget(QLabel("baud:"))
        top.addWidget(self.baud_spin)
        top.addWidget(self.open_btn)
        top.addWidget(self.close_btn)

        mid = QHBoxLayout()
        mid.addWidget(QLabel("入力:"))
        mid.addWidget(self.input_line, 1)
        mid.addWidget(self.send_btn)

        wav_row1 = QHBoxLayout()
        wav_row1.addWidget(QLabel("WAV:"))
        wav_row1.addWidget(self.wav_path_line, 1)
        wav_row1.addWidget(self.wav_browse_btn)

        wav_row2 = QHBoxLayout()
        wav_row2.addWidget(QLabel("chunk:"))
        wav_row2.addWidget(self.chunk_spin)
        wav_row2.addWidget(QLabel("trigger(hex):"))
        wav_row2.addWidget(self.trigger_hex_line, 1)
        wav_row2.addWidget(self.wav_send_btn)

        opt = QHBoxLayout()
        opt.addWidget(self.add_timestamp_cb)
        opt.addStretch(1)

        layout = QVBoxLayout()
        layout.addLayout(top)
        layout.addWidget(self.console, 1)
        layout.addLayout(mid)
        layout.addLayout(wav_row1)
        layout.addLayout(wav_row2)
        layout.addLayout(opt)
        self.setLayout(layout)

        # シグナル
        self.refresh_btn.clicked.connect(self.refresh_ports)
        self.open_btn.clicked.connect(self.open_port)
        self.close_btn.clicked.connect(self.close_port)

        self.send_btn.clicked.connect(self.send_text)
        self.input_line.returnPressed.connect(self.send_text)

        self.wav_browse_btn.clicked.connect(self.browse_wav)
        self.wav_send_btn.clicked.connect(self.send_wav)

        # 受信ポーリング（UIスレッドでキューを捌く）
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.drain_rx)
        self.timer.start(30)

        self.refresh_ports()

    def log(self, s: str):
        self.console.append(s)

    def refresh_ports(self):
        current = self.port_combo.currentText()
        self.port_combo.clear()
        ports = list(serial.tools.list_ports.comports())
        for p in ports:
            # 表示は "COM7 - USB Serial Device" のようにします
            label = f"{p.device} - {p.description}"
            self.port_combo.addItem(label, userData=p.device)

        # 可能なら元の選択を維持
        if current:
            idx = self.port_combo.findText(current)
            if idx >= 0:
                self.port_combo.setCurrentIndex(idx)

        if not ports:
            self.port_combo.addItem("(なし)", userData=None)

    def open_port(self):
        dev = self.port_combo.currentData()
        if not dev:
            QMessageBox.warning(self, "エラー", "利用可能なポートがありません。")
            return

        baud = int(self.baud_spin.value())
        try:
            self.worker.open(dev, baud)
        except Exception as e:
            QMessageBox.critical(self, "接続失敗", str(e))
            return

        self.log(f"[PC] connected: {dev} (baud={baud})")
        self.open_btn.setEnabled(False)
        self.close_btn.setEnabled(True)
        self.wav_send_btn.setEnabled(True)

    def close_port(self):
        self.worker.close()
        self.log("[PC] disconnected")
        self.open_btn.setEnabled(True)
        self.close_btn.setEnabled(False)
        self.wav_send_btn.setEnabled(False)

    def drain_rx(self):
        while True:
            try:
                msg = self.worker.rx_queue.get_nowait()
            except queue.Empty:
                break

            if self.add_timestamp_cb.isChecked():
                t = time.strftime("%H:%M:%S", time.localtime(msg.ts))
                self.log(f"[DEV {t}] {msg.text}")
            else:
                self.log(f"[DEV] {msg.text}")

    def send_text(self):
        if not self.worker.is_open():
            QMessageBox.information(self, "未接続", "先に接続してください。")
            return
        s = self.input_line.text()
        if s == "":
            return
        try:
            self.worker.write_text_line(s)
            self.log(f"[PC->DEV] {s}")
            self.input_line.clear()
        except Exception as e:
            QMessageBox.critical(self, "送信失敗", str(e))

    def browse_wav(self):
        path, _ = QFileDialog.getOpenFileName(
            self, "WAVファイルを選択", "", "WAV files (*.wav);;All files (*.*)"
        )
        if path:
            self.wav_path_line.setText(path)

    def _parse_trigger_hex(self) -> bytes:
        txt = self.trigger_hex_line.text().strip().replace(" ", "")
        if txt == "":
            return b"\x00"
        # 奇数長は先頭0埋め（例: "A" -> "0A"）
        if len(txt) % 2 == 1:
            txt = "0" + txt
        try:
            return bytes.fromhex(txt)
        except ValueError:
            raise ValueError("trigger(hex) が不正です。例: 00 / DEADBEEF")

    def send_wav(self):
        if not self.worker.is_open():
            QMessageBox.information(self, "未接続", "先に接続してください。")
            return

        path = self.wav_path_line.text().strip()
        if not path:
            QMessageBox.warning(self, "エラー", "WAVファイルを指定してください。")
            return

        wav_path = Path(path)
        if not wav_path.exists():
            QMessageBox.warning(self, "エラー", "指定されたファイルが存在しません。")
            return

        chunk = int(self.chunk_spin.value())
        if chunk % 512 != 0:
            QMessageBox.warning(self, "エラー", "chunk は 512 の整数倍にしてください。")
            return

        try:
            trig = self._parse_trigger_hex()
        except Exception as e:
            QMessageBox.warning(self, "エラー", str(e))
            return

        # UIを一時的にロック（長時間送信時の誤操作防止）
        self.wav_send_btn.setEnabled(False)
        self.send_btn.setEnabled(False)
        self.input_line.setEnabled(False)

        size = wav_path.stat().st_size
        self.log(f"[PC] WAV send start: {wav_path} ({size} bytes), trigger={trig.hex()}, chunk={chunk}")

        # 送信は別スレッドで実施（UIフリーズ回避）
        def _tx():
            try:
                self.worker.send_trigger_then_wav(wav_path, trigger_payload=trig, chunk=chunk)
                self.worker.rx_queue.put(RxMessage(time.time(), "[PC] WAV send done (device finishes by RIFF ChunkSize+8)."))
            except Exception as e:
                self.worker.rx_queue.put(RxMessage(time.time(), f"[PC] WAV send failed: {e}"))
            finally:
                # UI復帰はタイマ側で行う（安全のためフラグで制御）
                self.worker.rx_queue.put(RxMessage(time.time(), "__UI_UNLOCK__"))

        threading.Thread(target=_tx, daemon=True).start()

    def event(self, e):
        # UIアンロック用の簡易フック（受信キューに特殊メッセージ）
        if e.type() == 0:
            pass
        return super().event(e)

    def closeEvent(self, event):
        self.worker.close()
        super().closeEvent(event)

    # drain_rx で UI_UNLOCK を処理するために上書き
    def drain_rx(self):
        unlocked = False
        while True:
            try:
                msg = self.worker.rx_queue.get_nowait()
            except queue.Empty:
                break

            if msg.text == "__UI_UNLOCK__":
                unlocked = True
                continue

            if self.add_timestamp_cb.isChecked():
                t = time.strftime("%H:%M:%S", time.localtime(msg.ts))
                self.log(f"[DEV {t}] {msg.text}")
            else:
                self.log(f"[DEV] {msg.text}")

        if unlocked:
            self.wav_send_btn.setEnabled(self.worker.is_open())
            self.send_btn.setEnabled(True)
            self.input_line.setEnabled(True)


def main():
    app = QApplication(sys.argv)
    w = MainWindow()
    w.resize(980, 640)
    w.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()

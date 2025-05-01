from enum import Enum
import time
import serial
import threading
import queue

class MessageType(Enum):
    COLOR = 0x01
    NUMBER = 0x02
    STATUS = 0x03
    TEXT = 0x04

class PacketSender433Mhz:
    def __init__(self, port, baudrate=9600, delay=0.5):
        self.ser = serial.Serial(port, baudrate)
        self.send_queue = queue.Queue()
        self.delay = delay
        self._stop_event = threading.Event()
        self.sender_thread = threading.Thread(target=self._sender_worker)
        self.sender_thread.start()

    def _build_packet(self, message_type: MessageType, id_string: str, payload_data: str):
        if len(id_string) != 8:
            raise ValueError("ID must be 8 characters long!")

        header = 0xAA
        message_type_byte = message_type.value  # แปลง Enum เป็น int
        payload = id_string.encode() + payload_data.encode()
        data_length = len(payload)
        checksum = (header ^ message_type_byte ^ data_length ^ (sum(payload) & 0xFF)) & 0xFF
        end_byte = 0x55

        packet = bytearray([header, message_type_byte, data_length]) + payload + bytearray([checksum, end_byte])
        return packet

    def queue_message(self, message_type, id_string, payload_data):
        """เพิ่มคำสั่งลงคิว (message_type: int, id_string: str, payload_data: str)"""
        self.send_queue.put((message_type, id_string, payload_data))

    def _sender_worker(self):
        while not self._stop_event.is_set():
            try:
                message_type, id_string, payload_data = self.send_queue.get(
                    timeout=0.1)
                packet = self._build_packet(
                    message_type, id_string, payload_data)
                self.ser.write(packet)
                self.ser.flush()
                print("---------------------")
                print(
                    f"ส่ง message_type={message_type}, id={id_string}, payload={payload_data} \n hex={packet.hex()}")
                time.sleep(self.delay)
                self.send_queue.task_done()
            except queue.Empty:
                continue

    def close(self):
        self._stop_event.set()
        self.sender_thread.join()
        self.ser.close()
        print("ปิดการส่งข้อมูลและ Serial แล้ว")


if __name__ == "__main__":
    sender = PacketSender433Mhz(port='/dev/tty.usbserial-1230', delay=0.5)

    try:
        sender.queue_message(MessageType.COLOR, "DEVICE01", "red")
        sender.queue_message(MessageType.COLOR, "DEVICE01", "blue")

        sender.send_queue.join()
    except KeyboardInterrupt:
        print("การส่งถูกยกเลิกโดยผู้ใช้")
    finally:
        sender.close()

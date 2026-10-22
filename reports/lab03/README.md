# Chạy Lab 03 — A7680C

`src/main.cpp` chọn Lab 03 khi `LAB03_4G = true`. Đặt `false` để trở lại BLE.

## Đấu nối và khởi động

- GPIO17 (TX2) → RX của module.
- GPIO16 (RX2) ← TX của module.
- GND nối chung.
- Cấp nguồn theo thông số board A7680C đang dùng; sơ đồ VIN–VCC trong đề áp dụng cho board có nguồn đầu vào phù hợp, không phải mặc định cho module trần.
- Lắp SIM và anten; bật module theo hướng dẫn board.

Cấu hình chân và baud modem tại `include/cellular.h` (mặc định UART2 115200). Upload và mở USB Serial Monitor 115200, line ending LF/CR/CRLF. Nhập `AT` rồi Enter, đợi `OK`. Nếu không có phản hồi, kiểm tra nguồn, trạng thái bật, đấu chéo UART và baud thực tế.

## Lệnh nhập tại Serial Monitor

Lệnh AT được gửi khi nhấn Enter. Phản hồi modem được giữ nguyên để chụp log. `/help` hiển thị các lệnh cho bài thực hành. Không tự động gọi điện, gửi SMS hay xóa bộ nhớ.

Ví dụ SMS:

```text
AT+CMGF=1
AT+CMGS="<so_dien_thoai>"
```

Chờ dấu `>` rồi nhập nội dung chứa địa điểm và thời gian, Enter, sau đó nhập `/send` và Enter. `/send` gửi byte Ctrl+Z (0x1A), `/cancel` gửi Escape. Có thể dùng Ctrl+Z thực nếu terminal chuyển phím này tới cổng serial. Nội dung SMS nhập theo dòng được kèm CRLF.

Các bước kiểm tra mạng, đọc/xóa SMS, cuộc gọi, ping và TCP được liệt kê trong `report.typ` và tài liệu đề. Chạy lần lượt, chờ kết quả mỗi bước; cấu hình SIM/APN theo nhà mạng nếu cần. Mở TCP tới broker chưa tạo phiên MQTT.

## Minh chứng báo cáo

Lưu log thông tin module và tín hiệu trong nhà/ngoài trời; SMS và cuộc gọi hai chiều; đăng ký mạng, ping, IP và kết nối TCP. Che số điện thoại, IMSI trước khi đưa ảnh vào báo cáo.

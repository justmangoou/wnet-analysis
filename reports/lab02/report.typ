// Compile from this directory: typst compile report.typ report.pdf
// Cover information.
#let student = "Nguyễn Hoàng Tuấn"
#let class-group = "Mạng không dây (ELT3295 1)"

#let blue = rgb("00477f")
#set document(title: "Báo cáo Lab 02 — Truyền dữ liệu sử dụng Bluetooth", author: student)
#set text(font: "Times New Roman", size: 11pt, lang: "vi")
#set par(justify: true, leading: 0.55em)
#set page(paper: "a4", margin: (top: 21mm, bottom: 21mm, left: 25mm, right: 20mm),
  header: context if counter(page).get().first() > 1 {
    set text(size: 9pt, fill: gray)
    [MẠNG KHÔNG DÂY #h(1fr) LAB 02]
    line(length: 100%, stroke: 0.4pt + luma(190))
  },
  footer: context if counter(page).get().first() > 1 {
    align(center, text(size: 9pt)[#counter(page).display("1")])
  })
#set heading(numbering: "1.1")
#show heading: set text(fill: blue)
#show heading.where(level: 1): set block(above: 12pt, below: 12pt)
#set figure(gap: 6pt)
#show figure.caption: set text(size: 9pt)
#show raw: set text(font: "Menlo", size: 8pt)
#show raw.where(block: true): it => block(fill: rgb("f3f5f7"), inset: 9pt, radius: 3pt, width: 100%, it)
#set table(inset: 6pt, stroke: 0.4pt + luma(190))
#show table.cell.where(y: 0): set text(weight: "bold", fill: blue)
#let note(body) = block(width: 100%, fill: rgb("eef4f8"), inset: 9pt, body)

#align(center)[
  #text(size: 13pt, weight: "bold")[ĐẠI HỌC QUỐC GIA HÀ NỘI]
  #v(5pt)
  #text(size: 15pt, weight: "bold", fill: blue)[TRƯỜNG ĐẠI HỌC CÔNG NGHỆ]
  #v(20mm)
  #image("assets/logo-huet.svg", width: 36mm)
  #v(17mm)
  #text(size: 18pt, weight: "bold")[BÁO CÁO THỰC HÀNH]
  #v(7pt)
  #text(size: 15pt)[Học phần: Mạng không dây]
  #v(13mm)
  #text(size: 23pt, weight: "bold", fill: blue)[LAB 02]
  #v(7pt)
  #text(size: 19pt, weight: "bold")[TRUYỀN DỮ LIỆU\ SỬ DỤNG BLUETOOTH]
]
#v(15mm)
#align(center, table(columns: (42mm, 85mm), stroke: none, inset: 5pt,
  [Giảng viên hướng dẫn:], [Lâm Sinh Công\ Cấn Quang Trường],
  [Thành viên nhóm:], [Nguyễn Hoàng Tuấn\ Đào Hải Vinh\ Trịnh Thị Kim Oanh],
  [Lớp / Nhóm:], class-group,
))
#v(1fr)

#pagebreak()
= Bluetooth Classic — Bài 1

== Mục tiêu và cấu hình
Thực hiện truyền văn bản hai chiều giữa ESP32 và điện thoại Android qua Bluetooth Classic SPP; khảo sát khoảng cách liên lạc. ESP32 sử dụng Arduino framework trên PlatformIO, thư viện `BluetoothSerial` và tên thiết bị *WNET-ESP32-BT*. Cổng USB Serial được cấu hình ở *115200 baud*; đây là tốc độ cổng USB Serial, không phải tốc độ đường truyền Bluetooth.

== Cách thực hiện
+ Nạp chương trình gọi `bluetoothSetup()` và `bluetoothLoop()` trong `src/main.cpp`; mở USB Serial Monitor.
+ Ghép đôi Android với ESP32, sau đó mở kết nối *Classic Bluetooth SPP/RFCOMM* trong ứng dụng terminal. Ghép đôi chưa đồng nghĩa với mở kết nối truyền dữ liệu. Chỉ kết nối một client tại một thời điểm.
+ Nhập một dòng trên USB Serial Monitor rồi nhấn Enter. ESP32 gom tối đa *254 byte*, nhận kết thúc dòng CR hoặc LF và gửi kèm CRLF; log gửi có nhãn `[BT TX]`.
+ Gửi bản tin từ Android và quan sát log `[BT RX]` trên USB Serial Monitor. Dữ liệu nhận không được tự động gửi ngược lại cho client. Khi ngắt kết nối, chương trình khôi phục trạng thái cho phép phát hiện và kết nối lại.

== Kết quả thực nghiệm
#figure(
  image("assets/1_esp_serial.png", width: 85%),
  caption: [USB Serial Monitor: ESP32 kết nối, gửi và nhận bản tin.],
) <classic-esp>

#figure(
  image("assets/1_android_serial.png", width: 100%),
  caption: [Ứng dụng Android kết nối với WNET-ESP32-BT và trao đổi bản tin.],
) <classic-android>


== Khảo sát khoảng cách
Tại mỗi khoảng cách, gửi cùng số bản tin có đánh số thứ tự theo từng chiều; ghi riêng số nhận/gửi, vật cản và tình trạng kết nối. Chọn khoảng cách xa nhất còn trao đổi ổn định theo cả hai chiều.

#text(size: 9pt)[
#table(columns: (23mm, 1fr, 1fr, 1fr),
  [Độ dài tuyến đo], [Điều kiện / vật cản], [ESP → Android], [Android → ESP],
  [≈25 m], [1 cửa], [Nhận đủ 10/10], [Nhận đủ 10/10],
)
]
*Độ dài tuyến đo xa nhất còn liên lạc được: ~25 m*, với *1 cửa chắn*. Tuyến đo đi dọc hành lang và qua các đoạn rẽ, tương ứng đường màu đỏ trên sơ đồ được cung cấp. Giá trị 25 m là chiều dài theo tuyến di chuyển, không phải khoảng cách đường thẳng giữa hai thiết bị. Đây là kết quả trong điều kiện thử nghiệm, không phải giới hạn khoảng cách của thiết bị trong mọi môi trường.


#figure(
  image("assets/1_latency.png", height: 78mm),
  caption: [Log Android khi thử bản tin đánh số và di chuyển theo tuyến đo.],
) <classic-latency>

Trong quá trình thử nghiệm, các bản tin A1–A10 và E1–E10 đều được nhận đầy đủ. Khi di chuyển dọc hành lang, một số bản tin đến chậm rồi được nhận liên tiếp trong khoảng thời gian ngắn, thể hiện ở nhóm E3–E7 trong @classic-latency. Kết nối vẫn truyền được dữ liệu nhưng có độ trễ khi di chuyển.

= Bluetooth Low Energy — Bài 2

== Mô hình và cấu hình
Thử nghiệm sử dụng *ESP32 làm GATT server* và *điện thoại Android làm client* với ứng dụng Serial Bluetooth Terminal. ESP32 cung cấp Nordic UART Service: Android gửi dữ liệu bằng thao tác Write; ESP32 gửi dữ liệu tới ứng dụng bằng Notify sau khi client đăng ký nhận thông báo.

#table(columns: (34mm, 1fr),
  [Thành phần], [Cấu hình thực nghiệm],
  [Server], [`WNET-BLE-Server` — ESP32],
  [Client], [Android — Serial Bluetooth Terminal, profile Nordic UART],
  [Service UUID], [`6e400001-b5a3-f393-e0a9-e50e24dcca9e`],
  [Write UUID], [`6e400002-b5a3-f393-e0a9-e50e24dcca9e`],
  [Notify UUID], [`6e400003-b5a3-f393-e0a9-e50e24dcca9e`],
  [USB Serial], [115200 baud; CR, LF hoặc CRLF để kết thúc dòng],
)

== Cách thực hiện
+ Nạp chương trình gọi `bluetoothBleServerSetup()` trong `setup()` và `bluetoothBleServerLoop()` trong `loop()`. Mã xử lý nằm trong `src/bluetooth_ble.cpp`.
+ Trong ứng dụng Android, quét thiết bị BLE, chọn *WNET-BLE-Server* và kết nối bằng profile *Nordic UART*.
+ Gửi `Hello from Android` kèm ký tự kết thúc dòng. ESP32 gom dữ liệu và hiển thị một dòng có nhãn `[BLE RX]`.
+ Trên USB Serial Monitor, nhập `Hello from ESP` rồi nhấn Enter. ESP32 gửi qua Notify và ghi log `[BLE TX]`. Mỗi dòng tối đa 254 byte; dữ liệu gửi được chia thành các gói tối đa 20 byte để phù hợp ATT MTU mặc định.
+ Ngắt kết nối trên Android, chờ ESP32 quảng bá lại, kết nối lại và lặp lại việc gửi bản tin.

== Kết quả truyền dữ liệu và kết nối lại
Log USB Serial thu được trong thực nghiệm:

```text
[BLE] Server WNET-BLE-Server | MAC 68:09:47:9b:87:8a
[BLE] Advertising; waiting for client writes
[BLE] Client connected
[BLE RX] Hello from Android
[BLE TX] Hello from ESP
[BLE] Client disconnected
[BLE] Advertising restarted
[BLE] Client connected
[BLE RX] Hello from Android
[BLE TX] Hello from ESP
```

#figure(
  image("assets/2_android_ble.png", width: 100%),
  caption: [Android gửi, nhận bản tin qua BLE Nordic UART trước và sau khi kết nối lại.],
) <ble-android>

ESP32 nhận đúng bản tin `Hello from Android` trước và sau khi kết nối lại. Sau khi client ngắt kết nối, server khởi động lại quảng bá và chấp nhận kết nối mới. Ở cả hai lần kết nối, Android hiển thị đúng bản tin `Hello from ESP` do ESP32 gửi qua Notify. Log trên hai thiết bị xác nhận truyền văn bản thành công theo cả hai chiều trước và sau khi kết nối lại.


= Kết luận
Bluetooth Classic truyền được dữ liệu ở cuối tuyến hành lang dài *khoảng 25 m với 1 cửa chắn*, nhận đủ bản tin thử nhưng có độ trễ khi di chuyển. Với BLE Nordic UART, ESP32 và Android trao đổi văn bản thành công theo cả hai chiều trước và sau khi kết nối lại.

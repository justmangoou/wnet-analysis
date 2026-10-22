// Compile from this directory: typst compile report.typ report.pdf
// Cover information.
#let student = "Nguyễn Hoàng Tuấn"
#let student-id = "24022923"
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
  [Sinh viên thực hiện:], student,
  [Mã sinh viên:], student-id,
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

== Mục tiêu và nguyên lý
Triển khai kết nối BLE giữa hai ESP32 theo mô hình *GATT server–client*. Server quảng bá Service; client quét, kết nối và truy cập Characteristic theo UUID. Khác với luồng nối tiếp SPP của Bài 1, dữ liệu BLE trong bài được ghi và đọc qua một Characteristic.

#table(columns: (37mm, 1fr),
  [Thành phần], [Cấu hình],
  [Server], [`WNET-BLE-Server`],
  [Client], [`WNET-BLE-Client`],
  [Service UUID], [`6b8a0001-3c6d-4fa0-9e21-7a1081f00101`],
  [Characteristic UUID], [`6b8a0002-3c6d-4fa0-9e21-7a1081f00101`],
  [Quyền truy cập], [`READ | WRITE`],
  [Dữ liệu đầu vào], [Tối đa 20 byte mỗi dòng, không tính ký tự kết thúc dòng],
)

== Chuẩn bị và thực hiện
Mã nguồn BLE nằm trong `src/bluetooth_ble.cpp`. `src/main.cpp` hiện chạy Bluetooth Classic; để thực hiện Bài 2, cần thay cặp hàm gọi trong `setup()` và `loop()` theo vai trò rồi nạp riêng cho từng ESP32:

#table(columns: (25mm, 1fr, 1fr),
  [Vai trò], [Gọi sau `Serial.begin(115200)`], [Gọi trong `loop()`],
  [Server], [`bluetoothBleServerSetup()`], [`bluetoothBleServerLoop()`],
  [Client], [`bluetoothBleClientSetup()`], [`bluetoothBleClientLoop()`],
)

+ Khởi động server để tạo Service, Characteristic và bắt đầu quảng bá.
+ Khởi động client. Client quét theo Service UUID, kết nối và tìm Characteristic có quyền đọc/ghi.
+ Trên USB Serial của client, nhập `hello` rồi nhấn Enter với kết thúc dòng CR hoặc LF. Client ghi có phản hồi, sau đó đọc lại để đối chiếu.
+ Kiểm tra server hiển thị `[BLE RX] hello`; client chỉ báo `[BLE] Write verified: hello` khi giá trị đọc lại trùng với bản tin đã ghi. Dòng vượt 20 byte bị loại bỏ.

== Kiểm tra kết nối lại
Callback ngắt kết nối của server đặt cờ để vòng lặp khởi động lại quảng bá. Khi mất kết nối, client xóa trạng thái bản tin, quay lại quét và tìm lại Service/Characteristic sau khi kết nối.

Lần lượt khởi động lại server và client, chờ thiết lập lại kết nối rồi gửi một bản tin mới. Chỉ xác nhận thành công khi server nhận đúng và client đọc lại khớp nội dung. Việc có mã xử lý kết nối lại chưa thay thế cho kiểm thử thực tế.

#note[*Trạng thái thực nghiệm:* chưa có ảnh log BLE. Cần bổ sung minh chứng ghi/đọc Characteristic và truyền bản tin sau khi kết nối lại.]

= Kết luận
ESP32 và Android đã truyền văn bản hai chiều qua Bluetooth Classic, còn liên lạc được ở cuối tuyến hành lang dài *khoảng 25 m với 1 cửa chắn*. Các bản tin thử nghiệm được nhận đủ, có hiện tượng trễ và nhận dồn khi di chuyển. Phần BLE cần bổ sung kết quả kiểm thử thực tế.

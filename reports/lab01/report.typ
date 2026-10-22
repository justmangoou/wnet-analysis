// Compile from this directory: typst compile report.typ report.pdf
// Cover information.
#let student = "Nguyễn Hoàng Tuấn"
#let student-id = "24022923"
#let class-group = "Mạng không dây (ELT3295 1)"

#let blue = rgb("00477f")
#set document(title: "Báo cáo Lab 01 — Truyền dữ liệu sử dụng sóng Wi-Fi", author: student)
#set text(font: "Times New Roman", size: 11pt, lang: "vi")
#set par(justify: true, leading: 0.55em)
#set page(paper: "a4", margin: (top: 21mm, bottom: 21mm, left: 25mm, right: 20mm),
  header: context if counter(page).get().first() > 1 {
    set text(size: 9pt, fill: gray)
    [MẠNG KHÔNG DÂY #h(1fr) LAB 01]
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
#let shot(n, caption, width: 100%) = [#figure(image("assets/image" + str(n) + ".png", width: width), caption: caption)#label("evidence-" + str(n))]
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
  #text(size: 23pt, weight: "bold", fill: blue)[LAB 01]
  #v(7pt)
  #text(size: 19pt, weight: "bold")[TRUYỀN DỮ LIỆU\ SỬ DỤNG SÓNG WI-FI]
]
#v(15mm)
#align(center, table(columns: (42mm, 85mm), stroke: none, inset: 5pt,
  [Sinh viên thực hiện:], student,
  [Mã sinh viên:], student-id,
  [Lớp / Nhóm:], class-group,
))
#v(1fr)

#pagebreak()
= Mục tiêu và cơ sở lý thuyết

== Mục tiêu thực hành
Khảo sát khả năng kết nối và trao đổi dữ liệu Wi-Fi của ESP32: kết nối vào điểm truy cập ở chế độ Station, đọc tham số mạng, quét các AP lân cận, trao đổi bản tin giữa hai ESP32, và tạo mạng cục bộ bằng chế độ Access Point. Kiểm tra khả năng liên lạc IP bằng lệnh ping và đối chiếu kết quả với nhật ký Serial.

== Thiết bị và môi trường
Thiết bị thực hành gồm các board ESP32, máy tính kết nối qua USB và điểm truy cập Wi-Fi. Chương trình sử dụng Arduino framework trên PlatformIO, cấu hình board `esp32dev`; Serial Monitor chạy ở 115200 baud.

== Cơ sở lý thuyết
Ở chế độ *Station*, ESP32 tham gia mạng do một AP cung cấp; địa chỉ có thể được cấp bằng DHCP hoặc đặt tĩnh. Ở chế độ *SoftAP*, ESP32 tạo điểm truy cập để các thiết bị khác kết nối.

RSSI biểu diễn mức tín hiệu thu, đơn vị dBm; giá trị ít âm hơn tương ứng mức tín hiệu thu lớn hơn. Ping kiểm tra phản hồi ICMP và thời gian khứ hồi (RTT). UDP truyền bản tin theo địa chỉ IP và cổng; ứng dụng cần tự bổ sung xác nhận nếu muốn kiểm tra việc nhận từng bản tin.

= Cài đặt chế độ Station và truyền bản tin

== Cấu hình kết nối — Bài 1.1 đến 1.4
Hàm `wnetStationSetup(dhcp)` đặt `WIFI_STA`, chọn cấp IP động hoặc tĩnh, bật tự kết nối lại và gọi `WiFi.begin`. Truyền `nullptr` làm mật khẩu cho mạng mở; với mạng được bảo vệ, cung cấp mật khẩu tương ứng. Đoạn mã rút gọn từ chương trình:

```cpp
WiFi.mode(WIFI_STA);
bool configured = dhcp
  ? WiFi.config(IPAddress(), IPAddress(), IPAddress())
  : WiFi.config(ip, gateway, subnet, dns);
if (!configured) return;
WiFi.setAutoReconnect(true);
WiFi.begin(UPSTREAM_SSID, UPSTREAM_PASSWORD);
```

Thứ tự đối số cấu hình tĩnh ở đây theo mã đang sử dụng: IP cục bộ, gateway, subnet, DNS. Trong khi chờ `WL_CONNECTED`, chương trình in dấu chấm mỗi 500 ms. Khi kết nối thành công, chương trình in RSSI, BSSID của AP, MAC của ESP32, IP cục bộ, gateway và subnet mask.

#note[*Phân biệt địa chỉ:* `WiFi.BSSIDstr()` trả về MAC của AP đang kết nối; `WiFi.macAddress()` trả về MAC giao diện Station của ESP32. Hai giá trị thể hiện hai thiết bị/giao diện khác nhau.]

== Quét mạng — Bài 1.5
Chương trình gọi `WiFi.scanNetworks()`, sau đó duyệt kết quả để in SSID, RSSI, kênh và trạng thái `Open`/`Secured`. Kết thúc, `WiFi.scanDelete()` giải phóng bộ nhớ kết quả quét. Các bản quét là những quan sát tại từng thời điểm, không phải danh sách cố định của môi trường.

== Trao đổi UDP — Bài 1.6
Hai board cần kết nối cùng AP và đặt `peerIp` là địa chỉ của board còn lại. Mỗi bên mở UDP cổng *4210*, nhận nội dung từ Serial rồi gửi khi nhấn Enter. Log `[TX]` báo thao tác gửi phía cục bộ; log `[RX địa_chỉ]` ghi nội dung nhận và IP nguồn. Mã chỉ nhận gói có cổng nguồn 4210 và kích thước không quá *240 byte*.

Giới hạn được tính theo byte; dòng vượt giới hạn bị bỏ và báo lỗi. UDP trong chương trình chưa có cơ chế xác nhận hoặc truyền lại, vì vậy cần đối chiếu log RX để kiểm tra bản tin đã đến đích.

#pagebreak()
= Kết quả Station, quét Wi-Fi và ping

#shot(1, [Station tại 10.11.99.73, bản quét 25 mạng và các thống kê ping trên Windows.], width: 94%)
#shot(2, [Một lần quan sát khác: RSSI −81 dBm, 28 mạng được phát hiện và ping trung bình 62 ms.], width: 94%)

#text(size: 9pt)[
#table(columns: (1fr, 1fr),
  [Thông số], [Giá trị đo được],
  [IP ESP32 / gateway], [10.11.99.73 / 10.11.96.1],
  [Subnet mask], [255.255.248.0 (/21)],
  [BSSID AP / MAC ESP32], [4C:B1:CD:36:86:B8 / 88:13:BF:6F:84:30],
  [RSSI / số mạng quét], [@evidence-1: −74 dBm / 25; @evidence-2: −81 dBm / 28],
)
]

@evidence-2 có 4/4 phản hồi, mất 0%, RTT nhỏ nhất/trung bình/lớn nhất là 53/62/79 ms. @evidence-1 hiển thị cùng thống kê và thêm một lượt 4/4 phản hồi với 64/100/176 ms. Bản quét ghi nhận SSID `UET-Wifi-Office-Free 2.4Ghz` trạng thái Open, có RSSI tương ứng với mức tín hiệu kết nối.

#pagebreak()
= Kết quả trao đổi bản tin UDP

== Hai ESP trong mạng hiện có — Bài 1.6
#shot(3, [Một đầu nhận tin từ 10.11.101.116 và gửi các bản tin trả lời.])
#shot(4, [Đầu đối diện gửi tin và nhận phản hồi từ 10.11.101.115.])

#table(columns: (30mm, 1fr, 1fr),
  [Chiều truyền], [Nội dung gửi], [Minh chứng nhận],
  [.116 → .115], [`This is from ESP01`, `Hello World`], [@evidence-3: RX từ 10.11.101.116],
  [.115 → .116], [`This is from ESP02\`, `Hello`], [@evidence-4: RX từ 10.11.101.115],
)

Các bản tin gửi từ 10.11.101.116 được nhận tại 10.11.101.115 và ngược lại, xác nhận trao đổi văn bản hai chiều. Log nhận hiển thị đúng nội dung và địa chỉ IP nguồn. Các địa chỉ này thuộc lần cấu hình khác với phép thử Station tại 10.11.99.73.

== Thử nghiệm UDP bổ sung
#shot(7, [Log UDP cổng 4210: gửi “test”, “adf” và nhận bản tin từ 192.168.4.116.])

Log ghi nhận hai bản tin gửi `test`, `adf` và một bản tin nhận `nonono\` từ 192.168.4.116. Đây là lần cấu hình mạng riêng, khác dải 192.168.10.x của phép thử Access Point.

#pagebreak()
= Kết quả chế độ Access Point

== Tạo AP và liệt kê thiết bị — Bài 2.1 đến 2.3
Mã dùng `WiFi.mode(WIFI_AP)`, cấu hình IP bằng `WiFi.softAPConfig(apIp, apGateway, apSubnet)` rồi phát mạng bằng `WiFi.softAP(AP_SSID, AP_PASSWORD, apChannel, 0, apMaxClients)`. Giới hạn hiện đặt là 2 thiết bị, kênh 1. Hàm vòng lặp kiểm tra danh sách client mỗi 500 ms và in thay đổi địa chỉ MAC/IP.

#shot(5, [AP có SSID Namper, gateway 192.168.10.1 và danh sách hai client.])

#text(size: 9pt)[
#table(columns: (1fr, 1fr),
  [Thành phần], [Giá trị trong ảnh],
  [SSID / AP MAC], [Namper / 88:13:BF:6F:84:31],
  [Client 1], [4E:09:97:A7:86:9E — 192.168.10.2],
  [Client 2], [A6:62:17:D0:CC:D4 — 192.168.10.3],
  [Số thiết bị], [Tăng từ 0 lên 1 rồi 2; thông báo đạt 2/2],
)
]

Số client kết nối tăng từ 0 lên 2 và chương trình thông báo đạt giới hạn 2/2. IP của client đầu tiên được cập nhật từ 0.0.0.0 thành 192.168.10.2. Phép thử này ghi nhận số client đạt ngưỡng cấu hình, chưa kiểm tra việc từ chối thiết bị thứ ba.

== Ping tới gateway
#shot(6, [Máy tính ping AP tại 192.168.10.1: nhận đủ sáu phản hồi.], width: 88%)
Kết quả: gửi 6, nhận 6, mất 0%; RTT nhỏ nhất/trung bình/lớn nhất là *4.341/9.671/24.757 ms*, độ lệch chuẩn 7.051 ms. Kết quả xác nhận máy tính liên lạc được với gateway tại 192.168.10.1.

#pagebreak()
= Trả lời câu hỏi

== Khi nào dùng IP tĩnh, khi nào dùng IP động?
Dùng *IP tĩnh* khi cần địa chỉ ổn định cho gateway, server hoặc hai ESP trao đổi UDP. Dùng *IP động (DHCP)* cho thiết bị đầu cuối để tự động nhận cấu hình mạng, giảm thao tác thủ công.

== Tại sao địa chỉ Wi-Fi thường có dạng 192.168.X.X?
192.168.0.0/16 là dải IP riêng thường được router sử dụng trong mạng nội bộ. Wi-Fi không bắt buộc dùng dải này; mạng Station trong bài sử dụng địa chỉ 10.11.x.x.

== ESP32 phát 2.4 GHz hay 5 GHz? Kiểm tra thế nào?
ESP32 sử dụng trong bài hỗ trợ Wi-Fi *2.4 GHz*. Có thể kiểm tra datasheet của chip hoặc kênh Wi-Fi trong kết quả quét. Các kênh 1, 6, 10 và 11 trong bài thuộc băng 2.4 GHz; tên SSID có chữ “5Ghz” không xác định băng tần thực tế.

= Kết luận
Thực hành đã khảo sát kết nối Wi-Fi của ESP32 ở chế độ Station, đọc thông tin mạng, quét các AP xung quanh và trao đổi văn bản UDP giữa hai ESP. Ở chế độ Access Point, ESP32 hoạt động tại 192.168.10.1, liệt kê MAC/IP của hai client và phản hồi đầy đủ sáu gói ping từ máy tính.

Kết quả cho thấy ESP32 có thể đảm nhiệm vai trò thiết bị đầu cuối hoặc điểm truy cập trong mạng cục bộ. Nội dung thực nghiệm trong báo cáo tập trung vào Station, SoftAP và UDP; chưa đánh giá chế độ AP+STA và truyền dữ liệu lên Internet.

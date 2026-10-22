// Compile from this directory: typst compile report.typ report.pdf
// Cover information.
#let student = "Nguyễn Hoàng Tuấn"
#let class-group = "Mạng không dây (ELT3295 1)"

#let blue = rgb("00477f")
#set document(title: "Báo cáo Lab 03 — Truyền dữ liệu sử dụng sóng di động 4G", author: student)
#set text(font: "Times New Roman", size: 11pt, lang: "vi")
#set par(justify: true, leading: 0.55em)
#set page(paper: "a4", margin: (top: 21mm, bottom: 21mm, left: 25mm, right: 20mm),
  header: context if counter(page).get().first() > 1 {
    set text(size: 9pt, fill: gray)
    [MẠNG KHÔNG DÂY #h(1fr) LAB 03]
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
#show raw.where(block: false): set text(font: "Times New Roman", size: 11pt)
#show raw.where(block: true): set text(font: "Menlo", size: 8.5pt)
#show raw.where(block: true): it => block(breakable: false, fill: rgb("f3f5f7"), inset: 9pt, radius: 3pt, width: 100%, it)
#show table.cell: set par(justify: false)
#set table(inset: 6pt, stroke: 0.4pt + luma(190))
#show table.cell.where(y: 0): set text(weight: "bold", fill: blue)

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
  #text(size: 23pt, weight: "bold", fill: blue)[LAB 03]
  #v(7pt)
  #text(size: 19pt, weight: "bold")[TRUYỀN DỮ LIỆU\ SỬ DỤNG SÓNG DI ĐỘNG 4G]
]
#v(15mm)
#align(center, table(columns: (42mm, 85mm), stroke: none, inset: 5pt,
  [Giảng viên hướng dẫn:], [Lâm Sinh Công\ Cấn Quang Trường],
  [Thành viên nhóm:], [Nguyễn Hoàng Tuấn\ Đào Hải Vinh\ Trịnh Thị Kim Oanh],
  [Lớp / Nhóm:], class-group,
))
#v(1fr)

#pagebreak()
= Mục tiêu và cấu hình thực nghiệm
Sử dụng ESP32 điều khiển module A7680C-LANS bằng lệnh AT để kiểm tra mạng, khảo sát tín hiệu, gửi/nhận SMS, thực hiện cuộc gọi và truy cập Internet qua SIM 4G. Chương trình được xây dựng trên Arduino framework với PlatformIO.

#table(columns: (38mm, 1fr),
  [Thành phần], [Cấu hình],
  [Số SIM trong module], [`0902272836`],
  [Số điện thoại đối tác], [`0856770806`],
  [Module / firmware], [`SIMCOM A7680C-LANS` / `V6.9.01`],
  [UART2 của ESP32], [GPIO17 (TX) → RX module; GPIO16 (RX) ← TX module],
  [GND], [Nối chung giữa ESP32 và module],
  [Giao tiếp Serial], [115200 baud; lệnh AT kết thúc bằng CRLF],
)
Nguồn cấp sử dụng theo thông số board module. ESP32 chuyển tiếp lệnh từ USB Serial sang UART2 và hiển thị phản hồi nguyên bản. Lệnh được gửi khi nhấn Enter; `/send` gửi ký tự Ctrl+Z (0x1A) để kết thúc nội dung SMS.

= Kiểm tra module, SIM và chất lượng mạng

== Thông tin nhận dạng và đăng ký mạng
#table(columns: (31mm, 1fr),
  [Lệnh], [Phản hồi / kết quả],
  [`AT` / `ATI`], [`OK`; nhà sản xuất `SIMCOM INCORPORATED`],
  [`AT+CPIN?`], [`+CPIN: READY` — SIM sẵn sàng],
  [`AT+CIMI`], [`45…`],
  [IMEI (từ `ATI`)], [`86…`],
  [`AT+COPS?`], [`+COPS: 0,2,"45201",7`],
  [`AT+CREG?`], [`+CREG: 0,1`],
  [`AT+CEREG?`], [`+CEREG: 0,1`],
  [`AT+CPSI?`], [Thông tin hệ thống bắt đầu bằng `LTE,Online`],
)
Module phản hồi được lệnh AT, SIM sẵn sàng và đã đăng ký mạng. Mã mạng `45201` gồm MCC 452 (Việt Nam) và MNC 01, tương ứng MobiFone. `AT+CIMI` trả về IMSI của thuê bao; IMEI là định danh riêng của thiết bị, được đọc trong phản hồi `ATI`.

== So sánh tín hiệu trong nhà và ngoài trời
#table(columns: (1fr, 1fr, 1fr),
  [Vị trí], [Phản hồi `AT+CSQ`], [Chỉ số RSSI],
  [Trong nhà], [`+CSQ: 19,99`], [19],
  [Ngoài trời], [`+CSQ: 22,99`], [22],
)
Mẫu ngoài trời có chỉ số RSSI cao hơn trong nhà 3 đơn vị. Trong điều kiện đo, tín hiệu thu ngoài trời tốt hơn; kết quả cho thấy môi trường đặt thiết bị có ảnh hưởng đến tín hiệu. Trường thứ hai đều bằng 99, không thể dùng để đánh giá BER thực tế.

= Gửi và nhận SMS

== Gửi SMS từ module tới điện thoại
Thiết lập chế độ văn bản bằng `AT+CMGF=1`, nhập số nhận với `AT+CMGS`, chờ dấu `>` rồi gửi nội dung. Kết thúc bằng Ctrl+Z thông qua lệnh `/send` trên Serial Monitor.

```text
AT+CMGF=1
OK
AT+CMGS="0856770806"
> Thoi gian: 09:50, Dia diem: 304-GD2
<Ctrl+Z>
+CMGS: 105
OK
```
Module báo gửi thành công; điện thoại nhận đúng tin nhắn có thời gian và địa điểm. Tin thử trước đó có nội dung `Dia diem: 304-GD2` cũng được nhận, tương ứng phản hồi `+CMGS: 104`.

== Nhận SMS phản hồi từ điện thoại
Điện thoại gửi tin về số SIM trong module. Khi liệt kê tin nhắn bằng `AT+CMGL`, module trả về bản tin tại ô nhớ 19:

```text
+CMGL: 19,"REC UNREAD","+84856770806","","26/10/22,09:50:59+28"
Gui lai tu dien thoai 09:51, Dia diem: 304-GD2
```
Nội dung nhận được khớp tin phản hồi từ điện thoại. `REC UNREAD` là trạng thái chưa đọc tại thời điểm liệt kê. Ảnh dưới xác nhận việc trao đổi SMS theo cả hai chiều.

#figure(
  image("assets/sms_received_sent.png", height: 75mm),
  caption: [Điện thoại nhận SMS từ module và gửi phản hồi chứa thời gian, địa điểm.],
) <sms-phone>

== Kiểm tra bộ nhớ và đọc tin theo ô nhớ

```text
AT+CPMS?
+CPMS: "SM",21,40,"SM",21,40,"SM",21,40
OK
AT+CMGR=19
+CMGR: "REC READ","+84856770806","","26/10/22,09:50:59+28"
Gui lai tu dien thoai 09:51, Dia diem: 304-GD2
OK
```

Phản hồi `CPMS` cho thấy cả ba cấu hình bộ nhớ SMS đang chọn `SM`, mỗi cấu hình báo 21 ô đã dùng trên tổng 40 ô. Lệnh `CMGR=19` đọc thành công tin phản hồi tại ô 19; trạng thái `REC READ` xác nhận tin được đánh dấu đã đọc, nội dung khớp kết quả liệt kê trước đó.

Sau khi lưu nội dung, xóa tin tại ô 19 và kiểm tra lại bộ nhớ:

```text
AT+CMGD=19
OK
AT+CPMS?
+CPMS: "SM",20,40,"SM",20,40,"SM",20,40
OK
```

Lệnh xóa trả `OK`; số ô đã dùng giảm từ 21 xuống 20, xác nhận đã xóa một tin nhắn.

= Thực hiện cuộc gọi

== Cuộc gọi từ module tới điện thoại
Module gọi tới số `0856770806` và kết thúc cuộc gọi bằng `ATH`. Các dòng log chính:

```text
ATD0856770806;
OK
+CLCC: 1,0,2,0,0,"0856770806",129,""
+CLCC: 1,0,3,0,0,"0856770806",129,""
VOICE CALL: BEGIN
+CLCC: 1,0,0,0,0,"0856770806",129,""
+COLP: "0856770806",129
ATH
OK
+CLCC: 1,0,6,0,0,"0856770806",129,""
VOICE CALL: END: 000012
NO CARRIER
```

#table(columns: (42mm, 1fr),
  [Thông báo], [Ý nghĩa],
  [`+CLCC` — trạng thái 2], [Cuộc gọi đi đang quay số.],
  [`+CLCC` — trạng thái 3], [Đang báo chuông phía nhận.],
  [`VOICE CALL: BEGIN`; trạng thái 0], [Cuộc gọi đã được thiết lập và đang hoạt động.],
  [`+COLP`], [Thông tin số điện thoại được kết nối.],
  [`ATH` → `OK`], [Module chấp nhận lệnh kết thúc cuộc gọi.],
  [Trạng thái 6; `VOICE CALL: END`], [Cuộc gọi đã kết thúc.],
  [`NO CARRIER`], [Liên kết cuộc gọi đã được giải phóng sau khi kết thúc.],
)

Kết quả xác nhận thiết lập và kết thúc thành công cuộc gọi từ module tới điện thoại. Các thông báo `+CGEV: NW ACT 8,10` và `NW DEACT 8,10` trong log đầy đủ là thông báo sự kiện dữ liệu gói; không dùng riêng các dòng này để kết luận trạng thái cuộc gọi.

== Cuộc gọi từ điện thoại tới module
Điện thoại `0856770806` gọi tới SIM trong module. Khi xuất hiện `RING`, nhập `ATA` để trả lời; sau đó kết thúc bằng `ATH`. Các dòng log chính:

```text
RING
+CLCC: 1,1,4,0,0,"0856770806",129,""
ATA
VOICE CALL: BEGIN
+CLCC: 1,1,0,0,0,"0856770806",129,""
OK
ATH
OK
+CLCC: 1,1,6,0,0,"0856770806",129,""
VOICE CALL: END: 000014
NO CARRIER
```

`RING` báo cuộc gọi đến. Trong `+CLCC`, trường chiều gọi bằng 1 xác định đây là cuộc gọi đến; trạng thái 4 là đang chờ trả lời, trạng thái 0 là cuộc gọi hoạt động và trạng thái 6 là kết thúc. Sau lệnh `ATA`, thông báo `VOICE CALL: BEGIN` xác nhận module đã trả lời thành công. Lệnh `ATH` được chấp nhận và cuộc gọi kết thúc bình thường, sau đó xuất hiện `NO CARRIER`.

Kết hợp với phép thử gọi đi, kết quả xác nhận thiết lập, trả lời và kết thúc cuộc gọi thành công theo cả hai chiều.

= Truy cập Internet qua SIM 4G

== Dữ liệu di động và ping
Lệnh `AT+CGATT?` trả `+CGATT: 1`, cho biết module đã gắn dịch vụ dữ liệu gói. Phép thử ping thu được:

```text
AT+CPING="www.google.com",1
OK
+CPING: 1,142.251.155.119,92,495,118
+CPING: 1,142.251.155.119,92,65,118
+CPING: 1,142.251.155.119,92,130,118
+CPING: 1,142.251.155.119,92,645,118
+CPING: 3,4,4,0,65,645,333
```

Tên miền được phân giải thành `142.251.155.119`; nhận đủ 4 phản hồi, không mất gói trong phép thử. Kết quả xác nhận truy cập được đích kiểm tra qua mạng di động.

== Trạng thái dịch vụ mạng và địa chỉ IP
Khi nhập `AT+NETOPEN`, module thông báo `Network is already opened`, cho biết dịch vụ mạng đã mở sẵn. Lệnh đọc IP trả về:

```text
AT+IPADDR
+IPADDR: 10.106.60.144
OK
```

== Kiểm chứng ping trước khi mở dịch vụ socket
Phép thử bổ sung xác nhận trạng thái socket đang đóng, thực hiện ping rồi mở lại dịch vụ:

```text
AT+NETOPEN?
+NETOPEN: 0
OK
AT+CPING="www.google.com",1
OK
+CPING: 1,142.251.156.119,92,145,117
+CPING: 1,142.251.156.119,92,115,117
+CPING: 1,142.251.156.119,92,60,117
+CPING: 1,142.251.156.119,92,60,117
+CPING: 3,4,4,0,60,145,95
AT+NETOPEN
OK
+NETOPEN: 0
```

Cần phân biệt hai phản hồi có cùng giá trị 0: `AT+NETOPEN?` trả trạng thái 0 nghĩa là dịch vụ đang đóng; còn lệnh thực thi `AT+NETOPEN` trả mã kết quả 0 nghĩa là mở thành công.

Sau khi xác nhận trạng thái đóng, `CPING` vẫn nhận đủ 4 phản hồi. Kết quả cho thấy phép thử ICMP bằng `CPING` không yêu cầu người dùng mở trước dịch vụ socket bằng `NETOPEN` trên firmware đang dùng. Thao tác đóng/mở dịch vụ socket không đồng nghĩa ngắt/bật đăng ký LTE; kết nối TCP/UDP vẫn cần khởi tạo dịch vụ tương ứng.

== Kết nối TCP tới broker
Mở kết nối ID 1 tới broker `test.mosquitto.org` trên cổng 1883:

```text
AT+CIPOPEN=1,"TCP","test.mosquitto.org",1883
OK
+CIPOPEN: 1,0
```

Truy vấn `AT+CIPOPEN?` ghi nhận kết nối đang mở:

```text
+CIPOPEN: 1,"TCP","54.36.178.49",1883,-1
```

Có một kết nối TCP đang mở, ID 1, tới `54.36.178.49:1883`; các ID 0 và 2–9 không có kết nối đang mở. Phản hồi `+CIPOPEN: 1,0` xác nhận thao tác mở thành công. Phép thử mới xác nhận kết nối TCP tới broker, chưa thực hiện phiên MQTT hoặc publish/subscribe.

== Ý nghĩa các lệnh kiểm tra Internet
Trong `CPING`, tham số sau địa chỉ là loại địa chỉ (`1`: IPv4), không phải số lần ping. Các tham số tùy chọn lần lượt là số lần ping, kích thước dữ liệu, khoảng cách gửi, thời gian chờ và TTL. Lần thử dùng mặc định 4 lần; bản tổng kết cho biết gửi/nhận/mất và RTT nhỏ nhất/lớn nhất/trung bình.

#block[
  #set par(justify: false)
  Lệnh `NETOPEN` khởi tạo dịch vụ socket; `NETOPEN?` đọc trạng thái. Trong lệnh `CIPOPEN`, các tham số lần lượt là ID kết nối, giao thức, máy chủ và cổng đích. Lệnh `CIPOPEN?` liệt kê các kết nối; ID từ 0 đến 9, tương ứng tối đa 10 kết nối. Lần thử ghi nhận một kết nối đang mở với ID 1.
]

= Kết luận
ESP32 đã điều khiển A7680C-LANS để gửi/nhận SMS, đọc và xóa tin nhắn, thực hiện cuộc gọi hai chiều và truy cập Internet. Tín hiệu ngoài trời có chỉ số RSSI cao hơn trong nhà. Hai phép thử ping đều nhận đủ 4 phản hồi; kết nối TCP tới broker trên cổng 1883 được thiết lập thành công. Phép thử bổ sung xác nhận `CPING` hoạt động trước khi mở dịch vụ socket bằng `NETOPEN`.


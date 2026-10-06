# Attendance System

基于 **STM32F103RCT6** 的嵌入式考勤系统，集成 **AS608 指纹识别、RC522 RFID 读卡、ILI9341 LCD 显示、蜂鸣器与 LED 状态反馈**，并支持通过串口与上位机进行数据交互。

本项目使用 **STM32CubeMX + STM32 HAL + Keil MDK-ARM** 开发，采用模块化 BSP 结构组织各硬件驱动与业务逻辑。

## ✨ 功能特性

- **指纹识别**
  - 基于 AS608 指纹识别模块
  - 指纹录入
  - 指纹验证
  - 指纹模板删除
  - 指纹库索引管理
  - 指纹匹配结果获取

- **RFID 读卡**
  - 基于 RC522 / MFRC522
  - 支持 MIFARE 卡片基本操作
  - 读取 RFID 卡 UID

- **LCD 显示**
  - 基于 ILI9341 控制器
  - 240 × 320 分辨率
  - 支持文字、图形、图片等基本显示功能
  - 支持中文字符显示

- **状态反馈**
  - LED 状态指示
  - 蜂鸣器成功/失败提示
  - 支持非阻塞式蜂鸣器任务处理

- **上位机通信**
  - 通过 USART 与上位机通信
  - 自定义通信帧格式
  - 支持工作模式切换
  - 支持指纹删除命令
  - 支持向上位机上报系统状态及操作结果

- **模块化设计**
  - STM32 HAL 外设驱动
  - 独立 BSP 层
  - 独立 AS608、RC522、LCD、LED、Buzzer 驱动
  - STM32CubeMX `.ioc` 工程配置与 Keil MDK 工程均包含在仓库中

---

## 🧩 系统组成

系统主要由以下模块组成：

```text
                    ┌──────────────────────┐
                    │      上位机/PC       │
                    │   UART 通信/控制     │
                    └──────────┬───────────┘
                               │ USART
                               │
                    ┌──────────▼───────────┐
                    │    STM32F103RCT6      │
                    │      主控制器        │
                    └──────┬───────┬────────┘
                           │       │
                ┌──────────┘       └──────────┐
                │                             │
         ┌──────▼──────┐               ┌──────▼──────┐
         │   AS608     │               │    RC522    │
         │  指纹模块   │               │   RFID模块   │
         └─────────────┘               └─────────────┘
                │                             │
                └──────────┬──────────────────┘
                           │
                    ┌──────▼──────┐
                    │  考勤业务逻辑 │
                    └──────┬──────┘
                           │
              ┌────────────┼────────────┐
              │            │            │
        ┌─────▼─────┐ ┌────▼─────┐ ┌───▼────────┐
        │ ILI9341   │ │  Buzzer  │ │    LED     │
        │ LCD显示   │ │ 蜂鸣器   │ │ 状态指示   │
        └───────────┘ └──────────┘ └────────────┘
```

---

## 🔧 硬件平台

| 模块      | 型号 / 说明              |
| --------- | ------------------------ |
| MCU       | STM32F103RCT6            |
| CPU       | ARM Cortex-M3            |
| 主频      | 72 MHz                   |
| 指纹模块  | AS608                    |
| RFID 模块 | RC522 / MFRC522          |
| LCD       | ILI9341，240 × 320       |
| 通信      | USART1 / USART2 / USART3 |
| SPI       | SPI1 / SPI2              |
| 状态指示  | LED                      |
| 声音提示  | Buzzer                   |

STM32CubeMX 工程中配置了：

- SPI1
- SPI2
- USART1
- USART2
- USART3
- GPIO
- EXTI
- SysTick
- SWD 调试接口

系统时钟配置为 **72 MHz**。

---

## 📁 项目结构

```text
Attendance_System/
├── Attendance_System.ioc       # STM32CubeMX 工程配置
│
├── Core/
│   ├── Inc/                   # STM32 应用层头文件
│   ├── Src/                   # STM32 应用层源文件
│   └── all_code.txt
│
├── Drivers/
│   ├── CMSIS/                 # ARM CMSIS
│   └── STM32F1xx_HAL_Driver/  # STM32F1 HAL 驱动
│
├── Hardware/
│   ├── AS608/                 # AS608 指纹模块驱动
│   ├── BSP/                   # 系统 BSP / 业务调度
│   ├── Buzzer/                # 蜂鸣器驱动
│   ├── LED/                   # LED 驱动
│   ├── OLED/                  # ILI9341 LCD 驱动
│   └── RC522/                 # RC522 RFID 驱动
│
└── MDK-ARM/
    ├── Attendance_System.uvprojx
    ├── DebugConfig/
    └── RTE/
```

> `Hardware/OLED` 目录中的显示驱动实际使用的是 **ILI9341 LCD 控制器**，并非传统意义上的 OLED 模块。

---

## ⚙️ 软件架构

项目采用较为典型的 STM32 分层结构：

```text
Application
     │
     ▼
   BSP 层
     │
 ┌───┼─────────────┐
 ▼   ▼             ▼
AS608 RC522      UI/反馈
 │    │          │
 ▼    ▼       LCD/LED/Buzzer
Hardware Drivers
     │
     ▼
STM32 HAL
     │
     ▼
STM32F103RCT6
```

主循环通过 `bsp_loop()` 统一调度系统业务。

系统主要存在两种工作状态：

```text
             ┌──────────────┐
             │     IDLE     │
             │  指纹验证模式 │
             └──────┬───────┘
                    │
              模式切换命令
                    │
                    ▼
             ┌──────────────┐
             │    ENROLL    │
             │  指纹录入模式 │
             └──────────────┘
```

在 `IDLE` 状态下执行指纹验证，在 `ENROLL` 状态下执行指纹录入。

系统同时持续轮询 RC522，用于检测 RFID 卡片。

---

## 🔌 通信接口

项目使用多个串口分别承担不同的通信任务：

| 接口   |   配置 | 用途                  |
| ------ | -----: | --------------------- |
| USART1 | 115200 | 串口通信              |
| USART2 |  57600 | 指纹模块相关通信      |
| USART3 | 115200 | 串口通信 / 上位机交互 |

上位机通信采用自定义协议。

代码中定义了部分协议控制字，例如：

```text
0xA1  IDLE_MODE       空闲/验证模式
0xA2  ENROLL_MODE     指纹录入模式
0xB1  TOGGLE_MODE     切换工作模式

0xA4  PRESSED         检测到手指
0xA5  RELEASED        手指离开

0xA6  DELETE_FAILED   指纹删除失败
0xA7  DELETE_SUCCEED  指纹删除成功

0xB2  DELETE_FINGER   删除指纹
```

通信协议采用帧头、命令、数据和帧尾的方式进行组织，便于 MCU 与上位机之间进行可靠的数据交互。

---

## 🖐️ 指纹考勤流程

### 指纹录入

```text
进入录入模式
     │
     ▼
读取手指图像
     │
     ▼
生成指纹特征
     │
     ▼
再次读取手指
     │
     ▼
生成第二份特征
     │
     ▼
合并特征生成模板
     │
     ▼
写入指纹库
     │
     ▼
录入完成
```

AS608 驱动提供了包括以下操作在内的完整接口：

- 获取指纹图像
- 生成特征
- 指纹匹配
- 高速搜索
- 合并模板
- 存储模板
- 删除模板
- 清空指纹库
- 读取系统参数
- 读取指纹库索引

### 指纹验证

```text
检测手指
   │
   ▼
获取指纹图像
   │
   ▼
生成指纹特征
   │
   ▼
搜索指纹库
   │
   ▼
获得 Page ID / Match Score
   │
   ▼
上报验证结果
```

---

## 💳 RFID 识别

RC522 模块通过 SPI 与 STM32 通信。

当前驱动包含：

- MFRC522 初始化
- 卡片寻卡
- 防冲撞
- 卡片选择
- 密钥认证
- 数据块读写
- MIFARE Value 操作
- 卡片休眠
- UID 获取

系统业务层通过 `RC522_ReadCardUID()` 对 RFID 卡进行检测。

---

## 🖥️ LCD 显示

显示模块采用 **ILI9341** 控制器，分辨率：

```text
240 × 320
```

驱动支持：

- 像素绘制
- 直线
- 矩形
- 圆形
- 区域填充
- 中英文字符串
- 图片显示
- 指定区域刷新
- 中文字符显示

---

## 🔔 蜂鸣器

蜂鸣器采用基于 `HAL_GetTick()` 的非阻塞任务机制。

支持：

```c
buzzer_success();
buzzer_error();
buzzer_beep(ms);
```

成功与失败可以产生不同的声音反馈，例如：

```text
成功：
████

失败：
██  ██
```

这种实现不会因为蜂鸣器延时而长时间阻塞主循环。

---

## 💡 LED

系统提供两个 LED 的基础控制接口：

```c
turn_on_led();
turn_off_led();
toggle_led();
```

用于显示系统运行状态及操作结果。

---

## 🛠️ 开发环境

### 推荐环境

- **STM32CubeMX 6.18.0**
- **STM32Cube FW_F1 V1.8.7**
- **Keil MDK-ARM 5.32**
- ARM Cortex-M3
- STM32F1xx HAL Library

仓库已经包含：

- `.ioc` CubeMX 配置文件
- Keil `.uvprojx` 工程文件
- CMSIS
- STM32F1 HAL Driver
- 项目所需的硬件驱动源码

因此可以直接使用 Keil 打开：

```text
MDK-ARM/Attendance_System.uvprojx
```

进行编译和下载。

---

## 🚀 编译与烧录

### 1. 克隆仓库

```bash
git clone https://github.com/Ice-light8006/Attendance_System.git
cd Attendance_System
```

### 2. 使用 Keil 打开工程

打开：

```text
MDK-ARM/Attendance_System.uvprojx
```

确认设备选择为：

```text
STM32F103RC
```

### 3. 编译

在 Keil 中执行：

```text
Build → Build Target
```

项目配置会生成：

```text
Attendance_System.hex
```

### 4. 下载

连接 ST-Link 等调试器，并根据实际硬件连接 AS608、RC522、ILI9341 LCD 等外设，然后通过 Keil 下载程序。

---

## 📌 当前状态

本项目目前处于持续开发阶段。

目前已经完成：

- [x] STM32F103RCT6 基础工程
- [x] STM32CubeMX 外设配置
- [x] AS608 指纹模块驱动
- [x] 指纹录入
- [x] 指纹验证
- [x] 指纹删除
- [x] 指纹库索引处理
- [x] RC522 RFID 驱动
- [x] RFID UID 读取
- [x] ILI9341 LCD 驱动
- [x] LED 驱动
- [x] 蜂鸣器驱动
- [x] 上位机通信框架
- [x] 工作模式切换
- [ ] 完整考勤记录存储
- [ ] 完整考勤数据管理
- [ ] 更完善的上位机软件
- [ ] 系统参数持久化

---

## 📄 License

本项目目前未在仓库根目录声明独立的项目 License。

仓库中部分 STM32/CMSIS 第三方组件包含其自身的许可证文件。使用或再发布相关第三方代码时，请遵循对应组件的许可证要求。

---

## 👤 Author

**Ice-light8006**

GitHub：

https://github.com/Ice-light8006

---

## ⭐ About This Project

这是一个以 STM32F1 为核心的嵌入式考勤系统项目，重点实践了 **指纹识别、RFID、串口通信、SPI、LCD 显示、BSP 分层以及嵌入式状态机/任务调度** 等技术。

适合作为 STM32、嵌入式系统、物联网工程以及智能考勤设备相关的学习与实践项目。
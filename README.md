# LIBMODBUS

## 1、介绍

libmodbus是一个与使用Modbus协议的设备进行数据发送/接收的库。该库包含各种后端通过不同网络进行通信（例如，RTU模式下的串口或TCP / IPv6中的以太网）。

libmodbus提供了较低通信层的抽象，并在所有支持的平台上提供相同的API。

本源码实现了在RT-Thread上的移植，支持MODBUS-RTU和MODBUS-TCP。源码地址：
<https://github.com/loogg/libmodbus>  作者：[loogg](https://github.com/loogg)

### 1.1、使用背景

Modbus 协议是应用于电子控制器上的一种通用语言。通过此协议，控制器相互之间、控制器经由网络（例如以太网）和其它设备之间可以通信。它已经成为一通用工业标准。有了它，不同厂商生产的控制设备可以连成工业网络，进行集中监控。此协议定义了一个控制器能认识使用的消息结构,而不管它们是经过何种网络进行通信的。它描述了一控制器请求访问其它设备的过程，如何回应来自其它设备的请求，以及怎样侦测错误并记录。它制定了消息域格局和内容的公共格式。当在一Modbus网络上通信时，此协议决定了每个控制器须要知道它们的设备地址，识别按地址发来的消息，决定要产生何种行动。如果需要回应，控制器将生成反馈信息并用Modbus 协议发出。在其它网络上，包含了 Modbus 协议的消息转换为在此网络上使用的帧或包结构。这种转换也扩展了根据具体的网络解决节地址、路由路径及错误检测的方法。



### 1.2、目录结构


| 名称 | 说明 |
| ---- | ---- |
| docs  | 文档目录 |
| examples | 例子目录，并有相应的一些说明 |
| inc  | 头文件目录 |
| src  | 源代码目录 |

### 1.3、许可证

libmodbus package 遵循 LGPLv2.1 许可，详见 `LICENSE` 文件。

### 1.4、依赖

- RT-Thread 4.0+
- Filesystem
- libc
- pin
- serial
- POSIX
- SAL

## 2、如何打开 libmodbus

使用 libmodbus package 需要在 RT-Thread 的包管理器中选择它，具体路径如下：

```
RT-Thread online packages
    IoT - internet of things --->
        [*] libmodbus: A Modbus library for RT-Thread
```

然后让 RT-Thread 的包管理器自动更新，或者使用 `pkgs --update` 命令更新包到 BSP 中。

## 3、使用 libmodbus

在打开 libmodbus package 后，当进行 bsp 编译时，它会被加入到 bsp 工程中进行编译。

### 3.1、创建modbus RTU 环境

- 初试化RTU环境指针 

```
modbus_t *modbus_new_rtu(const char *device, int baud,char parity, int data_bit, int stop_bit)
```

- 设置串口模式 

```
int modbus_rtu_set_serial_mode(modbus_t *ctx, int mode)
```

- 在RTU环境下设置RTS模式,为RS485模式下设置控制引脚

```
int modbus_rtu_set_rts(modbus_t *ctx, int rts_pin, int mode)
```

| mode | 说明 |
| ---- | ---- |
| MODBUS_RTU_RTS_NONE  | 无控制 |
| MODBUS_RTU_RTS_UP | 高电平为发送模式 |
| MODBUS_RTU_RTS_DOWN | 低电平为发送模式 |

### 3.2、创建modbus TCP 环境

- 初始化TCP环境指针 

```
modbus_t *modbus_new_tcp(const char *ip, int port)
```

- 作为客户端连接服务器

```
int modbus_connect(modbus_t *ctx)
```

- 作为服务器监听

```
int modbus_tcp_listen(modbus_t * ctx, int nb_connection)

int modbus_tcp_accept(modbus_t * ctx, int * s)
```

### 3.3、示例
MODBUS-RTU和MODBUS-TCP使用示例在 [/examples](/examples) 下，使用方法与原生libmodbus一致。

## 4、注意事项

在使用RTU-RS485模式时，先设置引脚模式，再设置RS485模式，再设置控制引脚
```
rt_pin_mode(MAX3491_RE, PIN_MODE_OUTPUT);
modbus_t *ctx = RT_NULL;
ctx = modbus_new_rtu("/dev/uart2", 115200, 'N', 8, 1);
modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS485);
modbus_rtu_set_rts(ctx, MAX3491_RE, MODBUS_RTU_RTS_UP);
```
其余API和libmodbus官方一致

## 5、联系方式 & 感谢

* 维护：malongwei
* 主页：<https://github.com/loogg/libmodbus>
* 邮箱：<2544047213@qq.com>
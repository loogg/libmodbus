# LIBMODBUS

## 1、介绍

libmodbus是一个与使用Modbus协议的设备进行数据发送/接收的库。该库包含各种后端通过不同网络进行通信（例如，RTU模式下的串口或TCP / IPv6中的以太网）。

libmodbus提供了较低通信层的抽象，并在所有支持的平台上提供相同的API。

本源码实现了在RT-Thread上的移植，支持MODBUS-RTU和MODBUS-TCP。源码地址：
<https://github.com/loogg/libmodbus>  作者：[loogg](https://github.com/loogg)

### 1.1 目录结构


| 名称 | 说明 |
| ---- | ---- |
| docs  | 文档目录 |
| examples | 例子目录，并有相应的一些说明 |
| inc  | 头文件目录 |
| src  | 源代码目录 |

### 1.2 许可证

libmodbus package 遵循 LGPLv2.1 许可，详见 `LICENSE` 文件。

### 1.3 依赖

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
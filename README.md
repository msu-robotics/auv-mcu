# AUV-MCU
Репозиторий содержит прошивку для микроконтроллера ESP32


# Настрока среды разработки
Для работы с microros в platformio необходимо настроить среду разработки согластно инструкции https://github.com/micro-ROS/micro_ros_platformio


# Распиновка ESP32


|             | Left side<br>(USB down) | GPIO   | GPIO   | Right<br>side |               |
| ----------- | ----------------------- | ------ | ------ | ------------- | ------------- |
|             | 3V3                     |        |        | GND           |               |
|             | EN (Restart)            |        | GPIO23 | G23           |               |
|             | SP                      | GPIO36 | GPIO22 | G22           | Tr1           |
|             | SN                      | GPIO39 | GPIO1  | TX0           |               |
|             | G34                     | GPIO34 | GPIO3  | RX0           |               |
|             | G35                     | GPIO34 | GPIO21 | G21           | Tr2           |
| i2c sda     | G32                     | GPIO32 |        | GND           |               |
| i2c sdc     | G33                     | GPIO33 | GPIO19 | G19           | Tr4           |
|             | G25                     | GPIO25 | GPIO18 | G18           | Tr5           |
| imu uart tx | G26                     | GPIO26 | GPIO5  | G5            | Tr3           |
| imu uart rx | G27                     | GPIO27 | GPIO17 | G17           | debug uart tx |
|             | G14                     | GPIO14 | GPIO16 | G16           | debug uart rx |
|             | G12                     | GPIO12 | GPIO4  | G4            |               |
|             | GND                     |        | GPIO0  | G0            |               |
|             | G13                     | GPIO13 | GPIO2  | G2            |               |
|             | SD2                     | GPIO9  | GPIO15 | G15           |               |
|             | SD3                     | GPIO10 | GPIO08 | SD1           |               |
|             | CMD                     | GPIO11 | GPIO7  | SD0           |               |
|             | 5V                      |        | GPIO6  | CLK           |               |


# Конфигурации

Прошивку можно собрать с различным набором датчиков, например в качестве IMU:

- MPU6050
- HTW905-232

В качестве датчика глубины можно использовать:

- BMP180
- MS5837-30BA

В качестве магнитометра можно использовать:

- QMC5883L
- HTW905-232

Что бы задать то какой датчик использовать для какой задачи, необходимо в `platformio.ini` указать флаг сборки

```
build_flags = -DIMU_MPU6050
```

в такой конфигурации в качестве IMU будет использоваться MPU6050

Возможные варианты:

- IMU_GY87 | IMU_HWT905
- MAG_QMC5883L | MAG_HWT905
- DEPTH_BMP180 | DEPTH_MS5837
- TEMP_BMP180 | TEMP_MS5837

# Удаленная отладка и прошивка микроконтроллеар
В Platformio есть функция remote agent, что бы ты мог заливать прошивку в микроконтроллер который подключен к Jetson не перетыкая его к себе в пк, и не разбирать аппарат.
Что бы все настроить необходимо выполнить определенные действия в твоей IDE и на Jetson.

 ## Действи я которые необходимо сделать на ПК с Platformio
Необходимо создать аккаунт в Platformio если у тебя его еще нет, после создания аккаунта, необходимо залогиниться в IDE в него.

![Platformio Account](docs/pio%20account.png)

После того как ты авторизовался, необходимо войти в pio cli, для этого перейди:

![Platformio CLI](docs/pio%20cli.png)

После этого откроется терминал где необходимо выполнить команду

```bash
pio account token
```

У тебя запросят пароль от твоего аккаунта Platformio (не SUDO), если ты все сделал правильно, то тебе напечатает токен авторизации, сохрани его в надежном месте!

## Действия которые необходимо выполнить на Jetson
Необходимо установить необходимую зависимость:
```bash
sudo apt-get install python3.12-venv
```
после этого можно установить [platformio cli](https://docs.platformio.org/en/stable/core/installation/methods/installer-script.html)

подключить к jetson по ssh, начни tmux сессию, и сделай следующее:

```bash
export PLATFORMIO_AUTH_TOKEN="тут твой токен"
pio remote agent start -n jetson
```

если все хорошо то бует так:

```
2025-08-03 11:24:27 [info] Connecting to PlatformIO Remote Development Cloud
2025-08-03 11:24:28 [info] Successfully connected
2025-08-03 11:24:28 [info] Authenticating
2025-08-03 11:24:29 [info] Successfully authorized
```

После успешного запуска, в ide ты можешь сделать

```bash
pio remote device list
```
и увидеть список доступных портов на агенте

```
Agent utm-vm
...
тут будет много лишнего
...
/dev/ttyUSB0
------------
Hardware ID: USB VID:PID=10C4:EA60 SER=0001 LOCATION=5-3
Description: CP2102 USB to UART Bridge Controller - CP2102 USB to UART Bridge Controller
```

## Загрузка прошивки
После того как все настроено загрузить прошивку можно командой

Предварительно незабудь поменять порт загрузки для окружения в `platformio.ini`  в моем примере с `/dev/cu.usbserial-0001` на `/dev/ttyUSB0`

```bash
pio remote run -e esp32doit-ros -t upload
```

`esp32doit-ros` - это env который перечислены в файле `platformio.ini`

в процессе загрузки может возникнуть проблема

```
([Errno 13] could not open port /dev/ttyUSB0: [Errno 13] Permission denied: '/dev/ttyUSB0')
```

для этого добавь пользователя в группу

```
sudo usermod -aG dialout $USER
```
и перезагрузи


# Процедура тестирования и отладки
Необходимо собрать прошивку для окружения `esp32doit-teleplot` с указанием
необходимых флагов, определяющих набор датчиков. После прошивки открыть Teleplot и убедится в корректности данных с датчиков.

После того как датчики полностью работают, можно союрать и залить прошивку для `esp32doit-ros`, после этого запустить ноду для microros_agent, и проверить через `ros2 topic list` что появились потики с данными, проверить содержимое топиков данных.
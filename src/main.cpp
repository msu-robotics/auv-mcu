#include <Arduino.h>
#include <micro_ros_platformio.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/int32.h>

#include <REG.h>
#include <wit_c_sdk.h>

#define ACC_UPDATE 0x01
#define GYRO_UPDATE 0x02
#define ANGLE_UPDATE 0x04
#define MAG_UPDATE 0x08
#define READ_UPDATE 0x80
static volatile char s_cDataUpdate = 0, s_cCmd = 0xff;

static void CmdProcess(void);
static void AutoScanSensor(void);
static void SensorUartSend(uint8_t *p_data, uint32_t uiSize);
static void SensorDataUpdata(uint32_t uiReg, uint32_t uiRegNum);
static void Delayms(uint16_t ucMs);
const uint32_t c_uiBaud[8] = {0, 4800, 9600, 19200, 38400, 57600, 115200, 230400};

rcl_publisher_t publisher;
std_msgs__msg__Int32 msg;

rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_timer_t timer;

#define LED_PIN 13
#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}

void error_loop()
{
	while (1)
	{
		digitalWrite(LED_PIN, !digitalRead(LED_PIN));
		delay(100);
	}
}

void timer_callback(rcl_timer_t *timer, int64_t last_call_time)
{
	RCLC_UNUSED(last_call_time);
	if (timer != NULL)
	{
		RCSOFTCHECK(rcl_publish(&publisher, &msg, NULL));
		msg.data++;
	}
}

void CopeCmdData(unsigned char ucData)
{
	static unsigned char s_ucData[50], s_ucRxCnt = 0;

	s_ucData[s_ucRxCnt++] = ucData;
	if (s_ucRxCnt < 3)
		return; // Less than three data returned
	if (s_ucRxCnt >= 50)
		s_ucRxCnt = 0;
	if (s_ucRxCnt >= 3)
	{
		if ((s_ucData[1] == '\r') && (s_ucData[2] == '\n'))
		{
			s_cCmd = s_ucData[0];
			memset(s_ucData, 0, 50);
			s_ucRxCnt = 0;
		}
		else
		{
			s_ucData[0] = s_ucData[1];
			s_ucData[1] = s_ucData[2];
			s_ucRxCnt = 2;
		}
	}
}

static void SensorUartSend(uint8_t *p_data, uint32_t uiSize)
{
	Serial1.write(p_data, uiSize);
	Serial1.flush();
}
static void Delayms(uint16_t ucMs)
{
	delay(ucMs);
}
static void SensorDataUpdata(uint32_t uiReg, uint32_t uiRegNum)
{
	int i;
	for (i = 0; i < uiRegNum; i++)
	{
		switch (uiReg)
		{
		case AZ:
			s_cDataUpdate |= ACC_UPDATE;
			break;
		case GZ:
			s_cDataUpdate |= GYRO_UPDATE;
			break;
		case HZ:
			s_cDataUpdate |= MAG_UPDATE;
			break;
		case Yaw:
			s_cDataUpdate |= ANGLE_UPDATE;
			break;
		default:
			s_cDataUpdate |= READ_UPDATE;
			break;
		}
		uiReg++;
	}
}

static void AutoScanSensor(void)
{
	int i, iRetry;

	for (i = 0; i < sizeof(c_uiBaud) / sizeof(c_uiBaud[0]); i++)
	{
		Serial1.begin(c_uiBaud[i], SERIAL_8N1, 27, 26);
		Serial1.flush();
		iRetry = 2;
		s_cDataUpdate = 0;
		do
		{
			WitReadReg(AX, 3);
			delay(200);
			while (Serial1.available())
			{
				WitSerialDataIn(Serial1.read());
			}
			iRetry--;
		} while (iRetry);
	}
}

void setup()
{
	Serial.begin(115200);
	set_microros_serial_transports(Serial);
	pinMode(LED_PIN, OUTPUT);
	digitalWrite(LED_PIN, HIGH);

	delay(2000);

	allocator = rcl_get_default_allocator();

	// create init_options
	RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

	// create node
	RCCHECK(rclc_node_init_default(&node, "micro_ros_arduino_node", "", &support));

	// create publisher
	RCCHECK(rclc_publisher_init_default(
		&publisher,
		&node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
		"micro_ros_arduino_node_publisher"));

	// create timer,
	const unsigned int timer_timeout = 1000;
	RCCHECK(rclc_timer_init_default(
		&timer,
		&support,
		RCL_MS_TO_NS(timer_timeout),
		timer_callback));

	// create executor
	RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
	RCCHECK(rclc_executor_add_timer(&executor, &timer));

	msg.data = 0;

	WitInit(WIT_PROTOCOL_NORMAL, 0x50);
	WitSerialWriteRegister(SensorUartSend);
	WitRegisterCallBack(SensorDataUpdata);
	WitDelayMsRegister(Delayms);
	AutoScanSensor();
}

int i;
float fAcc[3], fGyro[3], fAngle[3];

void loop()
{
	delay(100);
	RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));

	while (Serial1.available())
	{
		WitSerialDataIn(Serial1.read());
	}

	if (s_cDataUpdate)
	{
		for (i = 0; i < 3; i++)
		{
			fAcc[i] = sReg[AX + i] / 32768.0f * 16.0f;
			fGyro[i] = sReg[GX + i] / 32768.0f * 2000.0f;
			fAngle[i] = sReg[Roll + i] / 32768.0f * 180.0f;
		}
		if (s_cDataUpdate & ACC_UPDATE)
		{
			// fAcc[0], fAcc[1], fAcc[2]

			s_cDataUpdate &= ~ACC_UPDATE;
		}
		if (s_cDataUpdate & GYRO_UPDATE)
		{
			// fGyro[0], fGyro[1], fGyro[2]

			s_cDataUpdate &= ~GYRO_UPDATE;
		}
		if (s_cDataUpdate & ANGLE_UPDATE)
		{
			// fAngle[0], fAngle[1], fAngle[2]
			// sReg[q0], sReg[q1], sReg[q2], sReg[q3]

			s_cDataUpdate &= ~ANGLE_UPDATE;
		}
		if (s_cDataUpdate & MAG_UPDATE)
		{
			// sReg[HX], sReg[HY], sReg[HZ]

			s_cDataUpdate &= ~MAG_UPDATE;
		}
		s_cDataUpdate = 0;
	}
}

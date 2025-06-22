#include <Arduino.h>
#include <micro_ros_platformio.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <sensor_msgs/msg/imu.h>
#include <std_msgs/msg/int32.h>

#include <REG.h>
#include <wit_c_sdk.h>

#define ACC_UPDATE 0x01
#define GYRO_UPDATE 0x02
#define ANGLE_UPDATE 0x04
#define MAG_UPDATE 0x08
#define READ_UPDATE 0x80
static volatile char s_cDataUpdate = 0, s_cCmd = 0xff;

#define LED_PIN 2
#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}

static void SensorUartSend(uint8_t *p_data, uint32_t uiSize);
static void SensorDataUpdata(uint32_t uiReg, uint32_t uiRegNum);
static void Delayms(uint16_t ucMs);
const uint32_t c_uiBaud[8] = {0, 4800, 9600, 19200, 38400, 57600, 115200, 230400};

rcl_allocator_t allocator;
rclc_support_t support;
rcl_node_t node;
rcl_publisher_t pub_imu;
rcl_publisher_t pub_temp;
rcl_timer_t timer_imu;
rcl_timer_t timer_temp;
rclc_executor_t executor;


sensor_msgs__msg__Imu imu_msg;
std_msgs__msg__Int32 temp_msg;

void error_loop()
{
	while (1)
	{
		digitalWrite(LED_PIN, !digitalRead(LED_PIN));
		delay(100);
	}
}

void imu_callback(rcl_timer_t *timer, int64_t last_call_time)
{
	RCLC_UNUSED(last_call_time);

	if (timer != NULL)
	{
		if (s_cDataUpdate)
		{
			// Заполняем imu_msg

			float fAcc[3], fGyro[3], fAngle[3];
			for (int i = 0; i < 3; i++)
			{
				fAcc[i] = sReg[AX + i] / 32768.0f * 16.0f;
				fGyro[i] = sReg[GX + i] / 32768.0f * 2000.0f;
				fAngle[i] = sReg[Roll + i] / 32768.0f * 180.0f;
			}

			// Orientation (кватернионы)
			imu_msg.orientation.w = sReg[q0];
			imu_msg.orientation.x = sReg[q1];
			imu_msg.orientation.y = sReg[q2];
			imu_msg.orientation.z = sReg[q3];

			// Angular velocity (рад/с)
			imu_msg.angular_velocity.x = fGyro[0];
			imu_msg.angular_velocity.y = fGyro[1];
			imu_msg.angular_velocity.z = fGyro[2];

			// Linear acceleration (м/с²)
			imu_msg.linear_acceleration.x = fAcc[0];
			imu_msg.linear_acceleration.y = fAcc[1];
			imu_msg.linear_acceleration.z = fAcc[2];

			// Publish
			s_cDataUpdate = 0;
		}
		RCSOFTCHECK(rcl_publish(&pub_imu, &imu_msg, NULL));
	}
}

void temp_callback(rcl_timer_t *, int64_t)
{
	static int temp = 25;
	temp_msg.data = temp++;
	RCSOFTCHECK(rcl_publish(&pub_temp, &temp_msg, NULL));
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
	for (int i = 0; i < uiRegNum; i++)
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

	set_microros_serial_transports(Serial);

	allocator = rcl_get_default_allocator();
	RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

	RCCHECK(rclc_node_init_default(&node, "esp32_node", "", &support));

	RCCHECK(rclc_publisher_init_default(
		&pub_imu, &node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Imu),
		"/imu/data"));

	RCCHECK(rclc_publisher_init_default(
		&pub_temp, &node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
		"/temperature"));

	RCCHECK(rclc_timer_init_default(
		&timer_imu, &support, RCL_MS_TO_NS(1000), imu_callback));

	RCCHECK(rclc_timer_init_default(
		&timer_temp, &support, RCL_MS_TO_NS(2000), temp_callback));

	RCCHECK(rclc_executor_init(&executor, &support.context, 2, &allocator));
	RCCHECK(rclc_executor_add_timer(&executor, &timer_imu));
	RCCHECK(rclc_executor_add_timer(&executor, &timer_temp));

	WitInit(WIT_PROTOCOL_NORMAL, 0x50);
	WitSerialWriteRegister(SensorUartSend);
	WitRegisterCallBack(SensorDataUpdata);
	WitDelayMsRegister(Delayms);
	AutoScanSensor();
}

void loop()
{
	delay(10);
	RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));

	while (Serial1.available())
	{
		WitSerialDataIn(Serial1.read());
	}
}

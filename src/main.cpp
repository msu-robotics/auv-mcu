#include <Arduino.h>
#include <micro_ros_platformio.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <sensor_msgs/msg/imu.h>
#include <std_msgs/msg/int32.h>

#include <REG.h>
#include <wit_c_sdk.h>

static volatile char s_cDataUpdate = 0;
static volatile char s_initialized = 0;

#define LED_PIN 2
#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}

#define ACC_UPDATE 0x01
#define GYRO_UPDATE 0x02
#define ANGLE_UPDATE 0x04
#define MAG_UPDATE 0x08
#define READ_UPDATE 0x80

static void SensorUartSend(uint8_t *p_data, uint32_t uiSize);
static void SensorDataUpdata(uint32_t uiReg, uint32_t uiRegNum);
static void Delayms(uint16_t ucMs);
const uint32_t c_uiBaud[8] = {0,4800, 9600, 19200, 38400, 57600, 115200, 230400};

void sensorTask(void *param);
void rosExecutorTask(void *param);
void blink_led(int8_t blink_num);

rcl_allocator_t allocator;
rclc_support_t support;
rcl_node_t node;

// Publishers
rcl_publisher_t pub_imu;
rcl_publisher_t pub_temp;

// Timers
rcl_timer_t timer_imu;
rcl_timer_t timer_temp;

rclc_executor_t executor;

// Messages
sensor_msgs__msg__Imu imu_msg;
std_msgs__msg__Int32 temp_msg;

void error_loop()
{
	while (1)
	{
		blink_led(3);
		delay(1000);
	}
}

void imu_callback(rcl_timer_t *timer, int64_t last_call_time)
{
	RCLC_UNUSED(last_call_time);

	if (s_cDataUpdate & s_initialized)
	{
		float fAcc[3], fGyro[3], fAngle[3], fMagnet[3], fQ[4];

		for (int i = 0; i < 3; i++)
		{
			fAcc[i] = sReg[AX + i] / 32768.0f * 16.0f;
			fGyro[i] = sReg[GX + i] / 32768.0f * 2000.0f;
			fAngle[i] = sReg[Roll + i] / 32768.0f * 180.0f;
			fMagnet[i] = sReg[HX + i];
		}
		for (int i = 0; i < 4; i++)
		{
			fQ[i] = sReg[q1 + i];
		}
		s_cDataUpdate = 0;

		// Orientation (кватернионы)
		imu_msg.orientation.w = fQ[0];
		imu_msg.orientation.x = fQ[1];
		imu_msg.orientation.y = fQ[2];
		imu_msg.orientation.z = fQ[3];

		const double ori_covar = 7.6e-7;
		imu_msg.orientation_covariance[0] = ori_covar;
		imu_msg.orientation_covariance[1] = 0;
		imu_msg.orientation_covariance[2] = 0;

		imu_msg.orientation_covariance[3] = 0;
		imu_msg.orientation_covariance[4] = ori_covar;
		imu_msg.orientation_covariance[5] = 0;

		imu_msg.orientation_covariance[6] = 0;
		imu_msg.orientation_covariance[7] = 0;
		imu_msg.orientation_covariance[8] = ori_covar;

		// Angular velocity (рад/с)
		imu_msg.angular_velocity.x = fGyro[0];
		imu_msg.angular_velocity.y = fGyro[1];
		imu_msg.angular_velocity.z = fGyro[2];

		const double ang_covar = 7.6e-7;
		imu_msg.angular_velocity_covariance[0] = ang_covar;
		imu_msg.angular_velocity_covariance[1] = 0;
		imu_msg.angular_velocity_covariance[2] = 0;

		imu_msg.angular_velocity_covariance[3] = 0;
		imu_msg.angular_velocity_covariance[4] = ang_covar;
		imu_msg.angular_velocity_covariance[5] = 0;

		imu_msg.angular_velocity_covariance[6] = 0;
		imu_msg.angular_velocity_covariance[7] = 0;
		imu_msg.angular_velocity_covariance[8] = ang_covar;

		// Linear acceleration (м/с²)
		imu_msg.linear_acceleration.x = fAcc[0];
		imu_msg.linear_acceleration.y = fAcc[1];
		imu_msg.linear_acceleration.z = fAcc[2];

		const double acc_covar = 2.4e-5;
		imu_msg.linear_acceleration_covariance[0] = acc_covar;
		imu_msg.linear_acceleration_covariance[1] = 0.0;
		imu_msg.linear_acceleration_covariance[2] = 0.0;

		imu_msg.linear_acceleration_covariance[3] = 0.0;
		imu_msg.linear_acceleration_covariance[4] = acc_covar;
		imu_msg.linear_acceleration_covariance[5] = 0.0;

		imu_msg.linear_acceleration_covariance[6] = 0.0;
		imu_msg.linear_acceleration_covariance[7] = 0.0;
		imu_msg.linear_acceleration_covariance[8] = acc_covar;

	}
	RCSOFTCHECK(rcl_publish(&pub_imu, &imu_msg, NULL));
}

void blink_led(int8_t blink_num) {
	for (int i = 0; i < blink_num; i++){
		delay(100);
		digitalWrite(LED_PIN, !digitalRead(LED_PIN));
		delay(100);
		digitalWrite(LED_PIN, !digitalRead(LED_PIN));
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
	Serial2.write(p_data, uiSize);
	Serial2.flush();
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
	Serial2.begin(9600, SERIAL_8N1, 27, 26);
	Serial2.flush();
	int i, iRetry;
	iRetry = 2;
	s_cDataUpdate = 0;
	do
	{
		WitReadReg(AX, 3);
		delay(200);
		while (Serial2.available())
		{
			WitSerialDataIn(Serial2.read());
		}
		if(s_cDataUpdate != 0)
		{
			blink_led(1);
			s_initialized = 1;
			return;
		}
		iRetry--;
	} while (iRetry);
	blink_led(3);
}

void setup()
{
	Serial.begin(115200);

	set_microros_serial_transports(Serial);

	pinMode(LED_PIN, OUTPUT);
	digitalWrite(LED_PIN, HIGH);

	delay(500);

	blink_led(1);

	delay(1000);

	set_microros_serial_transports(Serial);

	allocator = rcl_get_default_allocator();
	RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

	// Create node
	RCCHECK(rclc_node_init_default(&node, "esp32_node", "", &support));

	// Create imu publisher
	RCCHECK(rclc_publisher_init_default(
		&pub_imu, &node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Imu),
		"/sensors/imu"));

	RCCHECK(rclc_publisher_init_default(
		&pub_temp, &node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
		"/sensors/temperature"));

	// initialize imu timer
	RCCHECK(rclc_timer_init_default(
		&timer_imu, &support, RCL_MS_TO_NS(100), imu_callback));

	RCCHECK(rclc_timer_init_default(
		&timer_temp, &support, RCL_MS_TO_NS(2000), temp_callback));

	RCCHECK(rclc_executor_init(&executor, &support.context, 2, &allocator));
	RCCHECK(rclc_executor_add_timer(&executor, &timer_imu));
	RCCHECK(rclc_executor_add_timer(&executor, &timer_temp));

	WitInit(WIT_PROTOCOL_NORMAL, 0x50);
	WitSerialWriteRegister(SensorUartSend);
	WitRegisterCallBack(SensorDataUpdata);
	WitDelayMsRegister(Delayms);

	delay(500);
	blink_led(2);
	delay(500);

	AutoScanSensor();

	// Create tasks
	xTaskCreatePinnedToCore(sensorTask, "Sensor Reader", 4096, NULL, 1, NULL, 0);
	xTaskCreatePinnedToCore(rosExecutorTask, "Printer Task", 2048, NULL, 1, NULL, 1);
}

void sensorTask(void *param)
{
	while (true)
	{
		vTaskDelay(pdMS_TO_TICKS(5));

		while (Serial2.available())
		{
			WitSerialDataIn(Serial2.read());
		}
	}
}


void rosExecutorTask(void *param)
{
	while (true){
		vTaskDelay(pdMS_TO_TICKS(10));
		RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
	}
}

void loop()
{

}

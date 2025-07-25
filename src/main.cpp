#include <Arduino.h>
#include <utils.h>
#include <tasks/sensor_tasks.h>
#include <micro_ros_platformio.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <Wire.h>

#include <sensor_msgs/msg/imu.h>
#include <sensor_msgs/msg/temperature.h>
#include <sensor_msgs/msg/magnetic_field.h>
#include <sensor_msgs/msg/fluid_pressure.h>
#include <geometry_msgs/msg/wrench.h>
#include <std_msgs/msg/int32.h>
#include "Thruster.h"
#include "ThrusterAllocator.h"
#include <vector>


#define LED_PIN 2
#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){errorHandler();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}

void rosExecutorTask(void *param);
void blinkLed(int8_t blink_num);

rcl_allocator_t rcl_allocator;
rclc_support_t support;
rcl_node_t node;

rcl_publisher_t pub_imu;
rcl_publisher_t pub_magnetic;
rcl_publisher_t pub_fluid_pressure;
rcl_publisher_t pub_temp;
rcl_subscription_t sub_wrench;

rcl_timer_t timer_imu;
rcl_timer_t timer_fluid_pressure;
rcl_timer_t timer_temp;

rclc_executor_t executor;

sensor_msgs__msg__Imu imu_msg;
sensor_msgs__msg__MagneticField magnetic_msg;
sensor_msgs__msg__FluidPressure fluid_pressure_msg;
sensor_msgs__msg__Temperature fluid_temperature_msg;
std_msgs__msg__Int32 temp_msg;
geometry_msgs__msg__Wrench wrench_msg;


Thruster thr1(22, 0);
Thruster thr2(21, 1);
Thruster thr3(5, 2);
Thruster thr4(19, 3);
Thruster thr5(18, 4);

ThrusterAllocator allocator(6);


void errorHandler()
{
	uartDebug("❌Something went wrong, restart MCU");

	blinkLed(3);
	ESP.restart();

}

void imuCallback(rcl_timer_t *timer, int64_t last_call_time)
{
	RCLC_UNUSED(last_call_time);

	if (imu_data_updated && imu_initialized) {
		float fAcc[3], fGyro[3], fAngle[3], fMagnet[3], fQ[4];

		fillArray(fAcc, AX, 3, 16.0f / 32768.0f);
		fillArray(fGyro, GX, 3, 2000.0f / 32768.0f);
		fillArray(fAngle, Roll, 3, 180.0f / 32768.0f);
		fillArray(fMagnet, HX, 3);
		fillArray(fQ, q1, 4);

		imu_data_updated = 0;

		fillQuaternion(imu_msg.orientation, fQ);
		fillCovariance(imu_msg.orientation_covariance, 7.6e-7);

		fillVec3(imu_msg.angular_velocity, fGyro);
		fillCovariance(imu_msg.angular_velocity_covariance, 7.6e-7);

		fillVec3(imu_msg.linear_acceleration, fAcc);
		fillCovariance(imu_msg.linear_acceleration_covariance, 2.4e-5);

		fillVec3(magnetic_msg.magnetic_field, fMagnet);
		fillCovariance(magnetic_msg.magnetic_field_covariance, 1.69e-10);
	}
	RCSOFTCHECK(rcl_publish(&pub_imu, &imu_msg, NULL));
	RCSOFTCHECK(rcl_publish(&pub_magnetic, &magnetic_msg, NULL));
}

void thrustersCallback(const void *msgin){
	const geometry_msgs__msg__Wrench *msg = (const geometry_msgs__msg__Wrench *)msgin;

}

void blinkLed(int8_t blink_num)
{
	for (int i = 0; i < blink_num; i++)
	{
		delay(100);
		digitalWrite(LED_PIN, !digitalRead(LED_PIN));
		delay(100);
		digitalWrite(LED_PIN, !digitalRead(LED_PIN));
	}
}

void temperatureCallback(rcl_timer_t *, int64_t)
{
	temp_msg.data = depth_sensor.temperature();
	RCSOFTCHECK(rcl_publish(&pub_temp, &temp_msg, NULL));
}


void fluidPressureCallback(rcl_timer_t *, int64_t)
{
	fluid_pressure_msg.fluid_pressure = depth_sensor.pressure() / 1000;
	RCSOFTCHECK(rcl_publish(&pub_fluid_pressure, &fluid_pressure_msg, NULL));
}


void setup()
{
	Serial.begin(115200);
	Serial2.begin(115200, SERIAL_8N1);

	uartDebug("ℹ️ MCU setup started");

	bool success = Wire.begin(33, 32);
	if (success) {
		uartDebug("✅I2C initialized");
	} else {
		uartDebug("❌I2C initialize error");
	}

	initDepthSensor();
	initIMU();

	set_microros_serial_transports(Serial);

	pinMode(LED_PIN, OUTPUT);
	digitalWrite(LED_PIN, HIGH);

	blinkLed(1);
	delay(1000);

	uartDebug("ℹ️ Motors configuration");

	allocator.setThrusters({&thr1, &thr2, &thr3, &thr4, &thr5});
	// матрица распределения (веса thruster'ов на Fx, Fy, Fz, Mx, My, Mz)
	allocator.setAllocationMatrix({
	//  Fx.    Fy.   Fz.   Mx.   My.   Mz
        {0.0,  0.0,  0.0,  0.0,  0.0,  1.0},  // Thruster 1
        {0.0,  0.0,  0.0,  0.0,  0.0,  0.0},  // Thruster 2
        {0.0,  0.0,  0.0,  0.0,  0.0,  0.0},  // Thruster 3
        {0.0,  0.0,  0.0,  0.0,  0.0,  0.0},  // Thruster 4
		{0.0,  0.0,  0.0,  0.0,  0.0,  0.0}   // Thruster 5
    });
	allocator.setCorrectionFactors({1.0, 1.0, 1.1});
    allocator.setReverseFlags({false, true, false});

	thr1.setup();
	thr2.setup();
	thr3.setup();
	thr4.setup();
	thr5.setup();

	rcl_allocator = rcl_get_default_allocator();
	RCCHECK(rclc_support_init(&support, 0, NULL, &rcl_allocator));

	// Create node
	uartDebug("ℹ️ Create ROS nodes");
	RCCHECK(rclc_node_init_default(&node, "esp32_node", "", &support));

	// Create imu publisher
	RCCHECK(rclc_publisher_init_default(
		&pub_imu, &node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Imu),
		"/sensors/imu"));

	RCCHECK(rclc_publisher_init_default(
		&pub_magnetic, &node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, MagneticField),
		"/sensors/magnetic"));

	RCCHECK(rclc_publisher_init_default(
		&pub_fluid_pressure, &node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, FluidPressure),
		"/sensors/fluid_pressure"));

	RCCHECK(rclc_publisher_init_default(
		&pub_temp, &node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
		"/sensors/temperature"));

	// Create thruster subscriber

	RCCHECK(rclc_subscription_init_default(
		&sub_wrench, &node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Wrench),
		"/wrench_cmd"));

	// initialize imu timer
	RCCHECK(rclc_timer_init_default(
		&timer_imu, &support, RCL_MS_TO_NS(50), imuCallback));

	RCCHECK(rclc_timer_init_default(
		&timer_temp, &support, RCL_MS_TO_NS(2000), temperatureCallback));

	RCCHECK(rclc_timer_init_default(
		&timer_fluid_pressure, &support, RCL_MS_TO_NS(500), fluidPressureCallback));

	RCCHECK(rclc_executor_init(&executor, &support.context, 4, &rcl_allocator));

	rclc_executor_add_subscription(
		&executor,
		&sub_wrench,
		&wrench_msg,
		&thrustersCallback,
		ON_NEW_DATA
	);

	RCCHECK(rclc_executor_add_timer(&executor, &timer_imu));
	RCCHECK(rclc_executor_add_timer(&executor, &timer_temp));
	RCCHECK(rclc_executor_add_timer(&executor, &timer_fluid_pressure));

	blinkLed(2);

	// Create tasks
	xTaskCreatePinnedToCore(sensorTask, "Sensor reader task", 2048, NULL, 1, NULL, 1);
	xTaskCreatePinnedToCore(rosExecutorTask, "Ros executor Task", 2048, NULL, 1, NULL, 0);


	uartDebug("ℹ️ MCU setup finish");
}

unsigned long lastPrintTime = 0;
unsigned long lastPrintTime2 = 0;
const unsigned long printInterval = 1000; // 1000 мс = 1 секунда



void rosExecutorTask(void *param)
{
	uartDebug("ℹ️ ROS task started");

	while (true)
	{
		vTaskDelay(pdMS_TO_TICKS(10));
		RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
	}
}

void loop()
{

}

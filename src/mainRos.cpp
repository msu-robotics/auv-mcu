#include <Arduino.h>
#include "sensors/imu/modular.h"
#include "utils.h"
#include "sensors/imuFactory.h"
#include "sensors/depthFactory.h"
#include <micro_ros_platformio.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <Wire.h>
#include <rosidl_runtime_c/string_functions.h>
#include <sensor_msgs/msg/imu.h>
#include <sensor_msgs/msg/temperature.h>
#include <sensor_msgs/msg/magnetic_field.h>
#include <sensor_msgs/msg/fluid_pressure.h>
#include <geometry_msgs/msg/wrench_stamped.h>
#include <std_msgs/msg/int8_multi_array.h>
#include <std_msgs/msg/int32.h>
#include "thrusters/thrustersFactory.h"
#include "thrusters/thruster.h"
#include <vector>
#include <stdlib.h>
#include <esp_system.h>



#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){errorHandler();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}

void rosExecutorTask(void *param);

rcl_allocator_t rcl_allocator;
rclc_support_t support;
rcl_node_t node;

rcl_publisher_t pub_imu;
rcl_publisher_t pub_magnetic;
rcl_publisher_t pub_depth;
rcl_publisher_t pub_temp;

rcl_subscription_t sub_wrench;

rcl_timer_t imu_timer;
rcl_timer_t magnetic_timer;
rcl_timer_t fluid_pressure_timer;
rcl_timer_t timer_temp;

rclc_executor_t executor;

sensor_msgs__msg__Imu imu_msg;
sensor_msgs__msg__MagneticField magnetic_msg;
sensor_msgs__msg__FluidPressure fluid_pressure_msg;
sensor_msgs__msg__Temperature temperature;
std_msgs__msg__Int32 temp_msg;
#if defined(PROD)
geometry_msgs__msg__WrenchStamped wrench_msg;
#else
std_msgs__msg__Int8MultiArray thrusters_power_msg;

#endif
ModularIMU *imu;
ModularDepthSensor *depth;
Blinker blinker = Blinker();
ThrusterAllocator *tAllocator;
Thruster *thr1;
const char* frame = "base_link";


void errorHandler()
{
	uartDebug("❌Something went wrong, restart MCU");
	ESP.restart();
}


void imuCallback(rcl_timer_t *timer, int64_t last_call_time)
{
	RCLC_UNUSED(last_call_time);

	if (imu->ready) {
		int64_t time_ms = rmw_uros_epoch_millis();
		imu_msg.header.stamp.sec = time_ms / 1000;
		imu_msg.header.stamp.nanosec = (time_ms % 1000) * 1000000;

		rosidl_runtime_c__String__assign(&imu_msg.header.frame_id, frame);

		auto d = imu->read();

		float fAcc[3], fGyro[3], fAngle[3], fMagnet[3], fQ[4];

		float tmpAcc[3] = {d.accelX, d.accelY, d.accelZ};
		fillArray(tmpAcc, fAcc, 3, 16.0f / 32768.0f);
		float tmpGyro[3] = {d.velX, d.velY, d.velZ};
		fillArray(tmpGyro, fGyro, 3, 2000.0f / 32768.0f);
		float tmpAngle[3] = {d.angleX, d.angleY, d.angleZ};
		fillArray(tmpAngle, fAngle, 3, 180.0f / 32768.0f);
		float tmpMag[3] = {d.magX, d.magY, d.magZ};
		fillArray(tmpMag, fMagnet, 3);
		float tmpQ[4] = {d.qW, d.qX, d.qY, d.qZ};
		fillArray(tmpQ, fQ, 4);

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
	#if defined(PROD)

	const geometry_msgs__msg__WrenchStamped *msg = (const geometry_msgs__msg__WrenchStamped *)msgin;

	char buffer[70];
    sprintf(buffer,
		"ℹ️  thruster force x:%.2f y:%.2f z:%.2f",
		msg->wrench.force.x,
		msg->wrench.force.y,
		msg->wrench.force.z);
    uartDebug(buffer);
	std::vector<float> wrench = {
        static_cast<float>(msg->wrench.force.x),
		static_cast<float>(msg->wrench.force.y),
		static_cast<float>(msg->wrench.force.z),
		static_cast<float>(msg->wrench.torque.x),
		static_cast<float>(msg->wrench.torque.y),
		static_cast<float>(msg->wrench.torque.z)
    };

    tAllocator->allocate(wrench);

	#else

	const std_msgs__msg__Int8MultiArray *msg = (const std_msgs__msg__Int8MultiArray *)msgin;

	for (size_t i = 0; i < msg->data.size && i < 5; ++i) {
		tAllocator->setThrusterPowerManual(i, (float)msg->data.data[i]);
	}

	#endif
}



void temperatureCallback(rcl_timer_t *, int64_t)
{
	auto data = depth->read();
	temp_msg.data = data.temperature;
	RCSOFTCHECK(rcl_publish(&pub_temp, &temp_msg, NULL));
}


void fluidPressureCallback(rcl_timer_t *, int64_t)
{
	auto data = depth->read();
	fluid_pressure_msg.fluid_pressure = data.depth;
	RCSOFTCHECK(rcl_publish(&pub_depth, &fluid_pressure_msg, NULL));
}

void setup()
{
	Serial.begin(115200);
	Serial2.begin(115200, SERIAL_8N1);

	esp_reset_reason_t reason = esp_reset_reason();
  	Serial2.printf("ℹ️ Reset reason: %d\n", reason);

	// Предвыдели память под data (например, 10 элементов)
	thrusters_power_msg.data.data = (int8_t *) malloc(sizeof(int8_t) * 5);
	thrusters_power_msg.data.capacity = 5;
	thrusters_power_msg.data.size = 0;  // будет установлено в callback

	thrusters_power_msg.layout.dim.data = NULL;
	thrusters_power_msg.layout.dim.capacity = 0;
	thrusters_power_msg.layout.dim.size = 0;

	uartDebug("ℹ️ MCU setup started");

	imu = createIMU();
	depth = createDepthSensor();
	tAllocator = createThrusterAllocator();
	uartDebug("ℹ️ setup ros serial");
	set_microros_serial_transports(Serial);

	delay(1000);
	uartDebug("ℹ️ setup ros allocator");
	rcl_allocator = rcl_get_default_allocator();
	Serial2.printf("ℹ️ check ros allocator, free memory: %d\n", esp_get_free_heap_size());
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
		&pub_depth, &node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, FluidPressure),
		"/sensors/fluid_pressure"));

	RCCHECK(rclc_publisher_init_default(
		&pub_temp, &node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
		"/sensors/temperature"));

	// Create thruster subscriber

	#if defined(PROD)
	uartDebug("ℹ️ Build prod mode");
	RCCHECK(rclc_subscription_init_default(
		&sub_wrench, &node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, WrenchStamped),
		"/wrench_cmd"));
	#else
	uartDebug("ℹ️ Build dev mode");
	RCCHECK(rclc_subscription_init_default(
		&sub_wrench, &node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int8MultiArray),
		"/manual_thrusters"));

	#endif

	// initialize imu timer
	RCCHECK(rclc_timer_init_default(
		&imu_timer, &support, RCL_MS_TO_NS(50), imuCallback));

	RCCHECK(rclc_timer_init_default(
		&timer_temp, &support, RCL_MS_TO_NS(2000), temperatureCallback));

	RCCHECK(rclc_timer_init_default(
		&fluid_pressure_timer, &support, RCL_MS_TO_NS(500), fluidPressureCallback));

	RCCHECK(rclc_executor_init(&executor, &support.context, 4, &rcl_allocator));

	#if defined(PROD)
	rclc_executor_add_subscription(
		&executor,
		&sub_wrench,
		&wrench_msg,
		&thrustersCallback,
		ON_NEW_DATA
	);
	#else
	rclc_executor_add_subscription(
		&executor,
		&sub_wrench,
		&thrusters_power_msg,
		&thrustersCallback,
		ON_NEW_DATA
	);
	#endif

	RCCHECK(rclc_executor_add_timer(&executor, &imu_timer));
	RCCHECK(rclc_executor_add_timer(&executor, &timer_temp));
	RCCHECK(rclc_executor_add_timer(&executor, &fluid_pressure_timer));

	// // Create tasks
	// xTaskCreatePinnedToCore(sensorTask, "Sensor reader task", 2048, NULL, 1, NULL, 1);
	xTaskCreatePinnedToCore(rosExecutorTask, "Ros executor Task", 8192, NULL, 1, NULL, 0);


	uartDebug("ℹ️ MCU setup finish");
}




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

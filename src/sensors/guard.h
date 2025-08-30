#pragma once

#if defined(IMU_GY87)
  #define IMU_SELECTED_1 1
#else
  #define IMU_SELECTED_1 0
#endif

#if defined(IMU_HWT905)
  #define IMU_SELECTED_2 1
#else
  #define IMU_SELECTED_2 0
#endif

#define IMU_TOTAL_SELECTED \
  (IMU_SELECTED_1 + IMU_SELECTED_2)

#if (IMU_SELECTED_1 + IMU_SELECTED_2) > 1
  #error "❌ Multiple IMUs selected. Please define only one -DIMU_... flag"
#endif

#if defined(MAG_QMC5883L)
  #define MAG_SELECTED_1 1
#else
  #define MAG_SELECTED_1 0
#endif

#if defined(MAG_HWT905)
  #define MAG_SELECTED_2 1
#else
  #define MAG_SELECTED_2 0
#endif

#define MAG_TOTAL_SELECTED \
  (MAG_SELECTED_1 + MAG_SELECTED_2)

#if (IMU_SELECTED_1 + IMU_SELECTED_2) > 1
  #error "❌ Multiple MAGnetometers selected. Please define only one -DMAG_... flag"
#endif


#if defined(DEPTH_BMP180)
  #define DEPTH_SELECTED_1 1
#else
  #define DEPTH_SELECTED_1 0
#endif

#if defined(DEPTH_MS5837)
  #define DEPTH_SELECTED_2 1
#else
  #define DEPTH_SELECTED_2 0
#endif

#if (DEPTH_SELECTED_2 + DEPTH_SELECTED_1) > 1
  #error "❌ Multiple DEPTH selected. Please define only one -DDEPTH_... flag"
#endif
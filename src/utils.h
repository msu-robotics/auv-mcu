#ifndef UTILS
#define UTILS

void uartDebug(const char *debug);
void fillArray(float *in, float *out, int count, float scale = 1.0f);
void fillCovariance(double* cov, double value);

template<typename Vec3>
void fillVec3(Vec3& vec, const float* data) {
	vec.x = data[0];
	vec.y = data[1];
	vec.z = data[2];
}

template<typename Quaternion>
void fillQuaternion(Quaternion& q, const float* data) {
	q.w = data[0];
	q.x = data[1];
	q.y = data[2];
	q.z = data[3];
}

#endif
#include "thrusterAllocator.h"
#include "utils.h"

ThrusterAllocator::ThrusterAllocator(int num_dof)
    : _num_dof(num_dof)
{}


void ThrusterAllocator::setThrusters(const std::vector<Thruster*>& thrusters) {
    _thrusters = thrusters;
    size_t n = thrusters.size();
    _correction_factors.assign(n, 1.0f);
    _reverse_flags.assign(n, false);
    _allocation_matrix.assign(n, std::vector<float>(_num_dof, 0.0f));
}

void ThrusterAllocator::setAllocationMatrix(const std::vector<std::vector<float>>& matrix) {
    if (matrix.size() != _thrusters.size() || matrix[0].size() != _num_dof) {
        uartDebug("❌[Allocator] Matrix size mismatch");
        return;
    }
    _allocation_matrix = matrix;
}

void ThrusterAllocator::setCorrectionFactors(const std::vector<float>& factors) {
    if (factors.size() != _thrusters.size()) return;
    _correction_factors = factors;
}

void ThrusterAllocator::setReverseFlags(const std::vector<bool>& reverse) {
    if (reverse.size() != _thrusters.size()) return;
    _reverse_flags = reverse;
}

void ThrusterAllocator::allocate(const std::vector<float>& wrench) {
    if (wrench.size() != _num_dof) {
        uartDebug("❌[Allocator] Incorrect wrench size");
        return;
    }

    for (size_t i = 0; i < _thrusters.size(); ++i) {
        float output = 0.0f;
        for (int j = 0; j < _num_dof; ++j) {
            output += _allocation_matrix[i][j] * wrench[j];
        }

        output *= _correction_factors[i];
        if (_reverse_flags[i]) output *= -1.0f;

        output = saturate(output);
        _thrusters[i]->setPower(output);
        // char buffer[70];
        // sprintf(buffer, "ℹ️  [Allocator] set truster:%d force:%.2f", i, output);
        // uartDebug(buffer);
    }
}

void ThrusterAllocator::setThrusterPowerManual(uint8_t thruster, float power) {
    char buffer[70];
    sprintf(buffer, "ℹ️ [Allocator] set truster:%d power:%.2f", thruster, power);
    uartDebug(buffer);
    power *= _correction_factors[thruster];
    if (_reverse_flags[thruster]) power *= -1.0f;
    power = saturate(power);
    _thrusters[thruster]->setPower(power);

}
float ThrusterAllocator::saturate(float value, float min_val, float max_val) const {
    if (value > max_val) return max_val;
    if (value < min_val) return min_val;
    return value;
}

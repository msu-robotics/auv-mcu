#pragma once
#include <vector>
#include "Thruster.h"

class ThrusterAllocator {
public:
    ThrusterAllocator(int num_dof);

    void setThrusters(const std::vector<Thruster*>& thrusters);
    void setAllocationMatrix(const std::vector<std::vector<float>>& matrix);
    void setCorrectionFactors(const std::vector<float>& factors);
    void setReverseFlags(const std::vector<bool>& reverse);

    void allocate(const std::vector<float>& wrench); // Fx, Fy, Fz, Mz...

private:
    int _num_dof;
    std::vector<Thruster*> _thrusters;
    std::vector<std::vector<float>> _allocation_matrix;
    std::vector<float> _correction_factors;
    std::vector<bool> _reverse_flags;

    float saturate(float value, float min_val = -1.0f, float max_val = 1.0f) const;
};

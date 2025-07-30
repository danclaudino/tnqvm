void* initialize_mps(int);
void print_mps(void* ptr_mps);
void apply_single_qubit_gate(int, const char*);
void applySingleUGate(int, const char*, double, double, double);
void two_qubit_gate(int, int, const char*);
double compute_expectation_z(int*, int);
void finalize_mps();
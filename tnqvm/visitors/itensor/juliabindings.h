void* initialize(const int);
void print_mps(const void*);
void applySingleQubitGate(const int, const char*);
void applySingleQubitRotGate(const int, const char*, const double);
void applySingleQubitUGate(const int, const char*, const double, const double, const double);
void applyTwoQubitGate(const int, const int, const char*);
void applyTwoQubitRotGate(const int, const int, const char*, const double);
double computeExpectationValueZ(const int*, const int);
void finalize();
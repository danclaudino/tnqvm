#include "ITensorMPSVisitor.hpp"
#include <iostream>
#include <cassert>
#include <algorithm>
#include "xacc.hpp"
#include "xacc_service.hpp"
#include "AcceleratorBuffer.hpp"
#include "CompositeInstruction.hpp"
#include "base/Gates.hpp"

namespace tnqvm {

ITensorMPSVisitor::ITensorMPSVisitor() {}

ITensorMPSVisitor::~ITensorMPSVisitor() {}

void ITensorMPSVisitor::initialize(std::shared_ptr<AcceleratorBuffer> buffer, int nbShots) {

  if (!isJuliaInitialized) {
    juliabindings::init_julia(0, nullptr);
    if (!jl_is_initialized()) {
      throw std::runtime_error("Failed to initialize Julia runtime.");
    }
    isJuliaInitialized = true;
  }

  m_buffer = buffer;
  m_measureBits.clear();
  juliabindings::initialize_mps(buffer->size());
}

void ITensorMPSVisitor::finalize() {

  if (!m_measureBits.empty()) {
    m_buffer->addExtraInfo("exp-val-z", juliabindings::compute_expectation_z(m_measureBits.data(), m_measureBits.size()));
  }

  if (isJuliaInitialized) {
    juliabindings::finalize_mps();
    isJuliaInitialized = false;
  }

}

void ITensorMPSVisitor::applySingleQubitGate(xacc::Instruction &in_gate) {

  assert(in_gate.bits().size() == 1);
  int bit_loc = in_gate.bits()[0] + 1;
  if (in_gate.name() == "Rx" || in_gate.name() == "Ry" || in_gate.name() == "Rz") {
    double angle = in_gate.getParameter(0).as<double>();
    //juliabindings::applySingleQubitRotationGate(bit_loc, in_gate.name().c_str(), angle);
  } else {
    juliabindings::apply_single_qubit_gate(bit_loc, in_gate.name().c_str());
  }

}

void ITensorMPSVisitor::visit(Hadamard &gate) {applySingleQubitGate(gate);}
void ITensorMPSVisitor::visit(X &gate) {applySingleQubitGate(gate);}
void ITensorMPSVisitor::visit(Y &gate) {applySingleQubitGate(gate);}
void ITensorMPSVisitor::visit(Z &gate) {applySingleQubitGate(gate);}
void ITensorMPSVisitor::visit(Rx &gate) {applySingleQubitGate(gate); }
void ITensorMPSVisitor::visit(Ry &gate) {applySingleQubitGate(gate); }
void ITensorMPSVisitor::visit(Rz &gate) {applySingleQubitGate(gate); }
void ITensorMPSVisitor::visit(U &gate) {applySingleQubitGate(gate); }
void ITensorMPSVisitor::visit(Measure &gate) {
  // julia containers start at 1
  m_measureBits.push_back(gate.bits()[0] + 1);
}


const double ITensorMPSVisitor::getExpectationValueZ(std::shared_ptr<CompositeInstruction> function) {
  return 0.0;
}

} // namespace tnqvm

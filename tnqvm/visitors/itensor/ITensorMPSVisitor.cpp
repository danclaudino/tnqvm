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
  juliabindings::initialize(buffer->size());
}

void ITensorMPSVisitor::finalize() {

  if (!m_measureBits.empty()) {
    m_buffer->addExtraInfo("exp-val-z", juliabindings::computeExpectationValueZ(m_measureBits.data(), m_measureBits.size()));
  }

  if (isJuliaInitialized) {
    juliabindings::finalize();
    isJuliaInitialized = false;
  }

}

void ITensorMPSVisitor::applySingleQubitGate(xacc::Instruction &in_gate) {

  assert(in_gate.nRequiredBits() == 1);
  auto bit_loc = in_gate.bits()[0] + 1;
  auto gateName = in_gate.name();
  if (gateName == "Rx" || gateName == "Ry" || gateName == "Rz") {
    double angle = in_gate.getParameter(0).as<double>();
    juliabindings::applySingleQubitRotGate(bit_loc, gateName.c_str(), angle);
  } else {
    juliabindings::applySingleQubitGate(bit_loc, gateName.c_str());
  }

}

void ITensorMPSVisitor::applyTwoQubitGate(xacc::Instruction &in_gate) {

  assert(in_gate.bits().size() == 1);
  auto targetBit = in_gate.bits()[0] + 1;
  auto controlBit = in_gate.bits()[1] + 1;
  auto gateName = in_gate.name();
  if (gateName == "CPhase" || gateName == "CRZ") {
    double angle = in_gate.getParameter(0).as<double>();
    juliabindings::applyTwoQubitRotGate(targetBit, controlBit, gateName.c_str(), angle);
  } else {
    juliabindings::applyTwoQubitGate(targetBit, controlBit, gateName.c_str());
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
void ITensorMPSVisitor::visit(CNOT &gate) {applyTwoQubitGate(gate); }
void ITensorMPSVisitor::visit(Swap &gate) {applyTwoQubitGate(gate); }
void ITensorMPSVisitor::visit(CPhase &gate) {applyTwoQubitGate(gate); }
void ITensorMPSVisitor::visit(CRZ &gate) {applyTwoQubitGate(gate); }
void ITensorMPSVisitor::visit(Measure &gate) {
  // julia containers start at 1
  m_measureBits.push_back(gate.bits()[0] + 1);
}


const double ITensorMPSVisitor::getExpectationValueZ(std::shared_ptr<CompositeInstruction> function) {
  return 0.0;
}

} // namespace tnqvm

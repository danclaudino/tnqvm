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

void ITensorMPSVisitor::initialize(std::shared_ptr<AcceleratorBuffer> buffer, int nbShots) {

  if (!isJuliaInitialized) {
    juliabindings::init_julia(0, nullptr);
    if (!jl_is_initialized()) {
      throw std::runtime_error("Failed to initialize Julia runtime.");
    }
    isJuliaInitialized = true;
    entropyCalc = -1000.0;
  }

  m_buffer = buffer;
  m_measureBits.clear();
  juliabindings::initialize(buffer->size());


    // Parse SVD options:
  // Default:
  // Copied from original:
  // https://code.ornl.gov/6d3/tnqvm/-/blob/master/tnqvm/visitors/itensor/mps/ITensorMPSVisitor.cpp?ref_type=heads
  // Initially set to default values
  m_svdCutoff = 1e-6;
  m_maxDim = 1024;
  m_haarSamples = 50;
  if (xacc::optionExists("itensor-svd-cutoff")) {
    std::string cutoffStr = "";
    try {
      cutoffStr = xacc::getOption("itensor-svd-cutoff");
      m_svdCutoff = std::stod(cutoffStr);
      if (verbose)
        xacc::info("ITensorMPSVisitor setting SVD cutoff to " + cutoffStr);
    } catch (std::exception &e) {
      xacc::error("ITensorMPSVisitor: invalid svd cutoff value " + cutoffStr);
    }
  }
  // Also support setting 'svd-cutoff' from the initialize HetMap.
  if (options.keyExists<double>("svd-cutoff")) {
    m_svdCutoff = options.get<double>("svd-cutoff");
    xacc::info("ITensorMPSVisitor setting SVD cut-off to " +
               std::to_string(m_svdCutoff));
  }
  if (options.keyExists<int>("svd-max-dim")) {
    m_maxDim = options.get<int>("svd-max-dim");
    xacc::info("ITensorMPSVisitor setting SVD max bond dimension to " +
               std::to_string(m_maxDim));
  }

  if (options.keyExists<int>("haar-samples")) {
    m_haarSamples = options.get<int>("haar-samples");
    xacc::info("ITensorMPSVisitor setting number of Haar samples to " +
               std::to_string(m_haarSamples));
  }

  m_logSvd = false;
  if (options.keyExists<bool>("log-svd")) {
    m_logSvd = options.get<bool>("log-svd");
  }



// Check whether either setting differs from the defaults
if (m_maxDim != DEFAULT_MAX_BOND_DIM ||
    m_svdCutoff != DEFAULT_SVD_CUTOFF)
{
    // Call into the Julia library
    juliabindings::overwriteSVDdefaults(m_maxDim, m_svdCutoff);
}

}

void ITensorMPSVisitor::finalize() {
  entropyCalc = -1000.0;
  if (!m_measureBits.empty()) {
    auto expValueZ = juliabindings::computeExpectationValueZ(m_measureBits.data(), m_measureBits.size());
    m_buffer->addExtraInfo("exp-val-z", expValueZ);
  }

  if (isJuliaInitialized) {
    juliabindings::finalize();
    juliabindings::shutdown_julia(0);
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



const double ITensorMPSVisitor::getExpectationValueZ(std::shared_ptr<CompositeInstruction> in_function) {

if (entropyCalc < 0.0) {
  // Compute entanglement mean and stardard error of the entropy
  // using Haar-random samples 
  juliabindings::entropyInfo e = juliabindings::sampleEntanglementEntropy(m_haarSamples);
  m_buffer->addExtraInfo("avg-entropy", e.avg);
  m_buffer->addExtraInfo("stddev-entropy", e.stddev);

  // Also, compute the half-chain entropy
  juliabindings::halfMPSentropyInfo halfSiteEntropy = juliabindings::computeMidpointMPSEntropy();
  m_buffer->addExtraInfo("half-chain-entropy", halfSiteEntropy.entropy);
} 

entropyCalc = 1000.0; 



  InstructionIterator it(in_function);
  assert(m_measureBits.empty());
  std::vector<int> measureBits;
  while (it.hasNext()) {
    auto nextInst = it.next();
    if (nextInst->isEnabled() && !nextInst->isComposite()) {
      if (nextInst->name() != "Measure") {
        nextInst->accept(this);
      } else {
        measureBits.emplace_back(nextInst->bits()[0] + 1);
      }
    }
  }
  assert(!measureBits.empty());
  const double exp_val = juliabindings::computeExpectationValueZ(measureBits.data(), measureBits.size());;
  return exp_val;
}

} // namespace tnqvm

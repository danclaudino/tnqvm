#pragma once
#include <cstdlib>
#include <memory>
#include <string>
#include "TNQVMVisitor.hpp"
#include "Cloneable.hpp"
#include "julia.h"

namespace juliabindings {
extern "C" {
  #include "juliaitensortnqvm.h"
  #include "julia_init.h"
}
}

namespace tnqvm {

class ITensorMPSVisitor : public TNQVMVisitor {

public:
  ITensorMPSVisitor() = default;;
  virtual std::shared_ptr<TNQVMVisitor> clone() {
    return std::make_shared<ITensorMPSVisitor>();
  }

  virtual const double getExpectationValueZ(std::shared_ptr<CompositeInstruction> function) override;


  virtual void initialize(std::shared_ptr<AcceleratorBuffer> buffer, int nbShots = 1) override;
  virtual void finalize() override;

  virtual const std::string name() const { return "itensor-mps"; }
  virtual const std::string description() const { return ""; }

  virtual OptionPairs getOptions() {
    OptionPairs desc{{"julia-itensor-svd-cutoff",
                      "Provide the cutoff (default 1e-4) for the singular value decomposition."}};
    return desc;
  }

  bool supportVqeMode() const override { return true; }

  void visit(Identity &gate) {}
  void visit(Hadamard &gate);
  void visit(X &gate);
  void visit(Y &gate);
  void visit(Z &gate);
  void visit(Rx &gate);
  void visit(Ry &gate);
  void visit(Rz &gate);
  void visit(U &gate);
  void visit(CNOT &gate);
  void visit(Swap &gate);
  void visit(CPhase &gate);
  void visit(CRZ &gate);
  void visit(Measure &gate);

private:
  void applySingleQubitGate(xacc::Instruction &in_gate);
  void applyTwoQubitGate(xacc::Instruction &in_gate);

private:
  std::shared_ptr<AcceleratorBuffer> m_buffer;
  std::vector<int> m_measureBits;
  // Default values (mirror Julia)
  const int DEFAULT_MAX_BOND_DIM = 5012;
  const double DEFAULT_SVD_CUTOFF = 1e-12;
  const int DEFAULT_HAAR_SAMPLES = 50;

  // SVD options from parsed input
  double m_svdCutoff;
  int m_maxDim;
  int m_haarSamples;
  std::shared_ptr<CompositeInstruction> m_conjCircuit;
  bool m_logSvd = false;


public:
  static inline bool isJuliaInitialized = false;
  double entropyCalc = -1000.0; // if negative value, we can calculate the entropy; otherwise, don't
};

} // namespace tnqvm


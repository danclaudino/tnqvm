/***********************************************************************************
 * Copyright (c) 2017, UT-Battelle
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the xacc nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Contributors:
 *   Initial sketch - Mengsu Chen 2017/07/17;
 *   Implementation - Dmitry Lyakh 2017/10/05;
 *
 **********************************************************************************/
#ifndef QUANTUM_GATE_ACCELERATORS_TNQVM_EXATENSORMPSVISITOR_HPP_
#define QUANTUM_GATE_ACCELERATORS_TNQVM_EXATENSORMPSVISITOR_HPP_

#ifdef TNQVM_HAS_EXATENSOR

#include <cstdlib>
#include <complex>
#include <vector>
#include "AllGateVisitor.hpp"
#include "TNQVMBuffer.hpp"
#include "GateFactory.hpp"

#include "exatensor.hpp"

namespace xacc {
namespace quantum {

class ExaTensorMPSVisitor : public AllGateVisitor {

private:

//Type aliases:
 using TensDataType = std::complex<double>;
 using Tensor = exatensor::TensorDenseAdpt<TensDataType>;
 using TensorNetwork = exatensor::TensorNetwork<TensDataType>;
 using WaveFunction = std::vector<Tensor>;

//Gate factory member:
 GateFactory GateTensors;

//Data members:
 std::shared_ptr<TNQVMBuffer> Buffer; //accelerator buffer
 WaveFunction StateMPS;               //MPS wave-function of qubits
 TensorNetwork TensNet;               //currently constructed tensor network
 bool EagerEval;                      //if TRUE each gate will be applied immediately (defaults to FALSE)

//Private member functions:
 void initMPSTensor(const unsigned int tensNum); //initializes an MPS tensor to a pure |0> state
 int apply1BodyGate(const Tensor & gate, const unsigned int q0); //applies a 1-body gate to a qubit
 int apply2BodyGate(const Tensor & gate, const unsigned int q0, const unsigned int q1); //applies a 2-body gate to a pair of qubits
 int applyNBodyGate(const Tensor & gate, const unsigned int q[]); //applies an arbitrary N-body gate to N qubits

public:

//Constants:
 static const unsigned int BASE_SPACE_DIM = 2; //basic space dimension (2 for a qubit)
 static const std::size_t INITIAL_VALENCE = 2; //initial dimension extent for virtual MPS indices

//Life cycle:
 ExaTensorMPSVisitor(std::shared_ptr<TNQVMBuffer> buffer,
                     const std::size_t initialValence = INITIAL_VALENCE); //ctor
 virtual ~ExaTensorMPSVisitor(); //dtor

//Visitor methods:
 void visit(Hadamard & gate);
 void visit(X & gate);
 void visit(Y & gate);
 void visit(Z & gate);
 void visit(Rx & gate);
 void visit(Ry & gate);
 void visit(Rz & gate);
 void visit(CPhase & gate);
 void visit(CNOT & gate);
 void visit(Swap & gate);
 void visit(Measure & gate);
 void visit(ConditionalFunction & condFunc);
 void visit(GateFunction & gateFunc);

//Numerical evaluation:
 void setEvaluationStrategy(const bool eagerEval); //sets EagerEval member
 int evaluate(); //evaluates the constructed tensor network

}; //end class ExaTensorMPSVisitor

} //end namespace quantum
} //end namespace xacc

#endif //TNQVM_HAS_EXATENSOR

#endif //QUANTUM_GATE_ACCELERATORS_TNQVM_EXATENSORMPSVISITOR_HPP_

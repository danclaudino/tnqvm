#include "ExatnVisitor.hpp"
#include "exatn.hpp"

#include "OptionsProvider.hpp"

#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/ServiceProperties.h"

using namespace cppmicroservices;

class US_ABI_LOCAL ExaTNActivator : public BundleActivator {
public:
  ExaTNActivator() {}

  void Start(BundleContext context) {
    // Initialize ExaTN
    exatn::initialize();
    auto visitor = std::make_shared<tnqvm::ExatnVisitor>();
    context.RegisterService<tnqvm::TNQVMVisitor>(visitor);
    context.RegisterService<xacc::OptionsProvider>(visitor);
  }

  void Stop(BundleContext context) {}
};

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(ExaTNActivator)

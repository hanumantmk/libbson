#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/AST.h"
#include "clang/Frontend/CompilerInstance.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>
using namespace clang;

namespace {

class PrintFunctionsVisitor : public RecursiveASTVisitor<PrintFunctionsVisitor> {
public:
   bool VisitCallExpr(CallExpr * expr) {
      if (expr->getDirectCallee()->getNameInfo().getName().getAsString() == "bcon_append") {
      std::cerr << "--------------------\n";
      //expr->dump();
      //std::cerr << "\n\n";
      //expr->getDirectCallee()->getNameInfo().getName().dump();
      //std::cerr << "\n\n";
      for (auto a = expr->arg_begin(); a != expr->arg_end(); ++a) {
         a->dump();
      }
      std::cerr << "--------------------\n\n\n\n\n\n\n\n";
      }

      return true;
   }
};

class PrintFunctionsConsumer : public ASTConsumer {
   PrintFunctionsVisitor visitor;
public:
   virtual void HandleTranslationUnit(ASTContext & Context) {
      visitor.TraverseDecl(Context.getTranslationUnitDecl());
   }
};

class PrintFunctionNamesAction : public PluginASTAction {
protected:
  ASTConsumer *CreateASTConsumer(CompilerInstance &CI, llvm::StringRef) {
    return new PrintFunctionsConsumer();
  }

  bool ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string>& args) {
    for (unsigned i = 0, e = args.size(); i != e; ++i) {
      llvm::errs() << "PrintFunctionNames arg = " << args[i] << "\n";

      // Example error handling.
      if (args[i] == "-an-error") {
        DiagnosticsEngine &D = CI.getDiagnostics();
        unsigned DiagID = D.getCustomDiagID(
          DiagnosticsEngine::Error, "invalid argument '" + args[i] + "'");
        D.Report(DiagID);
        return false;
      }
    }
    if (args.size() && args[0] == "help")
      PrintHelp(llvm::errs());

    return true;
  }
  void PrintHelp(llvm::raw_ostream& ros) {
    ros << "Help for PrintFunctionNames plugin goes here\n";
  }

};

}

static FrontendPluginRegistry::Add<PrintFunctionNamesAction>
X("print-fns", "print function names");

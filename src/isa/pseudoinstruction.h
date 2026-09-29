#pragma once

#include <optional>

#include "instruction.h"
#include "isa_defines.h"
#include "symbolmap.h"

namespace Ripes {

struct PseudoInstructionBase {
  virtual ~PseudoInstructionBase() {}
  virtual Result<std::vector<LineTokens>> expand(const TokenizedSrcLine &line,
                                                 SymbolMap &symbols) const = 0;
  virtual QString name() const = 0;
};

using PseudoExpandFunc = std::function<Result<std::vector<LineTokens>>(
    const PseudoInstructionBase & /* this */, const TokenizedSrcLine &,
    const SymbolMap &)>;

template <typename PseudoInstrImpl>
struct PseudoInstruction : public PseudoInstructionBase {
  constexpr static unsigned expectedTokens() {
    return 1 + PseudoInstrImpl::Fields::numFields();
  }
  QString name() const override {
    return QString(PseudoInstrImpl::NAME.data());
  }
  Result<std::vector<LineTokens>> expand(const TokenizedSrcLine &line,
                                         SymbolMap &symbols) const override {
    if (line.tokens.length() != expectedTokens()) {
      const auto expected = expectedTokens() - 1;
      QString msg = "'" + name() + "' takes " + QString::number(expected) +
                    (expected == 1 ? " operand" : " operands");
      if (expected > 0)
        msg += " (" + PseudoInstrImpl::Fields::describe().join(", ") + ")";
      return Error(line, msg + ", but the line has " +
                             QString::number(line.tokens.length() - 1));
    }

    return PseudoInstrImpl::expander(*this, line, symbols);
  }
};

template <unsigned index, typename ISAImpl>
struct PseudoReg : public Reg<PseudoReg<index, ISAImpl>, index,
                              BitRange<index, index>, ISAImpl> {
  constexpr static std::string_view getName() { return "rd"; }
};

template <unsigned index>
struct PseudoImm : public Imm<index, 1, Repr::Hex, ImmPart<0, index, index>> {
  // Pseudo-instructions accept either: 'li' a number, 'la'/'j' a label.
  static void describe(QStringList &kinds) { kinds << "label or number"; }
};

using PseudoInstrVec = std::vector<std::shared_ptr<PseudoInstructionBase>>;

using PseudoInstrMap =
    std::map<QString, std::shared_ptr<PseudoInstructionBase>>;

template <typename... Instructions>
constexpr inline static void
enablePseudoInstructions(PseudoInstrVec &instructions) {
  // TODO(raccog): Ensure no duplicate pseudo-instruction definitions
  // TODO(raccog): Verify instructions generated from pseudoinstructions
  return _enableInstructions<PseudoInstrVec, Instructions...>(instructions);
}

} // namespace Ripes

#pragma once

#include "isa_defines.h"
#include <optional>

namespace Ripes {

using AbsoluteSymbolMap = std::map<Symbol, VIntS>;
struct SymbolMap {
  AbsoluteSymbolMap abs;
  // Names recorded by the parser, before address assignment in pass 2. This
  // lets pseudo-instructions distinguish forward labels from bad constants.
  std::set<QString> declaredAddresses;
  using RelativeSymbol = int;
  using SourceLine = unsigned;
  std::map<RelativeSymbol, std::map<SourceLine, VIntS>> rel;

  void clear() {
    abs.clear();
    rel.clear();
    declaredAddresses.clear();
  }

  bool isDeclaredAddress(const QString &name) const {
    if (declaredAddresses.count(name))
      return true;
    if (name.endsWith('f') || name.endsWith('b')) {
      const auto local = name.chopped(1);
      return Symbol(local).isLocal() && declaredAddresses.count(local);
    }
    return false;
  }

  std::optional<Error> addSymbol(const TokenizedSrcLine &line, const Symbol &s,
                                 VInt v) {
    return s.isLocal() ? addRelSymbol(line.sourceLine(), s, v)
                       : addAbsSymbol(line.sourceLine(), s, v);
  }

  /// Adds a symbol to the current symbol mapping of this assembler defined at
  /// the 'line' in the input program.
  std::optional<Error> addAbsSymbol(const TokenizedSrcLine &line,
                                    const Symbol &s, VInt v) {
    return addAbsSymbol(line.sourceLine(), s, v);
  }

  /// Adds a symbol to the current symbol mapping of this assembler.
  std::optional<Error> addAbsSymbol(const unsigned &line, const Symbol &s,
                                    VInt v);

  /// Adds a relative symbol to this symbol map. A relative symbol is unqiued
  /// based on the tuple <symbol ID, source line>.
  std::optional<Error> addRelSymbol(const TokenizedSrcLine &line,
                                    const Symbol &s, VInt v) {
    return addRelSymbol(line.sourceLine(), s, v);
  }

  std::optional<Error> addRelSymbol(const unsigned &line, const Symbol &s,
                                    VInt v);

  /// Returns a copy of this symbol map with relative symbols copied relative to
  /// 'line'. Relative symbols may be suffixed with additional strings in cases
  /// of being before/after the provided source line.
  AbsoluteSymbolMap copyRelativeTo(unsigned line,
                                   const QString &beforeSuffix = "b",
                                   const QString &afterSuffix = "f") const;
};

} // namespace Ripes

#pragma once

#include "assembler_defines.h"
#include "isa/symbolmap.h"
#include <QRegularExpression>
#include <variant>

namespace Ripes {
namespace Assembler {

extern const QRegularExpression s_exprOperatorsRegex;
extern const QString s_exprOperators;
extern const QString s_exprTokens;
using ExprEvalVT = int64_t; // Expression evaluation value type
using ExprEvalRes = Result<ExprEvalVT>;

/**
 * @brief evaluate
 * Very simple expression parser for evaluating a right-associative binary
 * (2-operand) mathematical expressions. For now, no operator precedence is
 * implemented - to ensure precedence, parentheses must be implemented. The
 * functionality is mainly intended to be used by the assembler to expand
 * complex pseudoinstructions and as such not by the user.
 * If resolvedSymbols is provided, fill it with the actual symbols read during
 * evaluation, preserving their address/constant types; clear it on each call.
 */
ExprEvalRes evaluate(const Location &, const QString &,
                     const AbsoluteSymbolMap *variables = nullptr,
                     AbsoluteSymbolMap *resolvedSymbols = nullptr);

/**
 * @brief couldBeExpression
 * @returns true if we have probably cause that the string is an expression and
 * not 'just' a single variable.
 */
bool couldBeExpression(const QString &s);
} // namespace Assembler
} // namespace Ripes

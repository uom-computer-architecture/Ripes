#include "parserutilities.h"
#include "binutils.h"

#include <memory>

namespace Ripes {
namespace Assembler {

/**
 * @brief joinParentheses takes a number of tokens and merges together tokens
 * contained within top-level parentheses. For example: [lw, x10, (B, +,
 * (3*2))(x10)] => [lw, x10, B + 3*2), x10]
 */
bool matchedParens(std::vector<QChar> &parensStack, QChar end) {
  if (parensStack.size() == 0) {
    return false;
  }
  const QChar toMatch = parensStack.at(parensStack.size() - 1);
  parensStack.pop_back();
  return (toMatch == '[' && end == ']') || (toMatch == '(' && end == ')');
}

Result<LineTokens> joinParentheses(const Location &loc,
                                   const LineTokens &tokens) {
  LineTokens outtokens;
  std::vector<QChar> parensStack;

  Token tokenBuffer;
  int nextGroup = 0;
  int group = -1;
  Token::Part part = Token::Part::Whole;
  auto commitBuffer = [&]() {
    if (!tokenBuffer.isEmpty()) {
      tokenBuffer.sourceGroup = group;
      tokenBuffer.sourcePart = part;
      outtokens << tokenBuffer;
      tokenBuffer.clear();
    }
  };

  for (const auto &token : tokens) {
    if (token.startsWith("\"") && token.endsWith("\"")) {
      // String literal; ignore parentheses inside
      Token quoted = token;
      quoted.sourceGroup = nextGroup++;
      outtokens << quoted;
      continue;
    }
    if (parensStack.empty()) {
      group = nextGroup++;
      part = Token::Part::Whole;
    }
    for (const auto &ch : token) {
      switch (ch.unicode()) {
      case '(':
      case '[':
        if (!parensStack.empty()) {
          tokenBuffer.append(ch);
        } else {
          const bool address = !tokenBuffer.isEmpty();
          if (address)
            part = Token::Part::Offset;
          commitBuffer();
          part = address ? Token::Part::Base : Token::Part::Parenthesized;
        }
        parensStack.push_back(ch);
        break;
      case ']':
      case ')': {
        if (matchedParens(parensStack, ch)) {
          if (parensStack.empty()) {
            commitBuffer();
          } else {
            tokenBuffer.append(ch);
          }
        } else {
          return {Error(loc, "Unmatched parenthesis")};
        }
        break;
      }
      default:
        tokenBuffer.append(ch);
        break;
      }
    }
    if (parensStack.empty()) {
      commitBuffer();
    }
  }

  if (parensStack.empty()) {
    return outtokens;
  } else {
    return {Error(loc, "Unmatched parenthesis")};
  }
}

Result<LineTokens> tokenizeQuotes(const Location &location,
                                  const QString &line) {
  LineTokens tokens;
  bool inQuotes = false;
  bool escape = false;
  QString substr;
  auto pushSubstr = [&] {
    Token token(substr);
    tokens.push_back(token);
    substr.clear();
  };
  for (qsizetype i = 0; i < line.size(); ++i) {
    const auto ch = line[i];
    if (inQuotes) {
      if (!escape) {
        if (inQuotes && ch == '"') {
          inQuotes = false;
          substr += ch;
          pushSubstr();
          continue;
        }
        if (ch == '\\')
          escape = true;
      } else
        escape = false;
      substr.push_back(ch);
    } else {
      // A semicolon inside a string or character literal is data. Everywhere
      // else it is an unsupported separator in this assembler, not a new
      // operand or an invalid register suffix.
      if (ch == '\'') {
        const auto length = line.mid(i + 1, 1) == "\\" ? 4 : 3;
        if (line.mid(i + length - 1, 1) == "'") {
          substr += line.mid(i, length);
          i += length - 1;
          continue;
        }
      }
      if (ch == ';')
        return Error(location, "Unexpected ';': this Ripes assembler does not "
                               "support semicolon statement separators; "
                               "use a new line between instructions");
      if (ch == ' ' || ch == ',' || ch == '\t') {
        if (!substr.isEmpty())
          pushSubstr();
      } else
        substr.push_back(ch);
      if (ch == '\"')
        inQuotes = true;
    }
  }

  if (inQuotes)
    return {Error(location, "Missing terminating '\"' character.")};

  if (!substr.isEmpty())
    pushSubstr();
  return {tokens};
}

} // namespace Assembler
} // namespace Ripes

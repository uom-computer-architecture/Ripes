#include <QtTest/QTest>

#include "assembler/expreval.h"

using namespace Ripes;
using namespace Ripes::Assembler;

class tst_ExprEval : public QObject {
  Q_OBJECT

private slots:
  void tst_binops();
  void tst_symbolProvenance();
};

void expect(const ExprEvalRes &res, const ExprEvalVT &expected) {
  if (auto *err = std::get_if<Error>(&res)) {
    QString errstr = "Got error: " + err->toString();
    QFAIL(errstr.toStdString().c_str());
  }
  QCOMPARE(std::get<ExprEvalVT>(res), expected);
}

void tst_ExprEval::tst_binops() {
  expect(evaluate(Location::unknown(), "(0x2*(3+4))+4"), 18);
  expect(evaluate(Location::unknown(), "2+3*7*5"), 107);
  SymbolMap symbols;
  Location::unknown(), symbols.abs["B"] = 2;
  expect(evaluate(Location::unknown(), "(B *(3+ 4))+4", &symbols.abs), 18);
}

void tst_ExprEval::tst_symbolProvenance() {
  AbsoluteSymbolMap symbols{{Symbol("input_n", Symbol::Address), 0x10000000},
                            {Symbol("offset", Symbol::Constant), 4},
                            {Symbol("unused", Symbol::Address), 0x20000000}};
  AbsoluteSymbolMap resolved;
  expect(
      evaluate(Location::unknown(), "input_n+(offset*2)", &symbols, &resolved),
      0x10000008);
  QCOMPARE(resolved.size(), size_t(2));
  QVERIFY(resolved.find("input_n")->first.is(Symbol::Address));
  QVERIFY(resolved.find("offset")->first.is(Symbol::Constant));
  QVERIFY(resolved.count("unused") == 0);

  // A new evaluation clears old provenance; numeric expressions do not
  // acquire address provenance just because their result looks like one.
  expect(evaluate(Location::unknown(), "0x10000000+4", &symbols, &resolved),
         0x10000004);
  QVERIFY(resolved.empty());
}

QTEST_APPLESS_MAIN(tst_ExprEval)
#include "tst_expreval.moc"

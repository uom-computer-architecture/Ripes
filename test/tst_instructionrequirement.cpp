#include "assembler/assembler.h"
#include "cli/instructionrequirement.h"
#include "isa/rv32isainfo.h"
#include <QtTest/QTest>

using namespace Ripes;

class tst_InstructionRequirement : public QObject {
  Q_OBJECT
private slots:
  void evidenceFailures() {
    Assembler::ISA_Assembler<ISA::RV32I> assembler(
        std::make_shared<ISAInfo<ISA::RV32I>>(QStringList{}));
    const std::map<std::pair<AInt, VInt>, uint64_t> evidence{
        {{0, 0x00151513}, 1}}; // slli a0,a0,1
    auto check = [&](bool supported, const Program::SourceMapping &mapping) {
      return evaluateInstructionRequirement({"sll", "slli"}, 3, supported,
                                            evidence, mapping, assembler);
    };
    for (const auto &[supported, mapping, reason] :
         std::vector<std::tuple<bool, Program::SourceMapping, QString>>{
             {false, {{0, {2}}}, "retirement_unsupported"},
             {true, {}, "source_unmapped"},
             {true, {{0, {}}}, "source_unmapped"},
             {true, {{0, {1, 2}}}, "source_ambiguous"}}) {
      const auto verdict = check(supported, mapping);
      QCOMPARE(verdict["status"].toString(), QString("error"));
      QCOMPARE(verdict["reason"].toString(), reason);
    }
    QCOMPARE(check(true, {{0, {1}}})["status"].toString(), QString("not_met"));
    const auto boundary = check(true, {{0, {2}}});
    QCOMPARE(boundary["status"].toString(), QString("met"));
    QCOMPARE(boundary["counts"].toMap()["slli"].toULongLong(), qulonglong{1});
    QCOMPARE(check(true, {{0, {2, 3}}})["status"].toString(), QString("met"));
    const auto unknown = evaluateInstructionRequirement(
        {"slli"}, 1, true, {{{0, 0xffffffff}, 1}}, {}, assembler);
    QCOMPARE(unknown["status"].toString(), QString("error"));
    QCOMPARE(unknown["reason"].toString(),
             QString("unknown_retired_instruction"));
  }
};

QTEST_GUILESS_MAIN(tst_InstructionRequirement)
#include "tst_instructionrequirement.moc"

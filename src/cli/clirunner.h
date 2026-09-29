#pragma once

#include "clioptions.h"
#include <QJsonObject>
#include <QObject>

namespace Ripes {

/// The CLIRunner class is used to run Ripes in CLI mode.
/// Based on a CLIModeOptions struct, it will run the appropriate combination
/// of source processing (assembler/compiler/...), processor model execution
/// as well as telemetry gathering and reporting.
class CLIRunner : public QObject {
  Q_OBJECT
public:
  CLIRunner(const CLIModeOptions &options);

  /// Runs the CLI mode.
  int run();

private:
  /// Process the provided source file (assembling, compiling, loading, ...)
  int processInput();

  /// Applies --datainit memory writes. Must run after processInput() (so
  /// symbols are resolvable) and before runModel().
  int applyDataInit();

  /// Runs the processor model until the program is finished.
  int runModel();

  /// Prints requested telemetry to the console/output file.
  int postRun();

  /// Reports a run that stopped before telemetry could be gathered. With
  /// --json the result is written where postRun() would have written, so a
  /// caller always gets a machine-readable outcome instead of having to parse
  /// the console text. Always returns 1.
  int fail(QJsonObject result);
  void info(QString msg, bool alwaysPrint = false, bool header = false,
            const QString &prefix = "INFO");
  void error(const QString &msg);

  CLIModeOptions m_options;
};

} // namespace Ripes

#pragma once

#include "opentelemetry/nostd/type_traits.h"
#include "opentelemetry/sdk/trace/exporter.h"
#include "opentelemetry/sdk/trace/span_data.h"
#include "opentelemetry/version.h"

#include <iostream>
#include <map>
#include <sstream>

namespace nostd    = opentelemetry::nostd;
namespace sdktrace = opentelemetry::sdk::trace;

OPENTELEMETRY_BEGIN_NAMESPACE
namespace exporter
{
namespace trace
{

/**
 * The OStreamSpanExporter exports span data through an ostream
 */
class OStreamSpanExporter final : public sdktrace::SpanExporter
{
public:
  /**
   * Create an OStreamSpanExporter. This constructor takes in a reference to an ostream that the
   * export() function will send span data into.
   * The default ostream is set to stdout
   */
  explicit OStreamSpanExporter(std::ostream &sout = std::cout) noexcept;

  std::unique_ptr<sdktrace::Recordable> MakeRecordable() noexcept override;

  sdktrace::ExportResult Export(
      const nostd::span<std::unique_ptr<sdktrace::Recordable>> &spans) noexcept override;

  void Shutdown(std::chrono::microseconds timeout = std::chrono::microseconds(0)) noexcept override;

private:
  std::ostream &sout_;
  bool isShutdown_ = false;

  // Mapping status number to the string from api/include/opentelemetry/trace/canonical_code.h
  std::map<int, std::string> statusMap{{0, "OK"},
                                       {1, "CANCELLED"},
                                       {2, "UNKNOWN"},
                                       {3, "INVALID_ARGUMENT"},
                                       {4, "DEADLINE_EXCEEDED"},
                                       {5, "NOT_FOUND"},
                                       {6, "ALREADY_EXISTS"},
                                       {7, "PERMISSION_DENIED"},
                                       {8, "RESOURCE_EXHAUSTED"},
                                       {9, "FAILED_PRECONDITION"},
                                       {10, "ABORTED"},
                                       {11, "OUT_OF_RANGE"},
                                       {12, "UNIMPLEMENTED"},
                                       {13, "INTERNAL"},
                                       {14, "UNAVAILABLE"},
                                       {15, "DATA_LOSS"},
                                       {16, "UNAUTHENTICATED"}};

  /*
    print_value is used to print out the value of an attribute within a vector.
    These values are held in a variant which makes the process of printing them much more
    complicated.
  */

  template <typename T>
  void print_value(const T &item)
  {
    sout_ << item;
  }

  template <typename T>
  void print_value(const std::vector<T> &vec)
  {
    sout_ << '[';
    size_t i  = 1;
    size_t sz = vec.size();
    for (auto v : vec)
    {
      sout_ << v;
      if (i != sz)
        sout_ << ',';
      i++;
    };
    sout_ << ']';
  }

  class SpanDataAttributeValueVisitor
  {
  public:
    SpanDataAttributeValueVisitor(OStreamSpanExporter &exporter) : exporter_(exporter) {}

    template <typename T>
    void operator()(T &&arg)
    {
      exporter_.print_value(arg);
    }

  private:
    OStreamSpanExporter &exporter_;
  };

  void print_value(sdktrace::SpanDataAttributeValue &value)
  {
    nostd::visit(SpanDataAttributeValueVisitor(*this), value);
  }

  void printAttributes(std::unordered_map<std::string, sdktrace::SpanDataAttributeValue> map)
  {
    size_t size = map.size();
    size_t i    = 1;
    for (auto kv : map)
    {
      sout_ << kv.first << ": ";
      print_value(kv.second);

      if (i != size)
        sout_ << ", ";
      i++;
    }
  }
};
}  // namespace trace
}  // namespace exporter
OPENTELEMETRY_END_NAMESPACE

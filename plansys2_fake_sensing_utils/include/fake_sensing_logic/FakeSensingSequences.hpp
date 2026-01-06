#ifndef FAKE_SENSING_SEQUENCES_HPP
#define FAKE_SENSING_SEQUENCES_HPP

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <random>
#include <ostream>

#include <rclcpp/rclcpp.hpp>

struct FakeSensingConfig
{
  std::optional<std::string> predicates_sequence_file;
  std::optional<std::string> functions_sequence_file;

  bool random_failure_enabled{false};
  double failure_probability{0.0};
  std::optional<unsigned int> seed;

  bool loop_sequence{false};
};

struct ExpectedPredicate
{
  std::string name;
  std::vector<std::string> args;
  bool value;
};

struct ExpectedFunction
{
  std::string name;
  std::vector<std::string> args;
  double value;
};

class FakeSensingSequences
{
public:
  FakeSensingSequences(
    const std::string & config_file,
    const rclcpp::Logger & logger = rclcpp::get_logger("FakeSensingSequences"));

  static std::shared_ptr<FakeSensingSequences> getInstance();

  bool sensePredicate(
    const std::string & name,
    const std::vector<std::string> & args);

  std::optional<double> senseFunction(
    const std::string & name,
    const std::vector<std::string> & args);

private:
  std::vector<ExpectedPredicate> predicateSequence_;
  std::vector<ExpectedFunction> functionSequence_;

  size_t predicateIndex_{0};
  size_t functionIndex_{0};

  FakeSensingConfig config_;

  // Random failure (predicates only)
  std::mt19937 rng_;
  std::bernoulli_distribution failure_dist_;

  static std::shared_ptr<FakeSensingSequences> instance_;

  rclcpp::Logger logger_;

  bool loadConfig(const std::string & config_file);
  void loadPredicateSequence(const std::string & filename);
  void loadFunctionSequence(const std::string & filename);

  static std::ostream & printArgs(
    std::ostream & os,
    const std::vector<std::string> & args);
};

#endif  // FAKE_SENSING_SEQUENCES_HPP

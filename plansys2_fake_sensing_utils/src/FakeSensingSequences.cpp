#include "fake_sensing_logic/FakeSensingSequences.hpp"

#include <fstream>
#include <sstream>
#include <random>

#include <yaml-cpp/yaml.h>
#include <ament_index_cpp/get_package_share_directory.hpp>

std::shared_ptr<FakeSensingSequences> FakeSensingSequences::instance_ = nullptr;

FakeSensingSequences::FakeSensingSequences(
  const std::string & config_file,
  const rclcpp::Logger & logger)
: logger_(logger)
{
  loadConfig(config_file);
}

std::shared_ptr<FakeSensingSequences> FakeSensingSequences::getInstance()
{
  if (!instance_) {
    const std::string cfg =
      ament_index_cpp::get_package_share_directory("plansys2_fake_sensing_utils") +
      "/config/fake_sensing_config.yaml";
    instance_.reset(new FakeSensingSequences(cfg));
  }
  return instance_;
}

bool FakeSensingSequences::loadConfig(const std::string & config_file)
{
  try {
    YAML::Node cfg = YAML::LoadFile(config_file);

    const std::string share_dir =
      ament_index_cpp::get_package_share_directory("plansys2_fake_sensing_utils");

    if (cfg["predicates_sequence_file"] && !cfg["predicates_sequence_file"].IsNull()) {
      config_.predicates_sequence_file =
        share_dir + "/" + cfg["predicates_sequence_file"].as<std::string>();
      loadPredicateSequence(*config_.predicates_sequence_file);
    }

    if (cfg["functions_sequence_file"] && !cfg["functions_sequence_file"].IsNull()) {
      config_.functions_sequence_file =
        share_dir + "/" + cfg["functions_sequence_file"].as<std::string>();
      loadFunctionSequence(*config_.functions_sequence_file);
    }

    if (cfg["random_failure"]) {
      config_.random_failure_enabled =
        cfg["random_failure"]["enabled"].as<bool>(false);
      config_.failure_probability =
        cfg["random_failure"]["probability"].as<double>(0.0);

      if (cfg["random_failure"]["seed"] && !cfg["random_failure"]["seed"].IsNull()) {
        config_.seed = cfg["random_failure"]["seed"].as<unsigned int>();
      }

      if (config_.seed.has_value()) {
        rng_.seed(*config_.seed);
      } else {
        std::random_device rd;
        rng_.seed(rd());
      }

      failure_dist_ = std::bernoulli_distribution(config_.failure_probability);
    }

    config_.loop_sequence = cfg["loop_sequence"].as<bool>(false);
  
  } catch (const std::exception & e) {
    RCLCPP_ERROR_STREAM(logger_, "Failed to load config: " << e.what());
    return false;
  }

  return true;
}

void FakeSensingSequences::loadPredicateSequence(const std::string & filename)
{
  std::ifstream file(filename);
  if (!file.is_open()) {
    RCLCPP_ERROR_STREAM(logger_, "Cannot open predicate sequence: " << filename);
    return;
  }

  predicateSequence_.clear();
  
  std::string line;
  while (std::getline(file, line)) {
    if (line.empty()) continue;

    std::istringstream iss(line);
    ExpectedPredicate p;
    std::string token;

    iss >> p.name;

    while (iss >> token) {
      if (token == "true" || token == "false") {
        p.value = (token == "true");
        break;
      }
      p.args.push_back(token);
    }

    predicateSequence_.push_back(p);
  }

  predicateIndex_ = 0;

  RCLCPP_INFO_STREAM(logger_, "Loaded " << predicateSequence_.size() << " predicate entries");
}

void FakeSensingSequences::loadFunctionSequence(const std::string & filename)
{
  std::ifstream file(filename);
  if (!file.is_open()) {
    RCLCPP_ERROR_STREAM(logger_, "Cannot open function sequence: " << filename);
    return;
  }

  functionSequence_.clear();

  std::string line;
  while (std::getline(file, line)) {
    if (line.empty()) continue;

    std::istringstream iss(line);
    ExpectedFunction f;

    iss >> f.name;

    std::string token;
    while (iss >> token) {
      if (iss.peek() == EOF) {
        f.value = std::stod(token);
        break;
      }
      f.args.push_back(token);
    }

    functionSequence_.push_back(f);
  }

  functionIndex_ = 0;

  RCLCPP_INFO_STREAM(logger_, "Loaded " << functionSequence_.size() << " function entries");
}

bool FakeSensingSequences::sensePredicate(
  const std::string & name,
  const std::vector<std::string> & args)
{
  if (predicateSequence_.empty()) {
    RCLCPP_WARN_STREAM(logger_, "Predicate sequence not loaded");
    return false;
  }

  if (predicateIndex_ >= predicateSequence_.size()) {
    if (!config_.loop_sequence) {
      return false;
    }
    predicateIndex_ = 0;
  }

  const auto & expected = predicateSequence_[predicateIndex_++];

  if (expected.name != name || expected.args != args) {
    std::ostringstream oss;
    oss << "Predicate mismatch\nExpected: " << expected.name << " ";
    printArgs(oss, expected.args);
    oss << "\nGot: " << name << " ";
    printArgs(oss, args);
    RCLCPP_ERROR_STREAM(logger_, oss.str());
  }

  bool result = expected.value;

  if (config_.random_failure_enabled && failure_dist_(rng_)) {
    result = !result;
    RCLCPP_INFO_STREAM(logger_, "Random failure injected for predicate");
  }

  return result;
}

std::optional<double> FakeSensingSequences::senseFunction(
  const std::string & name,
  const std::vector<std::string> & args)
{
  if (functionSequence_.empty()) {
    RCLCPP_WARN_STREAM(logger_, "Function sequence not loaded");
    return std::nullopt;
  }

  if (functionIndex_ >= functionSequence_.size()) {
    if (!config_.loop_sequence) return std::nullopt;
    functionIndex_ = 0;
  }

  const auto & expected = functionSequence_[functionIndex_++];

  if (expected.name != name || expected.args != args) {
    std::ostringstream oss;
    oss << "Function mismatch\nExpected: " << expected.name << " ";
    printArgs(oss, expected.args);
    oss << "\nGot: " << name << " ";
    printArgs(oss, args);
    RCLCPP_ERROR_STREAM(logger_, oss.str());
  }

  return expected.value;
}

std::ostream & FakeSensingSequences::printArgs(
  std::ostream & os,
  const std::vector<std::string> & args)
{
  os << "[";
  for (size_t i = 0; i < args.size(); ++i) {
    os << args[i];
    if (i + 1 < args.size()) os << ", ";
  }
  return os << "]";
}

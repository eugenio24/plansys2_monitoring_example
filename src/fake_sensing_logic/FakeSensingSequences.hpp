#ifndef FAKE_SENSING_SEQUENCES_HPP
#define FAKE_SENSING_SEQUENCES_HPP

#include <string>
#include <vector>
#include <cstdlib>
#include <memory>
#include <fstream>
#include <sstream>
#include <iostream>
#include <random>
#include <optional>

#include <yaml-cpp/yaml.h>

struct FakeSensingConfig
{
  std::string sequence_file;
  bool random_failure_enabled{false};
  double failure_probability{0.0};
  std::optional<unsigned int> seed;
  bool loop_sequence{false};
};

struct ExpectedEffect
{
  std::string predicate;
  std::vector<std::string> args;
  bool value;
};

class FakeSensingSequences
{
public:
  FakeSensingSequences(const std::string & config_file)
  : index_(0)
  {
    loadConfig(config_file);
  }

  static std::shared_ptr<FakeSensingSequences> getInstance(
    const std::string & config_file =
      "/plansys2_dev/plansys2_ws/src/plansys2_monitoring_example/src/fake_sensing_logic/fake_sensing_config.yaml")
  {
    if (!instance_) {
      instance_.reset(new FakeSensingSequences(config_file));
    }
    return instance_;
  }

  bool loadConfig(const std::string & config_file)
  {
    try {
      YAML::Node cfg = YAML::LoadFile(config_file);

      config_.sequence_file =
        cfg["sequence_file"].as<std::string>();

      if (cfg["random_failure"]) {
        config_.random_failure_enabled =
          cfg["random_failure"]["enabled"].as<bool>(false);
        config_.failure_probability =
          cfg["random_failure"]["probability"].as<double>(0.0);
        if (cfg["random_failure"]["seed"] && !cfg["random_failure"]["seed"].IsNull()) {
            config_.seed = cfg["random_failure"]["seed"].as<unsigned int>();
        } else {
            config_.seed.reset();
        }
          
        if (config_.seed.has_value()) {
          rng_.seed(*config_.seed);
        } else {
          std::random_device rd;
          rng_.seed(rd());
        }

        failure_dist_ =
          std::bernoulli_distribution(config_.failure_probability);
      }

      config_.loop_sequence = cfg["loop_sequence"].as<bool>(false);

      loadFromFile(config_.sequence_file);
      return true;

    } catch (const std::exception & e) {
      std::cerr << "[FakeSensingSequences] Config load failed: "
                << e.what() << std::endl;
      return false;
    }
  }


  void loadFromFile(const std::string& filename)
  {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open sequence file: " << filename << std::endl;
        return;
    }

    std::vector<ExpectedEffect> seq;
    std::string line;

    while (std::getline(file, line)) {
      if (line.empty()) continue;

      std::istringstream iss(line);
      ExpectedEffect effect;
      std::string token;

      iss >> effect.predicate;

      while (iss >> token) {
        if (token == "true" || token == "false") {
          effect.value = (token == "true");
          break;
        } else {
          effect.args.push_back(token);
        }
      }

      seq.push_back(effect);
    }

    sequence_ = seq;
    index_ = 0;
    std::cout << "[FakeSensingSequences] Loaded " << sequence_.size() << " effects from file." << std::endl;
  }

  bool get(const std::string& predicate,
           const std::vector<std::string>& args)
  {
    if (index_ >= sequence_.size()) {
      if(!config_.loop_sequence){
        std::cerr << "[FakeSensingSequences] end of sequence, false by default" << std::endl;
        return false;
      }else{
        std::cout << "[FakeSensingSequences] end of sequence, restarting sequence" << std::endl;
        index_ = 0;
      }
    }

    const auto& expected = sequence_[index_++];
  
    if (predicate != expected.predicate || args != expected.args) {
      std::cerr << "[FakeSensingSequences] WARNING: predicate mismatch. "
                << "Expected " << expected.predicate << " Got " << predicate << std::endl;
    }

    bool result = expected.value;

    if (config_.random_failure_enabled && failure_dist_(rng_)) {
      result = !result;
      std::cout << "[FakeSensingSequences] RANDOM FAILURE injected for "
                << expected.predicate << std::endl;
    }

    return result;
  }

private:
  std::vector<ExpectedEffect> sequence_;
  size_t index_;
  FakeSensingConfig config_;

  std::mt19937 rng_;
  std::bernoulli_distribution failure_dist_;

  static std::shared_ptr<FakeSensingSequences> instance_;
};

#endif // FAKE_SENSING_SEQUENCES_HPP
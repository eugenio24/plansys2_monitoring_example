#ifndef FAKE_SENSING_SEQUENCES_HPP
#define FAKE_SENSING_SEQUENCES_HPP

#include <string>
#include <vector>
#include <cstdlib>
#include <memory>
#include <fstream>
#include <sstream>
#include <iostream>

struct ExpectedEffect
{
  std::string predicate;
  std::vector<std::string> args;
  bool value;
};

class FakeSensingSequences
{
public:
  FakeSensingSequences()
  : index_(0)
  {
    std::string path = "/plansys2_dev/plansys2_ws/src/plansys2_monitoring_example/src/fake_sensing_logic/sequence.txt";
    loadFromFile(path);
  }

  static std::shared_ptr<FakeSensingSequences> getInstance()
  {
    if (!instance_) {
      instance_ = std::shared_ptr<FakeSensingSequences>(new FakeSensingSequences());
    }
    return instance_;
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
      std::cerr << "[FakeSensingSequences] end of sequence, false by default" << std::endl;
      return false;
    }

    const auto& expected = sequence_[index_];
    index_++;

    return expected.value;
  }

private:
  std::vector<ExpectedEffect> sequence_;
  size_t index_;

  static std::shared_ptr<FakeSensingSequences> instance_;
};

#endif // FAKE_SENSING_SEQUENCES_HPP
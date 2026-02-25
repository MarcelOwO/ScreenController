#ifndef I_FILE_PROCESSOR_H
#define I_FILE_PROCESSOR_H

namespace screen_controller {

class IFileProcessor {
 public:
  virtual ~IFileProcessor() = 0;
};

class ProcessorFactory {
  static std::unique_ptr<IFileProcessor> Create();
};

}  // namespace screen_controller

#endif

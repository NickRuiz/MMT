#pragma once

#include <string>
#include <map>

#include "mmt/Translator.h"
#include "legacy/ThreadPool.h"

namespace Moses2
{

class System;
class ManagerBase;
class Manager;

class TranslationTask: public Task
{
public:
  typedef std::map<std::string, float> weightmap_t;

  TranslationTask(System &system, const std::string &line, long translationId);
  virtual ~TranslationTask();
  virtual void Run();

  void SetContextWeights(const weightmap_t &weights) { m_contextWeights = weights; }
  const weightmap_t &GetContextWeights() const { return m_contextWeights; }

  TranslationResponse GetResult(size_t nbestListSize) const;

protected:
  ManagerBase *m_mgr;
  weightmap_t m_contextWeights;
};

}


//
// Created by Davide  Caroselli on 03/12/15.
//

#include <map>
#include <thread>
#include <mutex>

#include "TranslationTask.h"
#include "legacy/ThreadPool.h"
#include "MosesDecoder.h"
//#include "JNITranslator.h"
#include "Translator.h"
#include "legacy/Parameter.h"
#include "System.h"
//#include <moses/StaticData.h>
//#include <moses/FF/StatefulFeatureFunction.h>

using namespace mmt;
using namespace mmt::decoder;

namespace mmt {
    namespace decoder {
        class MosesDecoderImpl : public MosesDecoder {
            //MosesServer::JNITranslator m_translator;
            std::vector<feature_t> m_features;
            std::vector<IncrementalModel *> m_incrementalModels;
            Moses2::System &m_system;
            Moses2::ThreadPool m_pool;

            std::map<uint64_t, std::map<std::string, float>> m_sessionContext;
            std::mutex m_sessionsMut;
            static constexpr size_t kGlobalSession = 0; // empty-context default session
            size_t m_isession = 1;

        public:

            MosesDecoderImpl(Moses2::System &system);

            virtual std::vector<feature_t> getFeatures() override;

            virtual std::vector<float> getFeatureWeights(feature_t &feature) override;

            virtual void
            setDefaultFeatureWeights(const std::map<std::string, std::vector<float>> &featureWeights) override;

            virtual int64_t openSession(const std::map<std::string, float> &translationContext,
                                        const std::map<std::string, std::vector<float>> *featureWeights = NULL) override;

            virtual void closeSession(uint64_t session) override;

            virtual translation_t translate(const std::string &text, uint64_t session,
                                            const std::map<std::string, float> *translationContext,
                                            size_t nbestListSize) override;

            virtual void Add(const updateid_t &id, const domain_t domain, const std::vector<wid_t> &source,
                             const std::vector<wid_t> &target, const alignment_t &alignment) override;

            virtual std::vector<updateid_t> GetLatestUpdatesIdentifier() override;
        };
    }
}

MosesDecoder *MosesDecoder::createInstance(const char *inifile, Aligner *aligner, Vocabulary *vocabulary) {
    const char *argv[2] = {"-f", inifile};

    Moses2::Parameter params;

    if (!params.LoadParam(2, argv))
        return NULL;

    // initialize all "global" variables, which are stored in StaticData
    // note: this also loads models such as the language model, etc.
    /*
    if (!Moses::StaticData::LoadDataStatic(&params, "moses", aligner, vocabulary))
        return NULL;
        */
    // TODO: aligner, vocabulary

    Moses2::System system(params, aligner, vocabulary);

    return new MosesDecoderImpl(system);
}

MosesDecoderImpl::MosesDecoderImpl(Moses2::System &system) :
    m_features(), m_system(system),
    m_pool(system.options.server.numThreads, system.cpuAffinityOffset, system.cpuAffinityOffsetIncr)
{
    /*
    const std::vector<const Moses::StatelessFeatureFunction *> &slf = Moses::StatelessFeatureFunction::GetStatelessFeatureFunctions();
    for (size_t i = 0; i < slf.size(); ++i) {
        const Moses::FeatureFunction *feature = slf[i];
        feature_t f;
        f.name = feature->GetScoreProducerDescription();
        f.stateless = feature->IsStateless();
        f.tunable = feature->IsTuneable();
        f.ptr = (void *) feature;

        m_features.push_back(f);
        mmt::IncrementalModel *model = feature->GetIncrementalModel();
        if (model)
            m_incrementalModels.push_back(model);
    }

    const std::vector<const Moses::StatefulFeatureFunction *> &sff = Moses::StatefulFeatureFunction::GetStatefulFeatureFunctions();
    for (size_t i = 0; i < sff.size(); ++i) {
        const Moses::FeatureFunction *feature = sff[i];

        feature_t f;
        f.name = feature->GetScoreProducerDescription();
        f.stateless = feature->IsStateless();
        f.tunable = feature->IsTuneable();
        f.ptr = (void *) feature;

        m_features.push_back(f);
        mmt::IncrementalModel *model = feature->GetIncrementalModel();
        if (model)
            m_incrementalModels.push_back(model);
    }
     */

    m_sessionContext[kGlobalSession] = std::map<std::string, float>();
}

std::vector<feature_t> MosesDecoderImpl::getFeatures() {
    return m_features;
}

std::vector<float> MosesDecoderImpl::getFeatureWeights(feature_t &_feature) {
    /*
    Moses::FeatureFunction *feature = (Moses::FeatureFunction *) _feature.ptr;
    std::vector<float> weights;

    if (feature->IsTuneable()) {
        weights = Moses::StaticData::Instance().GetAllWeightsNew().GetScoresForProducer(feature);

        for (size_t i = 0; i < feature->GetNumScoreComponents(); ++i) {
            if (!feature->IsTuneableComponent(i)) {
                weights[i] = UNTUNEABLE_COMPONENT;
            }
        }
    }

    return weights;
     */
    return std::vector<float>();
}

void MosesDecoderImpl::setDefaultFeatureWeights(const std::map<std::string, std::vector<float>> &featureWeights) {
    //m_translator.set_default_feature_weights(featureWeights);
}

int64_t MosesDecoderImpl::openSession(const std::map<std::string, float> &translationContext,
                                      const std::map<std::string, std::vector<float>> *featureWeights) {

    // note: feature weights are now ignored. use setDefaultFeatureWeights() instead.
    // TODO: add docs, assert featureWeights==nullptr or remove it entirely

    std::lock_guard<std::mutex> lock(m_sessionsMut);
    uint64_t session = m_isession++;
    m_sessionContext[session] = translationContext;
    return session;
}

void MosesDecoderImpl::closeSession(uint64_t session) {
    std::lock_guard<std::mutex> lock(m_sessionsMut);
    if(session != kGlobalSession)
        m_sessionContext.erase(session);
}

translation_t MosesDecoderImpl::translate(const std::string &text, uint64_t session,
                                          const std::map<std::string, float> *translationContext,
                                          size_t nbestListSize) {

    boost::shared_ptr<Moses2::TranslationTask> task(new Moses2::TranslationTask(m_system, text, 0));
    if(translationContext)
        task->SetContextWeights(*translationContext);
    m_pool.Submit(task);
    task->Join();

    Moses2::TranslationResponse response = task->GetResult(nbestListSize);

    translation_t translation;

    translation.text = response.text;
    for (auto h: response.hypotheses)
        translation.hypotheses.push_back(hypothesis_t{h.text, h.score, h.fvals});
    translation.session = response.session;
    translation.alignment = response.alignment;

    return translation;
}

void MosesDecoderImpl::Add(const updateid_t &id, const domain_t domain, const std::vector<wid_t> &source,
                           const std::vector<wid_t> &target, const alignment_t &alignment) {
    for (size_t i = 0; i < m_incrementalModels.size(); ++i) {
        m_incrementalModels[i]->Add(id, domain, source, target, alignment);
    }
}

std::vector<updateid_t> MosesDecoderImpl::GetLatestUpdatesIdentifier() {
    std::unordered_map<stream_t, seqid_t> stream_map;

    for (auto it = m_incrementalModels.begin(); it != m_incrementalModels.end(); ++it) {
        std::vector<updateid_t> vec = (*it)->GetLatestUpdatesIdentifier();

        for (auto id = vec.begin(); id != vec.end(); ++id) {
            auto e = stream_map.emplace(id->stream_id, id->sentence_id);

            if (!e.second)
                e.first->second = std::min(e.first->second, id->sentence_id);
        }
    }

    std::vector<updateid_t> ret;
    ret.reserve(stream_map.size());
    for (auto it = stream_map.begin(); it != stream_map.end(); ++it) {
        ret.push_back(updateid_t(it->first, it->second));
    }

    return ret;
}

constexpr size_t MosesDecoderImpl::kGlobalSession;

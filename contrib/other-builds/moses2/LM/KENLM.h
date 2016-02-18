/*
 * KENLM.h
 *
 *  Created on: 4 Nov 2015
 *      Author: hieu
 */

#ifndef FF_LM_KENLM_H_
#define FF_LM_KENLM_H_

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include "../FF/StatefulFeatureFunction.h"
#include "lm/model.hh"
#include "../legacy/Factor.h"
#include "../legacy/Util2.h"
#include "../Word.h"

namespace Moses2
{

class Word;

class KENLM : public StatefulFeatureFunction
{
public:
  KENLM(size_t startInd, const std::string &line);
  virtual ~KENLM();

  virtual void Load(System &system);

  virtual FFState* BlankState(MemPool &pool) const;

  //! return the state associated with the empty hypothesis for a given sentence
  virtual void EmptyHypothesisState(FFState &state,
		  const Manager &mgr,
		  const InputType &input,
		  const Hypothesis &hypo) const;

  virtual void
  EvaluateInIsolation(MemPool &pool,
		  const System &system,
		  const Phrase &source,
		  const TargetPhrase &targetPhrase,
		  Scores &scores,
		  SCORE *estimatedScore) const;

  virtual void EvaluateWhenApplied(const Manager &mgr,
	const Hypothesis &hypo,
	const FFState &prevState,
	Scores &scores,
	FFState &state) const;

  /*
  virtual void EvaluateWhenAppliedNonBatch(const Manager &mgr,
    const Hypothesis &hypo,
    const FFState &prevState,
    Scores &scores,
	FFState &state) const
  {
	  EvaluateWhenApplied(mgr, hypo, prevState, scores, state);
  }
  */

  void SetParameter(const std::string& key, const std::string& value);

  virtual void InitializeForInput(const Manager &mgr) const;

  // clean up temporary memory, called after processing each sentence
  virtual void CleanUpAfterSentenceProcessing(const Manager &mgr) const;

protected:
  std::string m_path;
  FactorType m_factorType;
  bool m_lazy;
  const Factor *m_bos;
  const Factor *m_eos;

  typedef lm::ngram::ProbingModel Model;
  boost::shared_ptr<Model> m_ngram;
  std::vector<lm::WordIndex> m_lmIdLookup;

  typedef std::pair<const lm::ngram::State*, lm::WordIndex> LMCacheKey;
  typedef std::pair<float, const lm::ngram::State*> LMCacheValue;
  typedef boost::unordered_map<LMCacheKey, LMCacheValue> CacheColl;
  mutable boost::thread_specific_ptr<CacheColl> m_cache;

  void CalcScore(const Phrase &phrase, float &fullScore, float &ngramScore, std::size_t &oovCount) const;

  inline lm::WordIndex TranslateID(const Word &word) const
  {
	  std::size_t factor = word[m_factorType]->GetId();
	  return (factor >= m_lmIdLookup.size() ? 0 : m_lmIdLookup[factor]);
  }
  // Convert last words of hypothesis into vocab ids, returning an end pointer.
  lm::WordIndex *LastIDs(const Hypothesis &hypo, lm::WordIndex *indices) const;

  const LMCacheValue &ScoreAndCache(const Manager &mgr, const lm::ngram::State &in_state, const lm::WordIndex new_word) const;
};

}

#endif /* FF_LM_KENLM_H_ */

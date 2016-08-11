/*
 * Copyright 2016 Stoned Xander
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef HEADLESS_LOGIC_GENETIC_ALGORITHM
#define HEADLESS_LOGIC_GENERIC_ALGORITHM

namespace Headless {
    namespace Logic {
        /**
         * Here are proposed some implementations of General Algorithms.
         * Currently available GAs are:
         *  - Trivial.
         */
        namespace GA {

            /**
             * Trivial GA.
             * 1. Generate first pool.
             * 2. Evaluate pool against a testing environment.
             * 3. Save the elite.
             * 4. Create new pool from elite using set of operators.
             * 5. Back to step 2 until error is superior to specified
             *    or until generation number is inferior to specified.
             *
             * To this purpose, we need the following concepts :
             * @param <C> Candidates to be evaluated and modified.
             */
            template <typename C> class Trivial {

                public:

                    /**
                     * Constructor.
                     * @param pSize Pool Size.
                     */
                    Trivial(unsigned int pSize) : _count(pSize) {
                        _pool = new C*[pSize];
                        _score = new double[pSize];
                    }

                    /**
                     * Destructor.
                     */
                    ~Trivial() {
                        delete []_pool;
                        delete []_score;
                    }

                    /**
                     * Training.
                     * @param <E> Creation and evaluation environment type.
                     * @param <... M> Set of operators/mutators types.
                     * @param env Environment.
                     * @param maxGen Maximum number of generations.
                     * @param minErr Minimal accepable error.
                     * @param eliteSize Percentage of the pool to be taken for creating the next pool.
                     * @param store A store for results.
                     * @param size Size of the storage and maximum number of exit candidate.
                     * @param mutators Set of operators/mutators for new pool creation.
                     * @return The number of candidates stored in the specified buffer.
                     */
                    template <typename E, typename... M> int train(E* env,
                            unsigned int maxGen, double minErr, double eliteSize,
                            C** store, unsigned int size,
                            M... mutators) {
                        unsigned int eliteCount = _count * eliteSize;
                        // We assume that the pool is empty and needs to be filled.
                        env->reserve(_pool, _count);

                        // Loop on generations.
                        for(unsigned int g = 0;
                                (g < maxGen) && (evaluate(env) > minErr);
                                ++g) {
                            // At this point, the pool is full and sorted.
                            // Let's recycle candidates from eliteCount to _count - 1.
                            #pragma omp parallel for
                            for(unsigned int i = eliteCount; i < _count; ++i) {
                                // Randomly choose a mutators.
                                mutate(i, eliteCount, mutators);
                            }
                        }

                        // Clean-up the pool.
                        env->release(_pool, _count);
                    }

                private:
                    /**
                     * make a new offspring out of the available mutators.
                     */
                    template <typename M, typename... O> void mutate(unsigned int pos, unsigned int count,
                            M mutator, O... others) {
                        double rnd = 0; // TODO Random Score.
                        if(rnd < mutator->threshold()) {
                            mutator->mutate(_pool, count, _pool + pos);
                        } else {
                            mutate(pos, count, others);
                        }
                    }

                    template <typename M> void mutate(unsigned int pos, unsigned int count, M mutator) {
                        mutator->mutate(_pool, count, _pool + pos);
                    }

                    /**
                     * Evaluate the pool against the environment.
                     * @param <E> Environment type.
                     * @param env Environment.
                     * @return Minimal error. At return time, the pool is sorted
                     * using candidates scores.
                     */
                    template <typename E> double evaluate(E* env) {
                        // Evaluate ...
                        #pragma omp parallel for
                        for(unsigned int i = 0; i < _count; ++i) {
                            _score + i = env->evaluate(_pool + i);
                        }

                        // ... and sort.
                        qsort(0, _count - 1);

                        return _score[0];
                    }

                    /**
                     * Simple quick sort for our specific case.
                     * @param lo Lower bound.
                     * @param hi Higher bound.
                     */
                    void qsort(unsigned int lo, unsigned int hi) {
                        if(lo < hi) {
                            // We don't make fat partitionning as we are manipulating
                            // fine-grained over-distributed scores. We should not
                            // have arrays of identical scores.
                            unsigned int pivot = partition(lo, hi);
                            if((pivot - lo) < (hi - (pivot + 1))) {
                                qsort(lo, pivot);
                                qsort(pivot + 1, hi);
                            } else {
                                qsort(pivot + 1, hi);
                                qsort(lo, pivot);
                            }

                        }
                    }

                    unsigned int partition(unsigned int lo, unsigned int hi) {
                        double pivot = _score + lo;
                        unsigned int i = lo - 1;
                        unsigned int j = hi + 1;

                        for(;;) {
                            do {
                                ++i;
                            } while(_score + i < pivot);
                            do {
                                --j;
                            } while(_score + j > pivot);
                            if(i >= j) {
                                return j;
                            }
                            double score = _score + i;
                            C* candidate = _pool + i;
                            _score + i = _score + j;
                            _pool + i = _pool + j;
                            _score + j = score;
                            _pool + j = candidate;
                        }
                        return 0; // Should never happen.
                    }

                private:

                    /**
                     * Candidate pool.
                     */
                    C** _pool;

                    /**
                     * Pool score.
                     */
                    double *_score;

                    /**
                     * Pool count.
                     */
                    unsigned int _count;
            };

        } // Namespace 'GA'
    } // Namespace 'Logic'
} // Namespace 'Headless'

#endif

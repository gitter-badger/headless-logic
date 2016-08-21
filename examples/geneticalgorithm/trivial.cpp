#include <geneticalgorithm.hpp>
#include <iostream>
#include <string>
#include <random>

#define POOL_SIZE 256
#define MAX_GENERATION 10000
#define MIN_ERROR 0.1

// Candidate -----------------------------------------------------------------
class Candidate {
    public:
        Candidate();
        Candidate(const Candidate &);
        char *data() { return _data; }
        double distance(const Candidate &);
        const Candidate &operator=(const Candidate &);
    private:
        char _data[8];
};

Candidate::Candidate() {
    for(unsigned int i = 0; i < 8; ++i) {
        _data[i] = '\0';
    }
}

Candidate::Candidate(const Candidate &orig) {
    for(unsigned int i = 0; i < 8; ++i) {
        _data[i] = orig._data[i];
    }
}

double Candidate::distance(const Candidate &other) {
    double dist = 0;
    // Trivial distance computation.
    for(unsigned int i = 0; i < 8; ++i) {
        int current = _data[i] - other._data[i];
        if(current != 0) {
            dist += current<0?-current:current;
        }
    }
    return dist;
}

const Candidate& Candidate::operator=(const Candidate &other) {
    for(unsigned int i = 0; i < 8; ++i) {
        _data[i] = other._data[i];
    }
    return *this;
}

// Environment ---------------------------------------------------------------
class Environment {
    public:
        void set(Candidate &goal);
        void reserve(Candidate**&, unsigned int);
        void release(Candidate**, unsigned int);
        double evaluate(const Candidate *);
        Candidate *clone(const Candidate *);
    private:
        Candidate _goal;
};

void Environment::set(Candidate &goal) {
    _goal = goal;
}

void Environment::reserve(Candidate**& buffer, unsigned int size) {
    // perform an uniform distribution.
    for(unsigned int i = 0; i < size; ++i) {
        Candidate *candidate = new Candidate();
        char *data = candidate->data();
        data[0] = 'A';
        data[1] = 'B';
        data[2] = '\0';
        buffer[i] = candidate;
    }
}

void Environment::release(Candidate** buffer, unsigned int size) {
    for(unsigned int i = 0; i < size; ++i) {
        delete buffer[i];
    }
}

double Environment::evaluate(const Candidate *candidate) {
    return 0.01;
}

Candidate *Environment::clone(const Candidate *candidate) {
    return new Candidate(*candidate);
}

// Mate Mutator --------------------------------------------------------------
class MateMutator {
    public:
        double threshold();
        void mutate(Candidate**, unsigned int, Candidate*);
};

double MateMutator::threshold() { return 0.8; }

void MateMutator::mutate(Candidate** parents, unsigned int size, Candidate* offspring) {
  // Nothing for now.
}

// Classic Mutator -----------------------------------------------------------
class ClassicMutator {
    public:
        double threshold();
        void mutate(Candidate**, unsigned int, Candidate*);
};


double ClassicMutator::threshold() { return 0.3; }

void ClassicMutator::mutate(Candidate** parents, unsigned int size, Candidate* offspring) {
  // Nothing for now.
}



// Example Entry Point -------------------------------------------------------
int main(void) {
 /*
   std::random_device rd;
   std::mt19937 mt(rd());
   std::uniform_real_distribution<double> dist(0.0, 1.0);
 */

    Headless::Logic::GA::Trivial<Candidate> engine(POOL_SIZE);

    Environment env;
    MateMutator mate;
    ClassicMutator mutate;

    Candidate **store = new Candidate*[POOL_SIZE];

    int result = engine.train(&env,
            MAX_GENERATION, MIN_ERROR, POOL_SIZE / 10.0,
            store, POOL_SIZE,
            &mate, &mutate);

    std::cout << "Number of results " << result << std::endl;


    for(unsigned int i = 0; i < POOL_SIZE; ++i) {
        delete store[i];
    }
    delete[] store;
/*
    Environment env;
    Candidate **store = new Candidate*[POOL_SIZE];

    env.reserve(store, POOL_SIZE);
    for(unsigned int i = 0; i < POOL_SIZE; ++i) {
        std::cout << store[i]->data() << std::endl;
    }
    env.release(store, POOL_SIZE);*/

    return 0;
}

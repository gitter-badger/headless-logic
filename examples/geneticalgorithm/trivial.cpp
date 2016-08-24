#include <geneticalgorithm.hpp>
#include <iostream>
#include <string>
#include <random>

#define POOL_SIZE 256
#define MAX_GENERATION 1000000
#define MIN_ERROR 0.08

std::random_device s_rd;
std::mt19937 s_mt(s_rd());
std::uniform_int_distribution<char> s_upperDist('A', 'Z');
std::uniform_int_distribution<char> s_lowerDist('a', 'z');
std::uniform_int_distribution<int> s_cass(0, 1);
std::uniform_real_distribution<double> s_range(0, 1);


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
    return dist/7.0;
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
        for(unsigned int j = 0; j < 7; ++j) {
            if(s_cass(s_mt) == 1) { // upper case
                data[j] = s_upperDist(s_mt);
            } else {
                data[j] = s_lowerDist(s_mt);
            }
        }
        data[7] = '\0';
        buffer[i] = candidate;
    }
}

void Environment::release(Candidate** buffer, unsigned int size) {
    for(unsigned int i = 0; i < size; ++i) {
        delete buffer[i];
    }
}

double Environment::evaluate(const Candidate *candidate) {
    return _goal.distance(*candidate);
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
    // Let's take one of the offspring and mutate its genes !
    unsigned int index = static_cast<unsigned int>(s_range(s_mt) * (size - 1));
    Candidate *parent = parents[index];
    *offspring = *parent; // Copy ...
    // ... and mutate one of the character.
    index = static_cast<unsigned int>(s_range(s_mt) * 7);
    char *data = offspring->data();
    if(s_cass(s_mt) == 1) { // upper case
        data[index] = s_upperDist(s_mt);
    } else {
        data[index] = s_lowerDist(s_mt);
    }
}



// Example Entry Point -------------------------------------------------------
int main(void) {
    Headless::Logic::GA::Trivial<Candidate> engine(POOL_SIZE);

    Environment env;
    MateMutator mate;
    ClassicMutator mutate;

    Candidate goal;
    char *data = goal.data();
    data[0] = 'T';
    data[1] = 'e';
    data[2] = 's';
    data[3] = 't';
    data[4] = 'i';
    data[5] = 'n';
    data[6] = 'G';
    data[7] = '\0';
    

    env.set(goal);

    Candidate **store = new Candidate*[POOL_SIZE];

    int result = engine.train(&env,
            MAX_GENERATION, MIN_ERROR, 0.1,
            store, POOL_SIZE,
            &mutate);

    std::cout << "Number of results " << result << std::endl;


    for(unsigned int i = 0; i < POOL_SIZE; ++i) {
        std::cout << "#" << i << " : " << store[i]->data() << std::endl;
        delete store[i];
    }
    delete[] store;

    return 0;
}

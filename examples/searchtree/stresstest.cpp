#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <random>
#include <chrono>
#include "searchtree.hpp"
#include "common.hpp"

#define STRESSTEST_NODE_CARDINALITY 16
#define ELEMENT_BUFFER_SIZE 1024
#define ELEMENT_POOL_SIZE 64000
#define TEST_CHANGEKEY_OCCURENCE 10000000
#define TEST_SEARCH_OCCURENCE 10000000
#define TEST_FLUSHFILL_OCCURENCE 10000

/**
 * Main test procedure.
 */
int main(void) {
    // Search Tree.
    Region region(glm::vec4(0.0, 0.0, 1000.0, 1000.0));

    // Test on addition and removal.
    Element **pool = new Element*[ELEMENT_POOL_SIZE]; // Element pool.
    // - Initialize the pool.
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<double> dist(0.0, 1000.0);

    for(unsigned int i = 0; i < ELEMENT_POOL_SIZE; ++i) {
        pool[i] = new Element(glm::vec2(dist(mt), dist(mt)),
                std::string("Element#").append(std::to_string(i)));
    }

    unsigned int testPoolSize[] = { 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768 };
    unsigned int testCardinality[] = { 8, 16, 32, 64 };

    std::cout << "Node Cardinality, Element Count, Tree Fill, Depth, Remove/Change/Add, Flush/Fill, Find 8, Find 16, Find 32, Find 64, Find 128, Flush" << std::endl;

    for(unsigned int l = 0; l < 4; ++l) {
        unsigned int cardinality = testCardinality[l];

        for(unsigned int k = 0; k < 8; ++k) {
            Headless::Logic::SearchTree::Node<glm::vec2, Region, Element> tree(&region, cardinality);
            std::cout << cardinality << ", ";

            unsigned int poolSize = testPoolSize[k];
            std::cout << poolSize << ", ";

            auto start = std::chrono::steady_clock::now();
            // - Inserting the whole pool in the tree.
            for(unsigned int i = 0; i < poolSize; ++i) {
                tree.add(pool[i]);
            }
            auto end = std::chrono::steady_clock::now();
            auto diff = end - start;
            std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count()
                << ", ";
    
            DepthVisitor dVisitor;
            tree.visit(dVisitor);
            std::cout << dVisitor.depth() << ", ";
    
            // - Remove/Change Key/Add
            std::uniform_real_distribution<double> elemChooser(0, poolSize);
            start = std::chrono::steady_clock::now();
            for(unsigned int i = 0; i < TEST_CHANGEKEY_OCCURENCE; ++i) {
                Element *element = pool[(unsigned int) (elemChooser(mt))];
                tree.remove(element);
                element->set(glm::vec2(dist(mt), dist(mt)));
                tree.add(element);
            }
            end = std::chrono::steady_clock::now();
            diff = end - start;
            std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count() / TEST_CHANGEKEY_OCCURENCE << ", ";
    
            // Test on flush/removal.
            start = std::chrono::steady_clock::now();
            for(unsigned int i = 0; i < TEST_FLUSHFILL_OCCURENCE; ++i) {
                for(unsigned int j = 0; j < poolSize; ++j) {
                    tree.remove(pool[j]);
                    pool[j]->set(glm::vec2(dist(mt), dist(mt)));
                }
                for(unsigned int j = 0; j < poolSize; ++j) {
                    tree.add(pool[j]);
                }
            }
            end = std::chrono::steady_clock::now();
            diff = end - start;
            std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count() / TEST_FLUSHFILL_OCCURENCE << ", ";
    
            // Test on elements search.
            Region shape;
    
            Element **result = new Element*[ELEMENT_BUFFER_SIZE];
            double searchSize[] = { 8.0, 16.0, 32.0, 64.0, 128.0 };
    
            for(unsigned int j = 0; j < 5; ++j) {
                double size = searchSize[j];
                start = std::chrono::steady_clock::now();
                for(unsigned int i = 0; i < TEST_SEARCH_OCCURENCE; ++i) {
                    shape = glm::vec4(dist(mt), dist(mt), size, size);
                    (void) tree.retrieve(shape, result, ELEMENT_BUFFER_SIZE);
                }
                end = std::chrono::steady_clock::now();
                diff = end - start;
                std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count() / TEST_CHANGEKEY_OCCURENCE << ", ";
            }
    
            start = std::chrono::steady_clock::now();
            for(unsigned int i = 0; i < poolSize; ++i) {
                tree.remove(pool[i]);
            }
            std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count()
                << std::endl;
            delete []result;

#ifdef TREE_DEBUG
            MemoryInspector memoryInspector;
            std::cout << "digraph G {" << std::endl;
            memoryInspector.init();
            tree.deepVisit(memoryInspector);
            std::cout << "}" << std::endl;
#endif
        }
    }
    // Clean-up.
    for(unsigned int i = 0; i < ELEMENT_POOL_SIZE; ++i) {
        delete pool[i];
    }
    delete []pool;

    // Exit.
    return 0;
}

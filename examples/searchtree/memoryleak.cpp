#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <random>
#include <chrono>
#include "searchtree.hpp"
#include "common.hpp"

#define STRESSTEST_NODE_CARDINALITY 16
#define ELEMENT_BUFFER_SIZE 1024
#define ELEMENT_POOL_SIZE 32768
#define TEST_CHANGEKEY_OCCURENCE 100000
#define TEST_SEARCH_OCCURENCE 1
#define TEST_FLUSHFILL_OCCURENCE 5

#define ZONE_SIZE 128.0
/**
 * Main test procedure.
 */
int main(void) {
    // Search Tree.
    Region region(glm::vec4(0.0, 0.0, ZONE_SIZE, ZONE_SIZE));

    // Test on addition and removal.
    Element **pool = new Element*[ELEMENT_POOL_SIZE]; // Element pool.
    // - Initialize the pool.
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<double> dist(0.0, ZONE_SIZE);

    for(unsigned int i = 0; i < ELEMENT_POOL_SIZE; ++i) {
        pool[i] = new Element(glm::vec2(dist(mt), dist(mt)),
                std::string("Element#").append(std::to_string(i)));
    }

    unsigned int testPoolSize[] = { 128 };
    unsigned int testCardinality[] = { 3 };

    for(unsigned int l = 0; l < 1; ++l) {

        unsigned int cardinality = testCardinality[l];
        Headless::Logic::SearchTree::Node<glm::vec2, Region, Element> tree(&region,
            cardinality);


        for(unsigned int k = 0; k < 1; ++k) {
            unsigned int poolSize = testPoolSize[k];

            // - Inserting the whole pool in the tree.
            for(unsigned int i = 0; i < poolSize; ++i) {
                tree.add(pool[i]);
            }

            std::cout << ">>>>> Remove/Change Key/Add" << std::endl;
            // - Remove/Change Key/Add
            std::uniform_real_distribution<double> elemChooser(0, poolSize);
            for(unsigned int i = 0; i < TEST_CHANGEKEY_OCCURENCE; ++i) {
                Element *element = pool[(unsigned int) (elemChooser(mt))];
                tree.remove(element);
                element->set(glm::vec2(dist(mt), dist(mt)));
                tree.add(element);
            }
    
            std::cout << ">>>>> Flush/Add" << std::endl;
            // Test on flush/removal.
            for(unsigned int i = 0; i < TEST_FLUSHFILL_OCCURENCE; ++i) {
                for(unsigned int j = 0; j < poolSize; ++j) {
                    tree.remove(pool[j]);
                    pool[j]->set(glm::vec2(dist(mt), dist(mt)));
                }
                for(unsigned int j = 0; j < poolSize; ++j) {
                    tree.add(pool[j]);
                }
            }
   
            std::cout << ">>>>> Chech Consistency" << std::endl;
            ConsistencyVisitor cVisitor(cardinality);
            tree.visit(cVisitor);
            // Test on elements search.
            Region shape;
    
            Element **result = new Element*[ELEMENT_BUFFER_SIZE];
            unsigned int retrieved;
            double searchSize[] = { 8.0 };
    
            Visitor dumpVisitor;
            for(unsigned int j = 0; j < 1; ++j) {
                double size = searchSize[j];
                for(unsigned int i = 0; i < TEST_SEARCH_OCCURENCE; ++i) {
                    glm::vec4 gshape = glm::vec4(dist(mt), dist(mt), size, size);
                    shape = gshape;
                    retrieved = tree.retrieve(shape, result, ELEMENT_BUFFER_SIZE, &dumpVisitor);
                    std::cout << "Search At (" << gshape.x << ", " << gshape.y << ") - ("
                        << gshape.x + gshape.p << ", " << gshape.y + gshape.q << ")" << std::endl;
                    for(unsigned int m = 0; m < retrieved; ++m) {
                        std::cout << result[m]->name() << "\t";
                        glm::vec2 elemPos = result[m]->key();
                        std::cout << "(" << elemPos.x << ", " << elemPos.y << ")" << std::endl;
                    }
                }
            }
    
            std::cout << "Should have found : " << std::endl;
            for(unsigned int j = 0; j < poolSize; ++j) {
                glm::vec2 key = pool[j]->key();
                if(shape.contains(key)) {
                    std::cout << pool[j]->name();
                    std::cout << "\t(" << key.x << ", " << key.y << ")" << std::endl;
                }
            }

            for(unsigned int i = 0; i < poolSize/2; ++i) {
                tree.remove(pool[i]);
            }
            for(unsigned int i = 0; i < poolSize/2; ++i) {
                tree.add(pool[i]);
            }
            retrieved = tree.retrieve(shape, result, ELEMENT_BUFFER_SIZE);
            std::cout << "Re-Search" << std::endl;
            for(unsigned int m = 0; m < retrieved; ++m) {
                std::cout << result[m]->name() << "\t";
                glm::vec2 elemPos = result[m]->key();
                std::cout << "(" << elemPos.x << ", " << elemPos.y << ")" << std::endl;
            }

            delete []result;
            for(unsigned int i = 0; i < poolSize; ++i) {
                tree.remove(pool[i]);
            }
        }
#ifdef TREE_DEBUG
        std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        MemoryInspector inspector;
        inspector.init();
        tree.deepVisit(inspector);
        inspector.close();
#endif
    }
    // Clean-up.
    for(unsigned int i = 0; i < ELEMENT_POOL_SIZE; ++i) {
        delete pool[i];
    }
    delete []pool;

    // Exit.
    return 0;
}

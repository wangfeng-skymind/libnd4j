//
// @author raver119@gmail.com
//

#ifndef LIBND4J_INDICESLIST_H
#define LIBND4J_INDICESLIST_H

#include <initializer_list>
#include "NDIndex.h"

namespace nd4j {
    class IndicesList {
    protected:
        std::vector<NDIndex *> _indices;
    public:
        IndicesList(std::initializer_list<NDIndex *> list);

        int size();
        NDIndex* at(int idx);

        ~IndicesList();
    };
}
#endif //LIBND4J_INDICESLIST_H

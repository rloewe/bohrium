/*
This file is part of Bohrium and copyright (c) 2012 the Bohrium
team <http://www.bh107.org>.

Bohrium is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation, either version 3
of the License, or (at your option) any later version.

Bohrium is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the
GNU Lesser General Public License along with Bohrium.

If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include <map>
#include <vector>

#include <bh_instruction.hpp>
#include <jitk/block.hpp>
#include <jitk/statistics.hpp>


namespace bohrium {
namespace jitk {

class FuseCache {
private:
    // Help struct to contain the payload of the FuseCache
    struct CachePayload {
        std::vector<Block> block_list;
        std::vector<bh_base*> base_ids; // The base IDs corresponds to their position in `base_ids`
    };
    // The hash to payload map
    std::map<size_t, CachePayload> _cache;
public:
    // Some statistics
    jitk::Statistics &stat;

    // The constructor takes the statistic object
    FuseCache(jitk::Statistics &stat) : stat(stat) {}

    // Check the cache for a block list that matches 'instr_list'
    std::pair<std::vector<Block>, bool> get(const std::vector<bh_instruction *> &instr_list);
    // Insert 'block_list' as a hit when requesting 'instr_list'
    void insert(const std::vector<bh_instruction *> &instr_list, std::vector<Block> block_list);
};


} // jit
} // bohrium

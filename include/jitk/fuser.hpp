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

#ifndef __BH_JITK_FUSER_HPP
#define __BH_JITK_FUSER_HPP

#include <set>
#include <vector>

#include <jitk/block.hpp>
#include <bh_instruction.hpp>

namespace bohrium {
namespace jitk {

// Creates a block list based on the 'instr_list' where each instruction gets its own nested block
// NB: this function might reshape the instructions in 'instr_list'
std::vector<Block> fuser_singleton(const std::vector<bh_instruction *> &instr_list);

// Fuses 'block_list' in a serial naive manner
// 'min_threading' is the minimum amount of threading acceptable in the merged blocks
void fuser_serial(std::vector<Block> &block_list, uint64_t min_threading=0);

// Fuses 'block_list' in a topological breadth first manner
// 'min_threading' is the minimum amount of threading acceptable in the merged blocks
void fuser_breadth_first(std::vector<Block> &block_list, uint64_t min_threading=0);

// Fuses 'block_list' in a topological manner prioritizing fusion of reshapable blocks
// 'min_threading' is the minimum amount of threading acceptable in the merged blocks
void fuser_reshapable_first(std::vector<Block> &block_list, uint64_t min_threading=0);

// Fuses 'block_list' greedily
// 'min_threading' is the minimum amount of threading acceptable in the merged blocks
void fuser_greedy(std::vector<Block> &block_list, uint64_t min_threading=0);

} // jit
} // bohrium

#endif

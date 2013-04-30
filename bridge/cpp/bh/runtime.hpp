/*
This file is part of Bohrium and copyright (c) 2012 the Bohrium team:
http://bohrium.bitbucket.org

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
#ifndef __BOHRIUM_BRIDGE_CPP_RUNTIME
#define __BOHRIUM_BRIDGE_CPP_RUNTIME
#include <iostream>
#include <sstream>
#include <boost/ptr_container/ptr_map.hpp>

namespace bh {

typedef boost::ptr_map<unsigned int, bh_array> storage_type;
storage_type storage;

unsigned int keys = 1;

// Runtime : Definition

Runtime* Runtime::pInstance = 0;

Runtime* Runtime::instance()
{
    if (pInstance == 0) {
        pInstance = new Runtime;
    }
    return pInstance;
}

void stop()
{
    delete Runtime::instance();
}

Runtime::Runtime() : random_id(0), ext_in_queue(0), queue_size(0)
{
    bh_error err;
    char err_msg[100];

    self_component = bh_component_setup(NULL);
    bh_component_children( self_component, &children_count, &components );

    if(children_count != 1 || components[0]->type != BH_VEM) {
        fprintf(stderr, "Error in the configuration: the bridge must "
                        "have exactly one child of type VEM\n");
        exit(-1);
    }
    vem_component   = components[0];

    vem_init        = vem_component->init;
    vem_execute     = vem_component->execute;
    vem_shutdown    = vem_component->shutdown;

    vem_reg_func    = vem_component->reg_func;
    free(components);

    err = vem_init(vem_component);
    if (err) {
        fprintf(stderr, "Error in vem_init()\n");
        exit(-1);
    }

    //
    // Register extensions
    //
    err = vem_reg_func("bh_random", &random_id);
    if (err != BH_SUCCESS) {
        sprintf(err_msg, "Fatal error in the initialization of the user"
                        "-defined random operation: %s.\n",
                        bh_error_text(err));
        throw std::runtime_error(err_msg);
    }
    if (random_id <= 0) {
        sprintf(err_msg, "Fatal error in the initialization of the user"
                        "-defined random operation: invalid ID returned"
                        " (%ld).\n", (long)random_id);
        throw std::runtime_error(err_msg);
    }
}

Runtime::~Runtime()
{
    // Deconstructor is not called in a timely fashion.
    flush();
    vem_shutdown();
    bh_component_free(self_component);
    bh_component_free(vem_component);
}

bh_intp Runtime::get_queue_size()
{
    return queue_size;
}

/**
 * De-allocate all bh_arrays associated with BH_DISCARD instruction in the queue.
 *
 * @param count The max number of instructions to look at.
 * @return The number of bh_arrays that got de-allocated.
 */
inline
size_t Runtime::deallocate_meta(bh_intp count)
{
    size_t deallocated = 0;
    if (count > BH_CPP_QUEUE_MAX) {
        throw std::runtime_error("Trying to de-allocate more than physically possible.");
    }
    while(!garbage.empty()) {
        storage.erase(garbage.front());
        garbage.pop_front();
    }
    return deallocated;
}

/**
 * De-allocate all meta-data associated with any USERFUNCs in the instruction queue.
 *
 * @return The number of USERFUNCs that got de-allocated.
 */
inline
size_t Runtime::deallocate_ext()
{
    size_t deallocated = 0;

    if (ext_in_queue>0) {
        for(bh_intp i=0; i<ext_in_queue; i++) {
            if (ext_queue[i]->id == random_id) {
                free((bh_random_type*)ext_queue[i]);
                deallocated++;
            } else {
                throw std::runtime_error("Cannot de-allocate extension...");
            }
        }
        ext_in_queue = 0;
    }
    return deallocated;
}

/**
 * Sends the instruction-list to the bohrium runtime.
 *
 * NOTE: Assumes that the list is non-empty.
 */
inline
size_t Runtime::execute()
{
    size_t cur_size = queue_size;
    bh_error status = vem_execute(queue_size, queue);   // Send instructions to Bohrium
    queue_size = 0;                                     // Reset size of the queue

    if (status != BH_SUCCESS) {
        std::stringstream err_msg;
        err_msg << "Runtime::execute() -> vem_execute() failed: " << bh_error_text(status) << std::endl;
        /*
        for(int i=0; i<cur_size; i++) {
            bh_pprint_instr( &queue[i] );
        }*/

        throw std::runtime_error(err_msg.str());
    }

    deallocate_ext();
    deallocate_meta(cur_size);

    return cur_size;
}

/**
 * Flush the instruction-queue if it is about to get overflowed.
 *
 * This is used as a pre-condition to adding instructions to the queue.
 *
 * @return The number of instructions flushed.
 */
inline
size_t Runtime::guard()
{
    if (queue_size >= BH_CPP_QUEUE_MAX) {
        return execute();
    }
    return 0;
}

/**
 * Flush the instruction-queue.
 *
 * This will force the runtime system to execute the queued up instructions,
 * at least if there are any queued instructions.
 *
 * @return The number of instructions flushed.
 */
inline
size_t Runtime::flush()
{
    if (queue_size > 0) {
        return execute();
    }
    return 0;
}

template <typename T>
inline
multi_array<T>& Runtime::op()
{
    multi_array<T>* operand = new multi_array<T>();

    return *operand;
}

/**
 * Create an intermediate operand.
 */
template <typename T>
inline
multi_array<T>& Runtime::temp()
{
    multi_array<T>* operand = new multi_array<T>();
    unsigned int key = keys++;
    operand->link(key);
    operand->setTemp(true);
    storage.insert(key, new bh_array);
    assign_array_type<T>(&storage[key]);

    return *operand;
}

template <typename T>
inline
multi_array<T>& Runtime::temp(size_t n)
{
    multi_array<T>* operand = new multi_array<T>(n);
    operand->setTemp(true);
    return *operand;
}

/**
 * Create an intermediate operand based on another operand.
 */
template <typename T>
inline
multi_array<T>& Runtime::temp(multi_array<T>& input)
{
    multi_array<T>* operand = new multi_array<T>(input);
    operand->setTemp(true);
    return *operand;
}

/**
 * Create an alias/segment/view of the supplied base operand.
 */
template <typename T>
inline
multi_array<T>& Runtime::view(multi_array<T>& base)
{
    multi_array<T>* operand = new multi_array<T>();
    storage[operand->getKey()].base = bh_base_array(&storage[base.getKey()]);
    return *operand;
}

/**
 * Create an intermediate alias/segment/view of the supplied base operand.
 */
template <typename T>
inline
multi_array<T>& Runtime::temp_view(multi_array<T>& base)
{
    multi_array<T>* operand = new multi_array<T>();
    storage[operand->getKey()].base = bh_base_array(&storage[base.getKey()]);
    operand->setTemp(true);
    return *operand;
}

template <typename T>
inline
void Runtime::enqueue(bh_opcode opcode, multi_array<T>& op0, multi_array<T>& op1, multi_array<T>& op2)
{
    bh_instruction* instr;

    guard();
    
    instr = &queue[queue_size++];
    instr->opcode = opcode;
    instr->operand[0] = &storage[op0.getKey()];
    instr->operand[1] = &storage[op1.getKey()];
    instr->operand[2] = &storage[op2.getKey()];

    if (op1.getTemp()) { delete &op1; }
    if (op2.getTemp()) { delete &op2; }
}

template <typename T>
inline
void Runtime::enqueue(bh_opcode opcode, multi_array<T>& op0, multi_array<T>& op1, const T& op2)
{
    bh_instruction* instr;

    guard();

    instr = &queue[queue_size++];
    instr->opcode = opcode;
    instr->operand[0] = &storage[op0.getKey()];
    instr->operand[1] = &storage[op1.getKey()];
    instr->operand[2] = NULL;
    assign_const_type( &instr->constant, op2 );

    if (op1.getTemp()) { delete &op1; }
}

template <typename T>
inline
void Runtime::enqueue(bh_opcode opcode, multi_array<T>& op0, const T& op1, multi_array<T>& op2)
{
    bh_instruction* instr;

    guard();

    instr = &queue[queue_size++];
    instr->opcode = opcode;
    instr->operand[0] = &storage[op0.getKey()];
    instr->operand[1] = NULL;
    instr->operand[2] = &storage[op2.getKey()];
    assign_const_type( &instr->constant, op1 );

    if (op2.getTemp()) { delete &op2; }
}

template <typename T>
inline
void Runtime::enqueue(bh_opcode opcode, multi_array<T>& op0, multi_array<T>& op1)
{
    bh_instruction* instr;

    guard();

    instr = &queue[queue_size++];
    instr->opcode = opcode;
    instr->operand[0] = &storage[op0.getKey()];
    instr->operand[1] = &storage[op1.getKey()];
    instr->operand[2] = NULL;

    if (op1.getTemp()) { delete &op1; }
}

template <typename T>
inline
void Runtime::enqueue(bh_opcode opcode, multi_array<T>& op0, const T& op1)
{
    bh_instruction* instr;

    guard();

    instr = &queue[queue_size++];
    instr->opcode = opcode;
    instr->operand[0] = &storage[op0.getKey()];
    instr->operand[1] = NULL;
    instr->operand[2] = NULL;
    assign_const_type( &instr->constant, op1 );
}

template <typename T>
inline
void Runtime::enqueue(bh_opcode opcode, multi_array<T>& op0)
{
    bh_instruction* instr;

    guard();

    instr = &queue[queue_size++];
    instr->opcode = opcode;
    instr->operand[0] = &storage[op0.getKey()];
    instr->operand[1] = NULL;
    instr->operand[2] = NULL;
}

template <typename Ret, typename In>    // x = y
inline
void Runtime::enqueue(bh_opcode opcode, multi_array<Ret>& op0, multi_array<In>& op1)
{
    bh_instruction* instr;

    guard();

    instr = &queue[queue_size++];
    instr->opcode = opcode;
    instr->operand[0] = &storage[op0.getKey()];
    instr->operand[1] = &storage[op1.getKey()];
    instr->operand[2] = NULL;

    if (op1.getTemp()) { delete &op1; }
}

template <typename Ret, typename In>    // x = y < z
inline
void Runtime::enqueue(bh_opcode opcode, multi_array<Ret>& op0, multi_array<In>& op1, multi_array<In>& op2)
{
    bh_instruction* instr;

    guard();

    instr = &queue[queue_size++];
    instr->opcode = opcode;
    instr->operand[0] = &storage[op0.getKey()];
    instr->operand[1] = &storage[op1.getKey()];
    instr->operand[2] = &storage[op2.getKey()];
    assign_const_type( &instr->constant, op2 );

    if (op1.getTemp()) { delete &op1; }
    if (op2.getTemp()) { delete &op2; }
}

template <typename Ret, typename In>    // x = y < 1
inline
void Runtime::enqueue(bh_opcode opcode, multi_array<Ret>& op0, multi_array<In>& op1, const In& op2)
{
    bh_instruction* instr;

    guard();
    instr = &queue[queue_size++];
    instr->opcode = opcode;
    instr->operand[0] = &storage[op0.getKey()];
    instr->operand[1] = &storage[op1.getKey()];
    instr->operand[2] = NULL;
    assign_const_type( &instr->constant, op2 );

    if (op1.getTemp()) { delete &op1; }
}

template <typename Ret, typename In>    // reduce(), pow()
inline
void Runtime::enqueue(bh_opcode opcode, multi_array<Ret>& op0, multi_array<Ret>& op1, const In& op2)
{
    bh_instruction* instr;

    guard();

    instr = &queue[queue_size++];
    instr->opcode = opcode;
    instr->operand[0] = &storage[op0.getKey()];
    instr->operand[1] = &storage[op1.getKey()];
    instr->operand[2] = NULL;
    assign_const_type( &instr->constant, op2 );

    if (op1.getTemp()) { delete &op1; }
}

template <typename Ret, typename In>    // x = 1 < y
inline
void Runtime::enqueue(bh_opcode opcode, multi_array<Ret>& op0, const In& op1, multi_array<In>& op2)
{
    bh_instruction* instr;

    guard();

    instr = &queue[queue_size++];
    instr->opcode = opcode;
    instr->operand[0] = &storage[op0.getKey()];
    instr->operand[1] = NULL;
    instr->operand[2] = &storage[op2.getKey()];
    assign_const_type( &instr->constant, op1 );

    if (op2.getTemp()) { delete &op2; }
}

template <typename T>           // Userfunc / extensions
inline
void Runtime::enqueue(bh_userfunc* rinstr)
{
    bh_instruction* instr;

    guard();   

    instr = &queue[queue_size++];
    instr->opcode        = BH_USERFUNC;
    instr->userfunc      = (bh_userfunc *) rinstr;

    ext_queue[ext_in_queue++] = instr->userfunc;
}

//
//  Copy... properties
//
template <typename Ret, typename In>
void equiv(multi_array<Ret>& ret, multi_array<In>& in)
{
    bh_array *ret_a, *in_a;

    ret_a   = &storage[ret.getKey()];
    in_a    = &storage[in.getKey()];

    ret_a->base        = NULL;
    ret_a->ndim        = in_a->ndim;
    ret_a->start       = in_a->start;
    for(bh_index i=0; i< in_a->ndim; i++) {
        ret_a->shape[i] = in_a->shape[i];
    }
    for(bh_index i=0; i< in_a->ndim; i++) {
        ret_a->stride[i] = in_a->stride[i];
    }
    ret_a->data        = NULL;

    assign_array_type<Ret>(ret_a);
}

template <typename T>
T& scalar(multi_array<T>& op)
{
    Runtime::instance()->enqueue((bh_opcode)BH_SYNC, op);
    Runtime::instance()->flush();

    bh_array* op_a = &storage[op.getKey()];
    T* data = (T*)(bh_base_array( op_a )->data);
    data += op_a->start;

    if (op.getTemp()) {
        delete &op;
    }

    return data[0];
}


void Runtime::trash(unsigned int key)
{
    garbage.push_back(key);
}

}
#endif

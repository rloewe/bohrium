/*
This file is part of cphVB and copyright (c) 2012 the cphVB team:
http://cphvb.bitbucket.org

cphVB is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as 
published by the Free Software Foundation, either version 3 
of the License, or (at your option) any later version.

cphVB is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the 
GNU Lesser General Public License along with cphVB. 

If not, see <http://www.gnu.org/licenses/>.
*/

#include <mpi.h>
#include <cassert>
#include <set>
#include <cphvb.h>
#include "dispatch.h"
#include "darray_extension.h"


//Message buffer, current offset and buffer size
static int buf_size=sizeof(dispatch_msg);
static dispatch_msg *msg=NULL;

/* Initiate the dispatch system. */
cphvb_error dispatch_reset(void)
{
    const int dms = CPHVB_CLUSTER_DISPATCH_DEFAULT_MSG_SIZE;
    assert(((int)dms) > sizeof(dispatch_msg));
    if(msg == NULL)
    {
        msg = (dispatch_msg*) malloc(dms);
        if(msg == NULL)
            return CPHVB_OUT_OF_MEMORY;
        buf_size = dms;
    }
    msg->size = 0;
    return CPHVB_SUCCESS;
}

    
/* Finalize the dispatch system. */
cphvb_error dispatch_finalize(void)
{
    free(msg);
    return CPHVB_SUCCESS;
}


/* Reserve memory on the send message payload.
 * @size is the number of bytes to reserve
 * @payload is the output pointer to the reserved memory
 */
cphvb_error dispatch_reserve_payload(cphvb_intp size, void **payload)
{
    cphvb_intp new_msg_size = sizeof(dispatch_msg) + msg->size + size;
    //Expand the buffer if need
    if(buf_size < new_msg_size)
    {
        buf_size = new_msg_size*2;
        msg = (dispatch_msg*) realloc(msg, buf_size);
        if(msg == NULL)
           return CPHVB_OUT_OF_MEMORY;
    }
    *payload = msg->payload + msg->size;
    msg->size += size;
    return CPHVB_SUCCESS;
}


/* Add data to the send message payload.
 * @size is the size of the data in bytes
 * @data is the data to add to the send buffer
 */
cphvb_error dispatch_add2payload(cphvb_intp size, const void *data)
{
    cphvb_error e;
    void *payload;
    //Reserve memory on the send message
    if((e  = dispatch_reserve_payload(size, &payload)) != CPHVB_SUCCESS)
        return e;
    //Copy 'data' to the send message
    memcpy(payload, data, size);
    return CPHVB_SUCCESS;
}


/* Send payload to all slave processes.
 * @type is the type of the message
*/
cphvb_error dispatch_send(int type)
{
    const int dms = CPHVB_CLUSTER_DISPATCH_DEFAULT_MSG_SIZE;
    int e;

    //Create message
    msg->type = type;
        
    if((e = MPI_Bcast(msg, dms, MPI_BYTE, 0, MPI_COMM_WORLD)) != 0)
        return CPHVB_ERROR;
       
    int size_left = sizeof(dispatch_msg) + msg->size - dms;
    if(size_left > 0)
    {
        if((e = MPI_Bcast(((char*)msg) + dms, size_left, MPI_BYTE, 0, MPI_COMM_WORLD)) != 0)
            return CPHVB_ERROR;
    }
    return CPHVB_SUCCESS;
}


/* Receive payload from master process.
 * @msg the received message (should not be freed)
*/
cphvb_error dispatch_recv(dispatch_msg **message)
{
    int e;
    const int dms = CPHVB_CLUSTER_DISPATCH_DEFAULT_MSG_SIZE;
    
    //Get header of the message
    if((e = MPI_Bcast(msg, dms, MPI_BYTE, 0, MPI_COMM_WORLD)) != 0)
        return CPHVB_ERROR;

    //Read total message size
    int size = sizeof(dispatch_msg) + msg->size;
 
    //Expand the read buffer if need  
    if(buf_size < size)
    {
        buf_size = size;
        msg = (dispatch_msg*) realloc(msg, buf_size);
        if(msg == NULL)
           return CPHVB_OUT_OF_MEMORY;
    }
 
    //Get rest of the message
    int size_left = size - dms;
    if(size_left > 0)
    {
        if((e = MPI_Bcast(((char*)msg) + dms, size_left, MPI_BYTE, 0, MPI_COMM_WORLD)) != 0)
            return CPHVB_ERROR;
    }

    *message = msg;
    return CPHVB_SUCCESS;
}


/* Dispatch an instruction list to the slaves, which includes new array-structs.
 * @count is the number of instructions in the list
 * @inst_list is the instruction list
 */
static std::set<cphvb_array*> known_arrays;
cphvb_error dispatch_inst_list(cphvb_intp count,
                               const cphvb_instruction inst_list[])
{
    cphvb_error e;

    if((e = dispatch_reset()) != CPHVB_SUCCESS)
        return e;

    /* The execution message has the form:
     * 1   x cphvb_intp NOI //number of instructions
     * NOI x cphvb_instruction //instruction list
     * 1   x cphvb_intp NOA //number of new arrays
     * NOA x darray //list of new arrays unknown to the slaves
     */

    //Pack the number of instructions (NOI).
    if((e = dispatch_add2payload(sizeof(cphvb_intp), &count)) != CPHVB_SUCCESS)
        return e;
    
    //Pack the instruction list.
    if((e = dispatch_add2payload(count * sizeof(cphvb_instruction), inst_list)) != CPHVB_SUCCESS)
        return e;
 
    //Make reservation for the number of new arrays (NOA).
    cphvb_intp msg_noa_offset, noa=0;
    char *msg_noa;
    if((e = dispatch_reserve_payload(sizeof(cphvb_intp), (void**) &msg_noa)) != CPHVB_SUCCESS)
        return e;

    //We need a message offset instead of a pointer since dispatch_reserve_payload() may 
    //re-allocate the 'msg_noa' pointer at a later time.
    msg_noa_offset = msg_noa - msg->payload;

    //Pack the array list.
    for(cphvb_intp i=0; i<count; ++i)
    {
        const cphvb_instruction *inst = &inst_list[i];
        assert(inst->opcode != CPHVB_USERFUNC);

        int nop = cphvb_operands_in_instruction(inst);
        for(cphvb_intp j=0; j<nop; ++j)
        {
            cphvb_array *op;
            if(inst->opcode == CPHVB_USERFUNC)
                op = inst->userfunc->operand[j];
            else
                op = inst->operand[j];
 
            if(cphvb_is_constant(op))
                continue;//No need to exchange constants

            if(known_arrays.count(op) == 0)//The array is unknown to the slaves.
            {
                darray *dary;
                if((e = dispatch_reserve_payload(sizeof(darray),(void**) &dary)) != CPHVB_SUCCESS)
                    return e;
                //The master-process's memory pointer is the id of the array.
                dary->id = (cphvb_intp) op;
                dary->global_ary = *op;
                ++noa;
                if(op->base != NULL && known_arrays.count(op) == 0)//Also check the base-array.
                {
                    if((e = dispatch_reserve_payload(sizeof(darray),(void**) &dary)) != CPHVB_SUCCESS)
                        return e;
                    dary->id = (cphvb_intp) op->base;
                    dary->global_ary = *op->base;
                    ++noa;
                }
            }
        }        
    }
    msg->payload[msg_noa_offset] = noa;//Save the number of new arrays

    return dispatch_send(CPHVB_CLUSTER_DISPATCH_EXEC);
}

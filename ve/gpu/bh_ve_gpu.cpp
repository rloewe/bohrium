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

#include <iostream>
#include <stdexcept>
#include <bh.h>
#include "bh_ve_gpu.h"

bh_error bh_ve_gpu_init(bh_component* _component)
{
    component = _component;
    try {
        resourceManager = new ResourceManager(component);
        instructionScheduler = new InstructionScheduler(resourceManager);
    } 
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return BH_ERROR;
    }
    return BH_SUCCESS;
}

bh_error bh_ve_gpu_execute(bh_intp instruction_count,
                                 bh_instruction instruction_list[])
{
    try 
    {
        return instructionScheduler->schedule(instruction_count, instruction_list);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return BH_ERROR;
    }
}

bh_error bh_ve_gpu_shutdown()
{
    delete instructionScheduler;
    delete resourceManager;
    return BH_SUCCESS;
}

bh_error bh_ve_gpu_reg_func(char *fun, 
                                  bh_intp *id)
{
    bh_userfunc_impl userfunc;
    bh_component_get_func(component, fun, &userfunc);
    if (userfunc != NULL)
    {
        instructionScheduler->registerFunction(fun, id, userfunc);
    	return BH_SUCCESS;
    }
    else
	    return BH_USERFUNC_NOT_SUPPORTED;
}
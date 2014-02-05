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

#ifndef __RESOURCEMANAGER_HPP
#define __RESOURCEMANAGER_HPP

#include "cl.hpp"
#include <vector>
#include <map>
#include <bh.h>
#ifdef STATS
#include "timing.h"
#endif

class ResourceManager
{
private:
    bh_component* component;
    cl::Context context;
    std::vector<cl::Device> devices;
    std::vector<cl::CommandQueue> commandQueues;
    size_t maxWorkGroupSize;
    cl_uint maxWorkItemDims;
    std::vector<size_t> maxWorkItemSizes;
    std::vector<size_t> localShape1D;
    std::vector<size_t> localShape2D;
    std::vector<size_t> localShape3D;
    bool float64;
    bool float16;
    void calcLocalShape();
    void registerExtensions(std::vector<std::string> extensions);
public:
#ifdef STATS
    struct cl_stat {cl_ulong queued; cl_ulong submit; cl_ulong start; cl_ulong end;}; 
    double batchBuild;
    double batchSource;
    double resourceCreateKernel;
    std::vector<cl_stat> resourceBufferWrite;
    std::vector<cl_stat> resourceBufferRead;
    std::vector<cl_stat> resourceKernelExecute;
    ~ResourceManager();
    static void CL_CALLBACK eventProfiler(cl_event event, cl_int eventStatus, void* total);
#endif
    ResourceManager(bh_component* _component);
    cl::Buffer* createBuffer(size_t size);
    // We allways read synchronous with at most one event to wait for.
    // Because we are handing off the array
    void readBuffer(const cl::Buffer& buffer,
                    void* hostPtr, 
                    cl::Event waitFor,
                    unsigned int device);
    // We allways write asynchronous with NO events to wait for.
    // Because we just recieved the array from upstream
    cl::Event enqueueWriteBuffer(const cl::Buffer& buffer,
                                 const void* hostPtr, 
                                 std::vector<cl::Event> waitFor, 
                                 unsigned int device);
    cl::Event completeEvent();
    cl::Kernel createKernel(const std::string& source, 
                            const std::string& kernelName);
    std::vector<cl::Kernel> createKernelsFromFile(const std::string& fileName, 
                                                  const std::vector<std::string>& kernelNames);
    std::vector<cl::Kernel> createKernels(const std::string& source, 
                                          const std::vector<std::string>& kernelNames);
    cl::Event enqueueNDRangeKernel(const cl::Kernel& kernel, 
                                   const cl::NDRange& globalSize,
                                   const cl::NDRange& localSize,
                                   const std::vector<cl::Event>* waitFor,
                                   unsigned int device);
    std::vector<size_t> localShape(const std::vector<size_t>& globalShape);
    std::string getKernelPath();
    std::string getIncludeStr();
    bool float16support();
    bool float64support();
    bh_error childExecute(bh_ir* bhir);
};

#endif


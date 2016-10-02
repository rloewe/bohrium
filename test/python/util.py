from __future__ import print_function
import numpy as np
import random
import operator


class TYPES:
    NORMAL_INT   = ['np.int32','np.int64','np.uint32','np.uint64']
    ALL_INT      = NORMAL_INT + ['np.int8','np.int16','np.uint8','np.uint16']
    SIGNED_INT   = ['np.int8', 'np.int16', 'np.int32','np.int64']
    UNSIGNED_INT = list(set(ALL_INT) - set(SIGNED_INT))
    COMPLEX      = ['np.complex64', 'np.complex128']
    FLOAT = ['np.float32','np.float64']
    ALL_SIGNED   = SIGNED_INT + FLOAT + COMPLEX
    NORMAL       = NORMAL_INT + FLOAT
    ALL          = ALL_INT + FLOAT + COMPLEX


def gen_shapes(max_ndim, max_dim, iters=0, min_ndim=1):
    for ndim in xrange(min_ndim,max_ndim+1):
        shape = [1]*ndim
        if iters:
            yield shape #Min shape
            yield [max_dim]*(ndim) #Max shape
            for _ in xrange(iters):
                for d in xrange(len(shape)):
                    shape[d] = np.random.randint(1,max_dim)
                yield shape
        else:
            finished = False
            while not finished:
                yield shape
                #Find next shape
                d = ndim-1
                while True:
                    shape[d] += 1
                    if shape[d] > max_dim:
                        shape[d] = 1
                        d -= 1
                        if d < 0:
                            finished = True
                            break
                    else:
                        break


def gen_arrays(random_state_name, max_ndim, max_dim=10, min_ndim=1, samples_in_each_ndim=3, dtype="np.float32", bh_arg="BH"):
    for shape in gen_shapes(max_ndim, max_dim, samples_in_each_ndim, min_ndim):
        cmd = "%s.random(%s, dtype=%s, bohrium=%s)" % (random_state_name, shape, dtype, bh_arg)
        yield (cmd, shape)


class ViewOfDim:
    def __init__(self, start, step, end):
        self.start = start
        self.step = step
        self.end = end

    def size(self):
        ret = 0
        i = self.start
        while i < self.end:
            ret += 1
            i += self.step
        return ret

    def write(self):
        return "%d:%d:%d" % (self.start, self.end, self.step)


def write_subscription(view):
    ret = "["
    for dim in view[:-1]:
        ret += "%s, "%dim.write()
    ret += "%s]"%view[-1].write()
    return ret


def random_subscription(shape):
    view = []
    view_shape = []
    for dim in shape:
        start = random.randint(0, dim-1)
        if dim > 3:
            step = random.randint(1, dim/2)
        else:
            step = 1
        if start+1 < dim-1:
            end = random.randint(start+1, dim-1)
        else:
            end = start+1
        v = ViewOfDim(start, step, end)
        view.append(v)
        view_shape.append(v.size())
    return write_subscription(view), view_shape


def gen_random_arrays(random_state_name, max_ndim, max_dim=10, min_ndim=1, samples_in_each_ndim=3, dtype="np.float32", bh_arg="BH"):
    for cmd, shape in gen_arrays(random_state_name, max_ndim, max_dim, min_ndim, samples_in_each_ndim, dtype, bh_arg):
        yield ("%s" % cmd, shape)
        if reduce(operator.mul, shape) > 1:
            sub_tried = set()
            for _ in range(samples_in_each_ndim):
                sub, vshape = random_subscription(shape)
                if sub not in sub_tried:
                    yield ("%s%s" % (cmd, sub), vshape)
                    sub_tried.add(sub)

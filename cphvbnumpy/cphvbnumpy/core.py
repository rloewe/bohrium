"""
Core
~~~~

The ``core`` module provide the essential functions, such as all the array creation functions, diagonal and matrix multiplication.

"""
import numpy as np
from numpy import *
import cphvbbridge as bridge

def empty(shape, dtype=float, cphvb=True):
    """
    Return a new matrix of given shape and type, without initializing entries.

    Parameters
    ----------
    shape : int or tuple of int
        Shape of the empty matrix.
    dtype : data-type, optional
        Desired output data-type.
    cphvb : boolean, optional
        Determines whether it is a cphVB-enabled array or a regular NumPy array

    See Also
    --------
    empty_like, zeros

    Notes
    -----
    The order of the data in memory is always row-major (C-style).
    
    `empty`, unlike `zeros`, does not set the matrix values to zero,
    and may therefore be marginally faster.  On the other hand, it requires
    the user to manually set all the values in the array, and should be
    used with caution.

    Examples
    --------
    >>> import numpy.matlib
    >>> np.matlib.empty((2, 2))    # filled with random data
    matrix([[  6.76425276e-320,   9.79033856e-307],
            [  7.39337286e-309,   3.22135945e-309]])        #random
    >>> np.matlib.empty((2, 2), dtype=int)
    matrix([[ 6600475,        0],
            [ 6586976, 22740995]])                          #random

    """

    return np.empty(shape, dtype=dtype, cphvb=cphvb)

def ones(shape, dtype=float, cphvb=True):
    """
    Matrix of ones.

    Return a matrix of given shape and type, filled with ones.

    Parameters
    ----------
    shape : {sequence of ints, int}
        Shape of the matrix
    dtype : data-type, optional
        The desired data-type for the matrix, default is np.float64.
    cphvb : boolean, optional
        Determines whether it is a cphVB-enabled array or a regular NumPy array

    Returns
    -------
    out : matrix
        Matrix of ones of given shape, dtype, and order.

    See Also
    --------
    ones : Array of ones.
    matlib.zeros : Zero matrix.

    Notes
    -----
    The order of the data in memory is always row-major (C-style).

    If `shape` has length one i.e. ``(N,)``, or is a scalar ``N``,
    `out` becomes a single row matrix of shape ``(1,N)``.

    Examples
    --------
    >>> np.matlib.ones((2,3))
    matrix([[ 1.,  1.,  1.],
            [ 1.,  1.,  1.]])

    >>> np.matlib.ones(2)
    matrix([[ 1.,  1.]])

    """

    A = empty(shape, dtype=dtype, cphvb=cphvb)
    A[:] = 1
    return A

def zeros(shape, dtype=float, cphvb=True):
    """
    Return a matrix of given shape and type, filled with zeros.

    Parameters
    ----------
    shape : int or sequence of ints
        Shape of the matrix
    dtype : data-type, optional
        The desired data-type for the matrix, default is float.
    cphvb : boolean, optional
        Determines whether it is a cphVB-enabled array or a regular NumPy array

    Returns
    -------
    out : matrix
        Zero matrix of given shape, dtype, and order.

    See Also
    --------
    numpy.zeros : Equivalent array function.
    matlib.ones : Return a matrix of ones.

    Notes
    -----
    The order of the data in memory is always row-major (C-style).

    If `shape` has length one i.e. ``(N,)``, or is a scalar ``N``,
    `out` becomes a single row matrix of shape ``(1,N)``.

    Examples
    --------
    >>> import numpy.matlib
    >>> np.matlib.zeros((2, 3))
    matrix([[ 0.,  0.,  0.],
            [ 0.,  0.,  0.]])

    >>> np.matlib.zeros(2)
    matrix([[ 0.,  0.]])

    """

    A = empty(shape, dtype=dtype, cphvb=cphvb)
    A[:] = 0
    return A

def empty_like(a, dtype=None, cphvb=None):
    """
    Return a new array with the same shape and type as a given array.

    Parameters
    ----------
    a : array_like
        The shape and data-type of `a` define these same attributes of the
        returned array.
    dtype : data-type, optional
        Overrides the data type of the result.
    cphvb : boolean, optional
        Determines whether it is a cphVB-enabled array or a regular NumPy array

    Returns
    -------
    out : ndarray
        Array of uninitialized (arbitrary) data with the same
        shape and type as `a`.

    See Also
    --------
    ones_like : Return an array of ones with shape and type of input.
    zeros_like : Return an array of zeros with shape and type of input.
    empty : Return a new uninitialized array.
    ones : Return a new array setting values to one.
    zeros : Return a new array setting values to zero.

    Notes
    -----
    The order of the data in memory is always row-major (C-style).

    This function does *not* initialize the returned array; to do that use
    `zeros_like` or `ones_like` instead.  It may be marginally faster than
    the functions that do set the array values.

    Examples
    --------
    >>> a = ([1,2,3], [4,5,6])                         # a is array-like
    >>> np.empty_like(a)
    array([[-1073741821, -1073741821,           3],    #random
           [          0,           0, -1073741821]])
    >>> a = np.array([[1., 2., 3.],[4.,5.,6.]])
    >>> np.empty_like(a)
    array([[ -2.00000715e+000,   1.48219694e-323,  -2.00000572e+000],#random
           [  4.38791518e-305,  -2.00000715e+000,   4.17269252e-309]])
    """

    if dtype == None:
        dtype = a.dtype
    if cphvb == None:
        cphvb = a.cphvb
    return empty(a.shape, dtype, cphvb)

def zeros_like(a, dtype=None, cphvb=None):
    """
    Return an array of zeros with the same shape and type as a given array.

    With default parameters, is equivalent to ``a.copy().fill(0)``.

    Parameters
    ----------
    a : array_like
        The shape and data-type of `a` define these same attributes of
        the returned array.
    dtype : data-type, optional
        Overrides the data type of the result.
    cphvb : boolean, optional
        Determines whether it is a cphVB-enabled array or a regular NumPy array

    Returns
    -------
    out : ndarray
        Array of zeros with the same shape and type as `a`.

    See Also
    --------
    ones_like : Return an array of ones with shape and type of input.
    empty_like : Return an empty array with shape and type of input.
    zeros : Return a new array setting values to zero.
    ones : Return a new array setting values to one.
    empty : Return a new uninitialized array.

    Notes
    -----
    The order of the data in memory is always row-major (C-style).

    Examples
    --------
    >>> x = np.arange(6)
    >>> x = x.reshape((2, 3))
    >>> x
    array([[0, 1, 2],
           [3, 4, 5]])
    >>> np.zeros_like(x)
    array([[0, 0, 0],
           [0, 0, 0]])

    >>> y = np.arange(3, dtype=np.float)
    >>> y
    array([ 0.,  1.,  2.])
    >>> np.zeros_like(y)
    array([ 0.,  0.,  0.])

    """

    b = empty_like(a, dtype=dtype, cphvb=cphvb)
    b[:] = 0
    return b

def ones_like(a, dtype=None, cphvb=None):
    """
    Return an array of ones with the same shape and type as a given array.

    With default parameters, is equivalent to ``a.copy().fill(1)``.

    Parameters
    ----------
    a : array_like
        The shape and data-type of `a` define these same attributes of
        the returned array.
    dtype : data-type, optional
        Overrides the data type of the result.
    cphvb : boolean, optional
        Determines whether it is a cphVB-enabled array or a regular NumPy array

    Returns
    -------
    out : ndarray
        Array of zeros with the same shape and type as `a`.

    See Also
    --------
    zeros_like : Return an array of zeros with shape and type of input.
    empty_like : Return an empty array with shape and type of input.
    zeros : Return a new array setting values to zero.
    ones : Return a new array setting values to one.
    empty : Return a new uninitialized array.

    Notes
    -----
    The order of the data in memory is always row-major (C-style).

    Examples
    --------
    >>> x = np.arange(6)
    >>> x = x.reshape((2, 3))
    >>> x
    array([[0, 1, 2],
           [3, 4, 5]])
    >>> np.ones_like(x)
    array([[1, 1, 1],
           [1, 1, 1]])

    >>> y = np.arange(3, dtype=np.float)
    >>> y
    array([ 0.,  1.,  2.])
    >>> np.ones_like(y)
    array([ 1.,  1.,  1.])

    """

    b = empty_like(a, dtype=dtype, cphvb=cphvb)
    b[:] = 1
    return b

def flatten(A):
    """
    Return a copy of the array collapsed into one dimension.

    Parameters
    ----------
    a : array_like
        Array from which to retrive the flattened data from.

    Returns
    -------
    y : ndarray
        A copy of the input array, flattened to one dimension.

    Notes
    -----
    The order of the data in memory is always row-major (C-style).

    Examples
    --------
    >>> a = np.array([[1,2], [3,4]])
    >>> np.flatten(a)
    array([1, 2, 3, 4])
    """

    return A.reshape(np.multiply.reduce(np.asarray(A.shape)))

def diagonal(A,offset=0):
    """
    Return specified diagonals.

    If `a` is 2-D, returns the diagonal of `a` with the given offset,
    i.e., the collection of elements of the form ``a[i, i+offset]``.  

    Parameters
    ----------
    a : array_like
        Array from which the diagonals are taken.
    offset : int, optional
        Offset of the diagonal from the main diagonal.  Can be positive or
        negative.  Defaults to main diagonal (0).

    Returns
    -------
    array_of_diagonals : ndarray
        If `a` is 2-D, a 1-D array containing the diagonal is returned.
        If the dimension of `a` is larger, then an array of diagonals is
        returned, "packed" from left-most dimension to right-most (e.g.,
        if `a` is 3-D, then the diagonals are "packed" along rows).

    Raises
    ------
    ValueError
        If the dimension of `a` is less than 2.

    See Also
    --------
    diag : MATLAB work-a-like for 1-D and 2-D arrays.
    diagflat : Create diagonal arrays.
    trace : Sum along diagonals.

    Examples
    --------
    >>> a = np.arange(4).reshape(2,2)
    >>> a
    array([[0, 1],
           [2, 3]])
    >>> a.diagonal()
    array([0, 3])
    >>> a.diagonal(1)
    array([1])

    A 3-D example:

    >>> a = np.arange(8).reshape(2,2,2); a
    array([[[0, 1],
            [2, 3]],
           [[4, 5],
            [6, 7]]])
    """
    if A.ndim !=2 :
        raise Exception("diagonal only supports 2 dimensions\n")
    if offset < 0:
        offset = -offset
        if (A.shape[0]-offset) > A.shape[1]:
            d = A[offset,:]
        else:
            d = A[offset:,0]
    else:
         if A.shape[1]-offset > A.shape[0]:
             d = A[:,offset]
         else:
             d = A[0,offset:]
    d.strides=(A.strides[0]+A.strides[1])
    return d

def diagflat(d,k=0):
    """
    Create a two-dimensional array with the flattened input as a diagonal.

    Parameters
    ----------
    v : array_like
        Input data, which is flattened and set as the `k`-th
        diagonal of the output.
    k : int, optional
        Diagonal to set; 0, the default, corresponds to the "main" diagonal,
        a positive (negative) `k` giving the number of the diagonal above
        (below) the main.

    Returns
    -------
    out : ndarray
        The 2-D output array.

    See Also
    --------
    diag : MATLAB work-alike for 1-D and 2-D arrays.
    diagonal : Return specified diagonals.
    trace : Sum along diagonals.

    Examples
    --------
    >>> np.diagflat([[1,2], [3,4]])
    array([[1, 0, 0, 0],
           [0, 2, 0, 0],
           [0, 0, 3, 0],
           [0, 0, 0, 4]])

    >>> np.diagflat([1,2], 1)
    array([[0, 1, 0],
           [0, 0, 2],
           [0, 0, 0]])

    """
    d = np.asarray(d)
    d = flatten(d) 
    size = d.size+abs(k)
    A = zeros((size,size), dtype=d.dtype, cphvb=d.cphvb)
    Ad = diagonal(A, offset=k)
    Ad[:] = d 
    return A

def diag(v, k=0):
    """
    Extract a diagonal or construct a diagonal array.

    Parameters
    ----------
    v : array_like
        If `v` is a 2-D array, return a copy of its `k`-th diagonal.
        If `v` is a 1-D array, return a 2-D array with `v` on the `k`-th
        diagonal.
    k : int, optional
        Diagonal in question. The default is 0. Use `k>0` for diagonals
        above the main diagonal, and `k<0` for diagonals below the main
        diagonal.

    Returns
    -------
    out : ndarray
        The extracted diagonal or constructed diagonal array.

    See Also
    --------
    diagonal : Return specified diagonals.
    diagflat : Create a 2-D array with the flattened input as a diagonal.
    trace : Sum along diagonals.
    triu : Upper triangle of an array.
    tril : Lower triange of an array.

    Examples
    --------
    >>> x = np.arange(9).reshape((3,3))
    >>> x
    array([[0, 1, 2],
           [3, 4, 5],
           [6, 7, 8]])

    >>> np.diag(x)
    array([0, 4, 8])
    >>> np.diag(x, k=1)
    array([1, 5])
    >>> np.diag(x, k=-1)
    array([3, 7])

    >>> np.diag(np.diag(x))
    array([[0, 0, 0],
           [0, 4, 0],
           [0, 0, 8]])
    """

    if v.ndim == 1:
        return diagflat(v,k)
    elif v.ndim == 2:
        return diagonal(v,k)
    else:
        raise ValueError("Input must be 1- or 2-d.")

def dot(A,B):
    """Pleas doc me."""

    if A.cphvb or B.cphvb:
        bridge.handle_array(A)
        bridge.handle_array(B)
    if B.ndim == 1:
        return np.add.reduce(A*B,-1)
    if A.ndim == 1:
        return add.reduce(A*np.transpose(B),-1)
    return add.reduce(A[:,np.newaxis]*np.transpose(B),-1)

def matmul(A,B):
    """Pleas doc me."""

    if A.dtype != B.dtype:
        raise ValueError("Input must be of same type")
    if A.ndim != 2 and B.ndim != 2:
        raise ValueError("Input must be 2-d.")
    if A.cphvb or B.cphvb:
        A.cphvb=True
        B.cphvb=True
        C = empty((A.shape[0],B.shape[1]),dtype=A.dtype)
        bridge.matmul(A,B,C)
        return C
    else:
	return np.dot(A,B)
	
def lu(A):
    """Pleas doc me."""

    if A.dtype != np.float32 and A.dtype != np.float64:
        raise ValueError("Input must be floating point numbers")
    if A.ndim != 2 or A.shape[0] != A.shape[1]:
        raise ValueError("Input must be square 2-d.")
    if A.cphvb:
        LU = A.copy() #do not overwrite original A
        P = empty((A.shape[0],), dtype=np.int32)
        bridge.lu(LU,P)
        return (LU, P)
    else:
	    raise ValueError("LU not supported for non cphvb numpy")
	    
def fft(A):
    """Pleas doc me."""

    if A.cphvb and A.ndim <= 2:
      if A.dtype == np.complex64 or A.dtype == np.complex128: #maybe do type conversions for others
        B = empty(A.shape,dtype=A.dtype)
        bridge.fft(A,B)
        return B
    
	return np.fft.fft(A)
	
def fft2(A):
    """Pleas doc me."""

    if A.cphvb and A.ndim == 2:
      if A.dtype == np.complex64 or A.dtype == np.complex128: #maybe do type conversions for others
        B = empty(A.shape,dtype=A.dtype)
        bridge.fft2(A,B)
        return B
    
	return np.fft.fft2(A)

def rad2deg(x, out=None):
    """Pleas doc me."""

    if out == None:
        out = 180 * x / pi
    else:
        out[:] = 180 * x / pi
    return out

def deg2rad(x, out=None):
    """Pleas doc me."""

    if out == None:
        out = x * pi / 180
    else:
        out[:] = x * pi / 180
    return out
        
def logaddexp(x1, x2, out=None):
    """Pleas doc me."""

    if out == None:
        out = log(exp(x1) + exp(x2))
    else:
        out[:] = log(exp(x1) + exp(x2))
    return out
    
def logaddexp2(x1, x2, out=None):
    """Pleas doc me."""

    if out == None:
        out = log2(exp2(x1) + exp2(x2))
    else:
        out[:] = log2(exp2(x1) + exp2(x2))
    return out
    
def modf(x, out1=None, out2=None):
    """Pleas doc me."""

    if out1 == None:
        out1 = mod(x,1.0)
    else:
        out1[:] = mod(x,1.0)
    if out2 == None:
        out2 = floor(x)
    else: 
        out2[:] = floor(x)
    return (out1, out2)

def sign(x, out=None):
    """
    Returns an element-wise indication of the sign of a number.

    The `sign` function returns ``-1 if x < 0, 0 if x==0, 1 if x > 0``.

    Parameters
    ----------
    x : array_like
      Input values.

    Returns
    -------
    y : ndarray
      The sign of `x`.

    Examples
    --------
    >>> np.sign([-5., 4.5])
    array([-1.,  1.])
    >>> np.sign(0)
    0

    """
    return add(multiply(less(x,0),-1),multiply(greater(x,0),1),out) 

def signbit(x, out=None):
    """
    Returns element-wise True where signbit is set (less than zero).

    Parameters
    ----------
    x: array_like
        The input value(s).
    out : ndarray, optional
        Array into which the output is placed. Its type is preserved
        and it must be of the right shape to hold the output.
        See `doc.ufuncs`.

    Returns
    -------
    result : ndarray of bool
        Output array, or reference to `out` if that was supplied.

    Examples
    --------
    >>> np.signbit(-1.2)
    True
    >>> np.signbit(np.array([1, -2.3, 2.1]))
    array([False,  True, False], dtype=bool)

    """
    return less(x,0,out)

def hypot(x1, x2, out=None):
    """
    Given the "legs" of a right triangle, return its hypotenuse.

    Equivalent to ``sqrt(x1**2 + x2**2)``, element-wise.  If `x1` or
    `x2` is scalar_like (i.e., unambiguously cast-able to a scalar type),
    it is broadcast for use with each element of the other argument.
    (See Examples)

    Parameters
    ----------
    x1, x2 : array_like
        Leg of the triangle(s).
    out : ndarray, optional
        Array into which the output is placed. Its type is preserved and it
        must be of the right shape to hold the output. See doc.ufuncs.

    Returns
    -------
    z : ndarray
        The hypotenuse of the triangle(s).

    Examples
    --------
    >>> np.hypot(3*np.ones((3, 3)), 4*np.ones((3, 3)))
    array([[ 5.,  5.,  5.],
           [ 5.,  5.,  5.],
           [ 5.,  5.,  5.]])

    Example showing broadcast of scalar_like argument:

    >>> np.hypot(3*np.ones((3, 3)), [4])
    array([[ 5.,  5.,  5.],
           [ 5.,  5.,  5.],
           [ 5.,  5.,  5.]])

    """
    return sqrt(add(multiply(x1,x1),multiply(x2,bx2),out),out)

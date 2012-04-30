﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace NumCIL.Generic
{
    /// <summary>
    /// Class that represents a multidimensional array
    /// </summary>
    /// <typeparam name="T">The array element type</typeparam>
    public class NdArray<T> : IEnumerable<NdArray<T>>
    {
        /// <summary>
        /// The factory used to create accessors, defaults to creating DefaultAccessor instances
        /// </summary>
        public static IAccessorFactory<T> AccessorFactory = new DefaultAccessorFactory<T>();

        /// <summary>
        /// A wrapper that allows array style indexing of values
        /// </summary>
        public class ValueAccessor : IEnumerable<T>
        {
            /// <summary>
            /// The reference to the data
            /// </summary>
            private T[] m_data = null;

            /// <summary>
            /// The NdArray that owns this instance
            /// </summary>
            private readonly NdArray<T> m_parent;

            /// <summary>
            /// Constructs a new ValueAccessor
            /// </summary>
            /// <param name="parent">The parent to lock the wrapper to</param>
            internal ValueAccessor(NdArray<T> parent)
            {
                m_parent = parent;
            }

            /// <summary>
            /// Gets or sets a value in the array
            /// </summary>
            /// <param name="index">The indices to look up</param>
            /// <returns>The value read from the underlying array</returns>
            public T this[params long[] index]
            {
                get 
                {
                    if (m_data == null)
                        m_data = m_parent.Data;

                    return m_data[m_parent.Shape[index]]; 
                }
                set 
                {
                    if (m_data == null)
                        m_data = m_parent.Data;
                    m_data[m_parent.Shape[index]] = value; 
                }
            }

            #region IEnumerable<T> Members

            /// <summary>
            /// Returns an enumerator that iterates through a collection.
            /// </summary>
            /// <returns>
            /// An <see cref="T:System.Collections.Generic.IEnumerator"/> object that can be used to iterate through the collection.
            /// </returns>
            public IEnumerator<T> GetEnumerator()
            {
                return new Utility.AnyEnumerator<T>(index => this[index], m_parent.Shape.Dimensions[0].Length);
            }

            #endregion

            #region IEnumerable Members

            /// <summary>
            /// Returns an enumerator that iterates through a collection.
            /// </summary>
            /// <returns>
            /// An <see cref="T:System.Collections.IEnumerator"/> object that can be used to iterate through the collection.
            /// </returns>
            System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
            {
                return this.GetEnumerator();
            }

            #endregion
        }

        /// <summary>
        /// A reference to the underlying data storage, should not be accessed directly, may not be completely updated
        /// </summary>
        public readonly IDataAccessor<T> m_data;

        /// <summary>
        /// Gets the real underlying data, accessing this property may flush pending executions
        /// </summary>
        public T[] Data
        {
            get 
            {
                return m_data.Data;
            }
        }


        /// <summary>
        /// A reference to the shape instance that describes this view
        /// </summary>
        public readonly Shape Shape;

        /// <summary>
        /// The value instance that gives access to values
        /// </summary>
        public readonly ValueAccessor Value;

        /// <summary>
        /// Constructs a NdArray that is a scalar wrapper,
        /// allows simple scalar operations on arbitrary
        /// NdArrays
        /// </summary>
        /// <param name="value">The scalar value</param>
        public NdArray(T value)
            : this(new T[] { value }, new long[] { 1 })
        {
        }

        /// <summary>
        /// Constructs a new NdArray over a pre-allocated array
        /// </summary>
        /// <param name="shape">The shape of the NdArray</param>
        public NdArray(Shape shape)
            : this(AccessorFactory.Create(shape.Length), shape)
        {
        }

        /// <summary>
        /// Constructs a new NdArray over a pre-allocated array and optionally shapes it
        /// </summary>
        /// <param name="data">The data to wrap in a NdArray</param>
        /// <param name="shape">The shape to view the array in</param>
        public NdArray(T[] data, Shape shape = null)
            : this(AccessorFactory.Create(data), shape)
        {
        }

        /// <summary>
        /// Constructs a new NdArray over an existing data block and optionally shapes it
        /// </summary>
        /// <param name="data">The data to wrap in a NdArray</param>
        /// <param name="shape">The shape to view the array in</param>
        public NdArray(IDataAccessor<T> data, Shape shape = null)
        {
            this.Shape = shape ?? new long[] { data.Length };
            if (data.Length < this.Shape.Length)
                throw new ArgumentOutOfRangeException("dimensionsizes", string.Format("The length of the data is {0} but the shape requires {1} elements", data.Length, shape.Length));

            m_data = data;

            Value = new ValueAccessor(this);
        }

        /// <summary>
        /// Constructs a new NdArray over a pre-allocated array and shapes it
        /// </summary>
        /// <param name="source">An existing array that will be re-shaped</param>
        /// <param name="newshape">The shape to view the array in</param>
        public NdArray(NdArray<T> source, Shape newshape = null)
            : this(source.m_data, newshape)
        {
        }

        /// <summary>
        /// Generates a new view based on this array
        /// </summary>
        /// <param name="newshape">The new shape</param>
        /// <returns>The reshaped array</returns>
        public NdArray<T> Reshape(Shape newshape)
        {
            return new NdArray<T>(this, newshape);
        }

        /// <summary>
        /// Returns a view that is a view of a single element
        /// </summary>
        /// <param name="element">The element to view</param>
        /// <returns>The subview</returns>
        public NdArray<T> Subview(long element)
        {
            if (element < 0)
                element = this.Shape.Dimensions[0].Length + element;

            if (element < 0 || element > this.Shape.Dimensions[0].Length)
                throw new ArgumentOutOfRangeException("element");

            //Special case
            if (this.Shape.Dimensions.LongLength == 1)
            {
                long pos = this.Shape[element];
                return new NdArray<T>(this, new Shape(
                    new long[] { 1 }, //Single element
                    pos, //Offset to position
                    new long[] { this.Shape.Length - pos } //Skip the rest
                ));
            }

            Shape.ShapeDimension[] dims = new Shape.ShapeDimension[this.Shape.Dimensions.Length - 1];
            Array.Copy(this.Shape.Dimensions, 1, dims, 0, dims.Length);

            //We need to modify the top dimension to skip the elements we wish to hide
            long newoffset = this.Shape[element];
            return new NdArray<T>(this, new Shape(dims, newoffset));
        }

        /// <summary>
        /// Returns a view that is a view of a range of elements
        /// </summary>
        /// <param name="range">The range to view</param>
        /// <param name="dim">The dimension to apply the range to</param>
        /// <returns>The subview</returns>
        public NdArray<T> Subview(Range range, long dim)
        {
            long first = range.First < 0 ? (this.Shape.Dimensions[dim].Length) + range.First : range.First;
            long offset = dim == this.Shape.Dimensions.LongLength ? this.Shape.Length : this.Shape.Offset + first * this.Shape.Dimensions[dim].Stride;
            
            long last;
            long stride;

            if (range.Initialized)
            {
                if (range.NewDimension)
                {
                    Shape.ShapeDimension[] n = new Shape.ShapeDimension[this.Shape.Dimensions.LongLength + 1];
                    Array.Copy(this.Shape.Dimensions, 0, n, 0, dim);
                    Array.Copy(this.Shape.Dimensions, dim, n, dim + 1, n.LongLength - (dim + 1));
                    n[dim] = new NumCIL.Shape.ShapeDimension(1, 0);

                    return new NdArray<T>(this, new Shape(n, this.Shape.Offset));
                }
                else if (range.SingleElement)
                    last = first;
                else
                    last = range.Last <= 0 ? (this.Shape.Dimensions[dim].Length - 1) + range.Last : range.Last - 1;

                stride = range.Stride;
            }
            else
            {
                last = this.Shape.Dimensions[dim].Length - 1;
                stride = 1;
            }


            long j = 0;
            
            var dimensions = this.Shape.Dimensions.Select(x =>
                {
                    if (j++ == dim)
                    {
                        if (last * stride > this.Shape.Dimensions[dim].Length - 1)
                            last = (this.Shape.Dimensions[dim].Length - 1) / stride;
                        return new Shape.ShapeDimension((last - first) + 1, stride * x.Stride);
                    }
                    else
                        return x;
                }).ToArray();

            return new NdArray<T>(this, new Shape(dimensions, offset));
        }

        /// <summary>
        /// Gets a subview on the array
        /// </summary>
        /// <param name="index">The element to get the view from</param>
        /// <returns>A view on the selected element</returns>
        public NdArray<T> this[params long[] index]
        {
            get 
            {
                NdArray<T> v = this;
                foreach (long n in index)
                    v = v.Subview(n);
                
                return v;
            }
            set
            {
                NdArray<T> lv = this[index];

                //Self-assignment
                if (lv.Shape.Equals(value.Shape) && value.m_data == this.m_data)
                    return;


                if (lv.Shape.Dimensions.Length != value.Shape.Dimensions.Length)
                    throw new Exception("Cannot assign incompatible arrays");
                
                for(long i = 0; i < lv.Shape.Dimensions.Length; i++)
                    if (lv.Shape.Dimensions[i].Length != value.Shape.Dimensions[i].Length)
                        throw new Exception("Cannot assign incompatible arrays");

                UFunc.UFunc_Op_Inner_Unary<T, NumCIL.CopyOp<T>>(new NumCIL.CopyOp<T>(), value, ref lv);
            }
        }

        /// <summary>
        /// Gets a subview on the array
        /// </summary>
        /// <param name="ranges">The range get the view from</param>
        /// <returns>A view on the selected element</returns>
        public NdArray<T> this[params Range[] ranges]
        {
            get
            {
                if (ranges == null || ranges.Length == 0)
                    return this;

                NdArray<T> v = this;
                for (long i = 0; i < ranges.LongLength; i++)
                    v = v.Subview(ranges[i], i);

                //We reduce the last dimension if it only has one element
                while (ranges.LongLength == v.Shape.Dimensions.LongLength && v.Shape.Dimensions[v.Shape.Dimensions.LongLength - 1].Length == 1)
                {
                    long j = 0;
                    v = v.Reshape(new Shape(v.Shape.Dimensions.Where(x => j++ < v.Shape.Dimensions.LongLength - 1).ToArray(), v.Shape.Offset));
                }

                return v;
            }
            set
            {
                NdArray<T> lv = this[ranges];
                var broadcastShapes = Shape.ToBroadcastShapes(value.Shape, lv.Shape);
                UFunc.Apply<T, NumCIL.CopyOp<T>>(value.Reshape(broadcastShapes.Item1), lv.Reshape(broadcastShapes.Item2));
            }
        }

        /// <summary>
        /// Returns a flattened (1-d copy) of the current data view
        /// </summary>
        /// <returns>A flattened copy</returns>
        public NdArray<T> Flatten()
        {
            NdArray<T> cp = this.Clone();
            return new NdArray<T>(cp, new long[] { cp.Shape.Length });
        }

        /// <summary>
        /// Returns a copy of the underlying data, shaped as this view
        /// </summary>
        /// <returns>A copy of the view data</returns>
        public NdArray<T> Clone()
        {
            return UFunc.Apply<T, CopyOp<T>>(this);
        }

        /// <summary>
        /// Sets all elements in the view to a specific value
        /// </summary>
        /// <param name="value">The value to set the elements to</param>
        public void Set(T value)
        {
            UFunc.Apply<T, GenerateOp<T>>(new GenerateOp<T>(value), this);
        }

        #region IEnumerable<NdArray<T>> Members

        /// <summary>
        /// Returns an enumerator that iterates through a collection.
        /// </summary>
        /// <returns>
        /// An <see cref="T:System.Collections.Generic.IEnumerator"/> object that can be used to iterate through the collection.
        /// </returns>
        public IEnumerator<NdArray<T>> GetEnumerator()
        {
            return new Utility.AnyEnumerator<NdArray<T>>(index => this[index], Shape.Dimensions[0].Length);
        }

        #endregion

        #region IEnumerable Members

        /// <summary>
        /// Returns an enumerator that iterates through a collection.
        /// </summary>
        /// <returns>
        /// An <see cref="T:System.Collections.IEnumerator"/> object that can be used to iterate through the collection.
        /// </returns>
        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            return this.GetEnumerator();
        }

        #endregion

        /// <summary>
        /// Returns a string representation of the data viewed by this NdArray
        /// </summary>
        /// <returns>A string representation of the data viewed by this NdArray</returns>
        public override string ToString()
        {
            return string.Format("NdArray<{0}>({1}): {2}", typeof(T).FullName, string.Join(", ", this.Shape.Dimensions.Select(x => x.Length.ToString()).ToArray()), this.AsString());
        }

        /// <summary>
        /// Returns the contents of this NdArray as a parseable string
        /// </summary>
        /// <param name="sb">The stringbuilder used to buffer the output</param>
        /// <returns>The contents as a parseable string</returns>
        public string AsString(StringBuilder sb = null)
        {
            sb = sb ?? new StringBuilder();

            if (this.Shape.Dimensions.LongLength == 1)
                sb.Append("[" + string.Join(", \n", this.Value.Select(x => x.ToString()).ToArray()) + "] ");
            else
                sb.Append("[" + string.Join(", \n", this.Select(x => x.AsString()).ToArray()) + "] ");

            return sb.ToString();
        }

        /// <summary>
        /// Flushes all pending operations on this array
        /// </summary>
        public void Flush()
        {
            if (m_data is ILazyAccessor<T>)
                ((ILazyAccessor<T>)m_data).Flush();
        }

        /// <summary>
        /// Returns a transposed view of this array. If <paramref name="out"/> is supplied, the contents are copied into that array.
        /// </summary>
        /// <param name="out">Optional output array</param>
        /// <returns>A transposed view or copy</returns>
        public NdArray<T> Transpose(NdArray<T> @out = null)
        {
            if (this.Shape.Dimensions.LongLength == 1)
                return new NdArray<T>(this);
            
            //Optimal case, just reshape the array
            if (@out == null)
                return new NdArray<T>(this, new Shape(this.Shape.Dimensions.Reverse().ToArray()));

            var lv = new NdArray<T>(this, new Shape(this.Shape.Dimensions.Reverse().ToArray()));
            UFunc.Apply<T, CopyOp<T>>(lv, @out);
            return @out;
        }

        /// <summary>
        /// Gets or sets data in a transposed view
        /// </summary>
        public NdArray<T> Transposed
        {
            get { return this.Transpose(); }
            set { UFunc.Apply<T, CopyOp<T>>(value, this.Transpose()); }
        }

        /// <summary>
        /// Repeats elements of the array
        /// </summary>
        /// <param name="repeats">The number of repeats to perform</param>
        /// <param name="axis">The axis to repeat, if not speficied, repeat is done on a flattened array</param>
        /// <returns>A repeated copy of the input data</returns>
        public NdArray<T> Repeat(long repeats, long? axis = null)
        {
            long real_axis =
                axis.HasValue ?
                (axis.Value < 0 ? this.Shape.Dimensions.LongLength - axis.Value : axis.Value)
                : this.Shape.Dimensions.LongLength;

            //First we add a new axis so all elements are in their own dimension
            var lv = this.Subview(Range.NewAxis, real_axis);

            //Then we extend the element dimension, and make the shapes match
            long[] targetDims = lv.Shape.Dimensions.Select(x => x.Length).ToArray();
            targetDims[real_axis] = repeats;
            var tp = Shape.ToBroadcastShapes(lv.Shape, new Shape(targetDims));

            //And then we can just copy data as normal
            var res = UFunc.Apply<T, CopyOp<T>>(lv.Reshape(tp.Item1), new NdArray<T>(tp.Item2));

            //With no axis specified, we return a flat view
            if (!axis.HasValue)
                return new NdArray<T>(res.m_data);
            else
            {
                //With a specified axis, we return a reshaped array
                long[] newDims = this.Shape.Dimensions.Select(x => x.Length).ToArray();
                newDims[real_axis] = repeats * newDims[real_axis];
                return res.Reshape(newDims);
            }
        }

        /// <summary>
        /// Repeats elements of the array
        /// </summary>
        /// <param name="repeats">The number of repeats to perform in each axis</param>
        /// <param name="axis">The axis to repeat, if not specified, repeat is done on a flattened array</param>
        /// <returns>A repeated copy of the input data</returns>
        public NdArray<T> Repeat(long[] repeats, long? axis = null)
        {
            long real_axis =
                axis.HasValue ?
                (axis.Value < 0 ? this.Shape.Dimensions.LongLength - axis.Value : axis.Value)
                : this.Shape.Dimensions.LongLength;

            if (!axis.HasValue)
            {
                //Inefficient because we need to access each element

                long elements = this.Shape.Elements;
                if (elements % repeats.LongLength != 0)
                    throw new ArgumentException(string.Format("The repeats array has length {0} and is not broadcast-able to length {1}", repeats.LongLength, elements), "repeats");

                //We need to be able to address each element individually, 
                // so we do not use tricks stride tricks and 
                // just copy each element individually
                long resultSize = 0;
                for (long i = 0; i < elements; i++)
                    resultSize += repeats[i % repeats.LongLength];

                T[] result = new T[resultSize];

                long[] counters = new long[this.Shape.Dimensions.LongLength];
                long[] limits = this.Shape.Dimensions.Select(x => x.Length).ToArray();
                var va = this.Value;
                long resCount = 0;
                //The we remove all the outer axis definitions, but preserve the stride for each element
                for (long i = 0; i < elements; i++)
                {
                    T value = va[counters];
                    for (long j = 0; j < repeats[i % repeats.LongLength]; j++)
                        result[resCount++] = value;

                    //Basically a ripple carry adder
                    long p = counters.LongLength - 1;
                    while (++counters[p] == limits[p] && p > 0)
                    {
                        counters[p] = 0;
                        p--;
                    }
                }

                return new NdArray<T>(result);
            }
            else
            {
                if (this.Shape.Dimensions[real_axis].Length % repeats.LongLength != 0)
                    throw new ArgumentException(string.Format("The repeats array has length {0} and is not broadcast-able to length {1}", repeats.LongLength, this.Shape.Dimensions[real_axis].Length), "repeats");

                long resultSize = 1;
                if (repeats.LongLength != this.Shape.Dimensions[real_axis].Length)
                {
                    long[] tmp = new long[this.Shape.Dimensions[real_axis].Length];
                    for(long i = 0; i < tmp.LongLength; i++)
                        tmp[i] = repeats[i % repeats.LongLength];

                }

                long extendedSize = repeats.Aggregate((a, b) => a + b);

                for (long i = 0; i < this.Shape.Dimensions.LongLength; i++)
                    if (i == real_axis)
                        resultSize *= extendedSize;
                    else
                        resultSize *= this.Shape.Dimensions[i].Length;

                long[] resultDims = this.Shape.Dimensions.Select(x => x.Length).ToArray();
                resultDims[real_axis] = extendedSize;
                var resultShape = new Shape(resultDims);
                var result = new NdArray<T>(resultShape);

                long curStart = 0;
                for (long i = 0; i < repeats.LongLength; i++)
                {
                    var lv = this.Subview(Range.El(i), real_axis);
                    var xv = result.Subview(new Range(curStart, curStart + repeats[i]), real_axis);
                    var broadcastShapes = Shape.ToBroadcastShapes(lv.Shape, xv.Shape);
                    UFunc.Apply<T, CopyOp<T>>(lv.Reshape(broadcastShapes.Item1), xv.Reshape(broadcastShapes.Item2));
                    curStart += repeats[i];
                }

                return result;
            }
        }

        /// <summary>
        /// Concatenates multiple arrays into a single array, joined at the axis
        /// </summary>
        /// <param name="axis">The axis to join at</param>
        /// <param name="args">The arrays to join</param>
        /// <returns>The joined array</returns>
        public static NdArray<T> Concatenate(NdArray<T>[] args, long axis = 0)
        {
            if (args == null)
                throw new ArgumentNullException("args");
            if (args.LongLength == 1)
                return args[0];

            long[] dims = args[0].Shape.Plain.Dimensions.Select(x => x.Length).ToArray();
            long newAxisSize = 0;
            foreach (var a in args)
            {
                if (a.Shape.Dimensions.LongLength != dims.LongLength)
                    throw new Exception(string.Format("Incompatible shapes, size {0} vs {1}", a.Shape.Dimensions.LongLength, dims.LongLength));
                
                for (long i = 0; i < dims.LongLength; i++)
                {
                    if (i == axis)
                        newAxisSize += a.Shape.Dimensions[i].Length;
                    else
                        if (dims[i] != a.Shape.Dimensions[i].Length)
                            throw new Exception(string.Format("Incompatible shapes in dimension {0}, size {1} vs {2}", i, a.Shape.Dimensions[i].Length, dims[i]));
                }
            }

            dims[axis] = newAxisSize;
            var res = new NdArray<T>(new Shape(dims));
            long startOffset = 0;

            foreach (NdArray<T> a in args)
            {
                long endOffset = startOffset + a.Shape.Dimensions[axis].Length;
                var lv = res.Subview(new Range(startOffset, endOffset), axis);
                UFunc.Apply<T, CopyOp<T>>(a, lv);
                startOffset = endOffset;
            }

            return res;
        }

        /// <summary>
        /// Concatenates an array onto this array, joined at the axis
        /// </summary>
        /// <param name="arg">The array to join</param>
        /// <param name="axis">The axis to join at</param>
        /// <returns>The joined array</returns>
        public NdArray<T> Concatenate(NdArray<T> arg, long axis = 0)
        {
            return Concatenate(new NdArray<T>[] { this, arg }, axis);
        }

        /// <summary>
        /// Extension to support unmanaged mapping
        /// </summary>
        public object Tag;

        /// <summary>
        /// Flag for debugging purposes
        /// </summary>
        public string Name;
    }
}

